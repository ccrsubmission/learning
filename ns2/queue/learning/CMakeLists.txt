add_library(learning OBJECT 
    learning_queue.cc 
    learning_impl.cc
    learning.cc
    reward.cc 
    reward/throughput_reward.cc
    reward/delay_reward.cc
    reward/power_reward.cc
    reward/flow_statistics.cc
    utils/drop_proxy.cc
    utils/queue_utils.cc
    utils/order_correlator.cc
    interval_selection/binary_search_interval_selector.cc
    interval_selection/fixed_interval_selector.cc
    )
set_target_properties(learning PROPERTIES CXX_STANDARD 17)
target_compile_options(learning PRIVATE -Wall)
target_include_directories(learning 
    INTERFACE ${nlohmann_json_INCLUDE_DIRS}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

