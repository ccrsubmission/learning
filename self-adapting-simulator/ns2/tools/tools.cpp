#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include <iostream>
#include <limits>
#include <fstream>
#include <vector>
#include <map>
#include <string>

namespace {

namespace py = boost::python;
namespace np = py::numpy;

struct QueueEvent {
    uint16_t event;
    uint16_t size;
    uint16_t src;
    uint16_t dst;
    double ts;
    uint64_t id;
    uint64_t flow_id;
    double delay;
};

auto parse_queue_trace(std::string trace_path) -> np::ndarray {
    auto const tracing = py::import("tracing");

    auto const QUEUE_DTYPE = np::dtype(tracing.attr("QueueTrace").attr("DTYPE"));
    auto const QueueEventKind = tracing.attr("QueueEventKind");
    auto const ENQUEUE = (uint16_t) py::extract<int>(
        QueueEventKind.attr("ENQUEUE").attr("value")
    );
    auto const DEQUEUE = (uint16_t) py::extract<int>(
        QueueEventKind.attr("DEQUEUE").attr("value")
    );
    auto const DROP = (uint16_t) py::extract<int>(
        QueueEventKind.attr("DROP").attr("value")
    );

    if (QUEUE_DTYPE.get_itemsize() != sizeof(QueueEvent)) {
        throw std::logic_error("Wrong QUEUE_DTYPE size!");
    }

    std::ifstream trace(trace_path, std::ios_base::in);
    std::vector<QueueEvent> events{};
    std::unordered_map<uint64_t, double> arrival_ts{};

    char cmd;
    std::string _;
    while (trace >> cmd) {
        QueueEvent ev;
        std::string src, dst;
        trace >> ev.ts >> _ >> _ >> _ >> ev.size >> _ >> ev.flow_id
              >> src >> dst >> _ >> ev.id >> _ >> _ >> _ >> _;

        ev.src = stoul(src.substr(0, src.find('.')));
        ev.dst = stoul(dst.substr(0, dst.find('.')));

        switch (cmd) {
            case '+': {
                ev.event = ENQUEUE;
            }
                break;
            case '-': {
                ev.event = DEQUEUE;
            }
                break;
            case 'd': {
                ev.event = DROP;
            }
                break;
            default:
                continue;
        }

        if (ev.event == ENQUEUE) {
            arrival_ts[ev.id] = ev.ts;
            ev.delay = std::numeric_limits<double>::infinity();
        } else {
            ev.delay = ev.ts - arrival_ts.at(ev.id);
            arrival_ts.erase(ev.id);
        }

        events.push_back(ev);
    }
    auto const shape = py::make_tuple(events.size());
    auto const strides = py::make_tuple(sizeof(QueueEvent));
    auto const result = np::from_data(&events[0], QUEUE_DTYPE, shape, strides, py::object());
    return result.copy();
}

struct CodelDrop {
    double ts;
    uint16_t reason;
};

auto parse_codel_drop_trace(std::string trace_path) -> np::ndarray {
    auto const tracing = py::import("tracing");

    auto const CODEL_DROP_DTYPE= np::dtype(tracing.attr("CodelDropTrace").attr("DTYPE"));
    auto const CodelDropReason = tracing.attr("CodelDropReason");
    auto const OVERFLOW_ = (uint16_t) py::extract<int>(
        CodelDropReason.attr("OVERFLOW").attr("value")
    );
    auto const SCHEDULED = (uint16_t) py::extract<int>(
        CodelDropReason.attr("SCHEDULED").attr("value")
    );

    if (CODEL_DROP_DTYPE.get_itemsize() != sizeof(CodelDrop)) {
        throw std::logic_error("Wrong CODEL_DROP_DTYPE size!");
    }

    std::ifstream trace(trace_path, std::ios_base::in);
    std::vector<CodelDrop> drops{};

    CodelDrop temp{};
    std::string reason_str{};
    while (trace >> temp.ts >> reason_str) {
        if (reason_str == "overflow") {
            temp.reason = OVERFLOW_;
        } else if (reason_str == "scheduled") {
            temp.reason = SCHEDULED;
        }

        drops.push_back(temp);
    }

    auto const shape = py::make_tuple(drops.size());
    auto const strides = py::make_tuple(sizeof(CodelDrop));
    auto const result = np::from_data(
        &drops[0], CODEL_DROP_DTYPE, shape, strides, py::object());
    return result.copy();
}

}

BOOST_PYTHON_MODULE(libtools)
{
    using namespace boost::python;

    numpy::initialize();
    def("parse_queue_trace", &parse_queue_trace);
    def("parse_codel_drop_trace", &parse_codel_drop_trace);
}

