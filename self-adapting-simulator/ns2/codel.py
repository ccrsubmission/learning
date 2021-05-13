import os
import sys
from collections import namedtuple
import subprocess
from enum import Enum, auto
from itertools import chain, islice, dropwhile, takewhile
import json


def _optional(flag, *values):
    return list(values) if flag else []


def _optionals(flag, values):
    return list(values) if flag else []


class Interval(namedtuple('Interval', ['interval_'])):
    __slots__ = ()

    def __float__(self):
        if self.interval_.endswith('ms'):
            return float(self.interval_[:-2]) * 10.0 ** -3
        raise ValueError

    def __str__(self):
        return self.interval_


class Rate(namedtuple('Rate', ['rate_'])):
    __slots__ = ()

    def __float__(self):
        if self.rate_.endswith('Mb'):
            return float(self.rate_[:-2]) * 10.0 ** 6
        raise ValueError

    def __str__(self):
        return self.rate_


class ParameterizedBase:
    @classmethod
    def parse_param(cls, param):
        name = takewhile(lambda x: x.isalpha(), param)
        value = dropwhile(lambda x: x.isalpha(), param)
        return "".join(name), "".join(value)

    @classmethod
    def parse_params(cls, params):
        return {
            name: value for name, value in
            [cls.parse_param(param) for param in params]
        }

    @classmethod
    def from_params(cls, params):
        raise NotImplementedError

    @classmethod
    def from_string(cls, s):
        name, *params = s.split('-')
        params = cls.parse_params(params)
        if name not in cls.options:
            raise AssertionError(f'Unknown {cls.__name__} name: {name}')
        return cls.options[name].from_params(params)


class IntervalSelector(ParameterizedBase):
    options = {}

    def __init_subclass__(cls, name, **kwargs):
        super().__init_subclass__(**kwargs)
        IntervalSelector.options[name] = cls

    def command(self):
        raise NotImplementedError

    @property
    def short_rep(self):
        raise NotImplementedError


class FixedIntervalSelector(IntervalSelector,
                            namedtuple('FixedIntervalSelector',
                                       ['interval', 'num_subintervals']),
                            name='fixed'):
    __slots__ = ()

    def __new__(cls, interval=Interval('100ms'), num_subintervals=1):
        return super(FixedIntervalSelector, cls).__new__(cls, interval, num_subintervals)

    def command(self):
        return 'new IntervalSelector/Fixed'\
               f' {float(self.interval)} 0 {self.num_subintervals}'

    @property
    def short_rep(self):
        return (['fixed', f'i{str(self.interval)}']
                + _optional(self.num_subintervals > 1, f'nsi{self.num_subintervals}'))

    @classmethod
    def from_params(cls, params):
        return FixedIntervalSelector(
            interval=Interval(params['i']),
            num_subintervals=int(params.get('nsi', 1)))


class BinarySearchIntervalSelector(IntervalSelector,
                                   namedtuple('BinarySearchIntervalSelector',
                                              ['initial_interval', 
                                               'min_samples', 
                                               'min_threshold']),
                                   name='binary'):
    __slots__ = ()

    def __new__(cls, initial_interval=Interval('10000ms'), 
                     min_samples=10,
                     min_threshold=0.5):
        return super(BinarySearchIntervalSelector, cls).__new__(
                cls, initial_interval, min_samples, min_threshold)
                
    def command(self):
        return 'new IntervalSelector/BinarySearch'\
                f' {float(self.initial_interval)} {self.min_samples}'\
                f' {self.min_threshold}'

    #TODO: remove
    @property
    def num_subintervals(self):
        return 2;

    @property
    def short_rep(self):
        return ['binary', f'i{str(self.initial_interval)}',
                f'mins{self.min_samples}',
                f'minth{self.min_threshold}']

    @classmethod
    def from_params(cls, params):
        return BinarySearchIntervalSelector(
            initial_interval=Interval(params['i']),
            min_samples=int(params['mins']),
            min_threshold=float(params['minth']))


class QueueManagement(ParameterizedBase):
    options = {}

    def __init_subclass__(cls, name, **kwargs):
        super().__init_subclass__(**kwargs)
        QueueManagement.options[name] = cls

    def command(self, ns2):
        raise NotImplementedError

    @property
    def short_rep(self):
        raise NotImplementedError


class SFQCodelQueueManagement(QueueManagement,  
                              namedtuple('SFQCodelQueueManagement', 
                                         ['interval', 'target']),
                              name='sfqcodel'):
    __slots__ = ()

    def __str__(self):
        return f'[int={str(self.interval)},tgt={str(self.target)}]'

    def command(self, ns2):
        return ";".join(f'''
            set codel [new Queue/sfqCoDel]
            $codel set target_ {float(self.target)}
            $codel set interval_ {float(self.interval)}

            $codel trace curq_
            $codel trace d_exp_
            $codel attach $codel_trace
            set codel
        '''.splitlines())

    @property
    def short_rep(self):
        return 'sfqcodel', f't{self.target}', f'i{self.interval}'

    @classmethod
    def from_params(cls, params):
        return SFQCodelQueueManagement(
            interval=Interval(params['i']),
            target=Interval(params['t']))

class SFQQueueManagement(QueueManagement, name='sfq'):

    def __str__(self):
        return f'[]'

    def command(self, ns2):
        return ";".join(f'''
            set codel [new Queue/SFQ]
            set codel
        '''.splitlines())

    @property
    def short_rep(self):
        return 'sfq'

    @classmethod
    def from_params(cls, params):
        return SFQQueueManagement()


class REDQueueManagement(QueueManagement,  
                         namedtuple('REDQueueManagement',
                                    ['max_th', 'min_th', 'w', 'p']),
                         name='red'):
    __slots__ = ()

    def __str__(self):
        return f'[min={str(self.min_th)},max={str(self.max_th)},w={str(self.w)},p={str(self.p)}]'

    def command(self, ns2):
        return ";".join(f'''
            set queue [new Queue/RED]
            $queue set thresh_ {float(self.min_th)} 
            $queue set maxthresh_ {float(self.max_th)}
            $queue set q_weight_ {float(self.w)}
            $queue set linterm_ {float(self.p)}
            
            set queue
        '''.splitlines())

    @property
    def short_rep(self):
        return 'red', f'minth{self.min_th}', f'maxth{self.max_th}', f'w{self.w}', f'p{self.p}'

    @classmethod
    def from_params(cls, params):
        return REDQueueManagement(
            min_th=float(params['minth']),
            max_th=float(params['maxth']),
            w=float(params['w']),
            p=float(params['p'])
        )


class CBRParams(namedtuple('CBRParams', ['num', 'rate', 'packet_size'])):
    __slots__ = ()

    def __new__(cls, num=0, rate=Rate('10Mb'), packet_size=100):
        return super(CBRParams, cls).__new__(cls, num, rate, packet_size)

    @property
    def short_rep(self):
        return [f'c{self.num}'] + _optional(
            self.num > 0, f'cr{self.rate}', f'cps{self.packet_size}')


class WebParams(namedtuple('WebParams', ['rate', 'start_time'])):
    __slots__ = ()

    def __new__(cls, rate=0, start_time=0):
        return super(WebParams, cls).__new__(cls, rate, start_time)

    @property
    def short_rep(self):
        return [f'w{self.rate}'] + _optional(
                self.start_time > 0, f'wst{self.start_time}'
                )


class FTPParams(namedtuple('FTPParams',
                           ['num', 'start_time', 'file_size', 'greedy', 'count_rev'])):
    __slots__ = ()

    def __new__(cls, num=1, start_time=0, file_size=10_000_000, greedy=False, count_rev=0):
        if greedy and num == 0:
            raise ValueError('Cannot set greedy ftp when there is no FTP')
        return super(FTPParams, cls).__new__(
            cls, num, start_time, file_size, greedy, count_rev)

    @property
    def short_rep(self):
        return [f'f{self.num}'] + _optionals(
            self.num > 0, _optional(self.greedy, 'g') +
            [f'fs{self.file_size}']) + _optional(
            self.start_time > 0, f'fst{self.start_time}')


class ThroughputReward(namedtuple('ThroughputReward',
                                  ['min_delay', 'max_delay'])):
    __slots__ = ()

    def __new__(cls, min_delay=None, max_delay=Interval('10ms')):
        if min_delay is None:
            min_delay = max_delay
        if float(min_delay) > float(max_delay):
            raise ValueError("minimal delay cannot be less than maximal")
        return super(ThroughputReward, cls).__new__(cls, min_delay, max_delay)

    def command(self, ns2):
        max_throughput = (float(ns2.bottleneck.rate) *
                          float(ns2.learning.interval))
        return f'new Reward/Throughput '\
               f'{float(self.min_delay)} {float(self.min_delay)} '\
               f'{1.0 / max_throughput}'

    @property
    def short_rep(self):
        return [f'thr', f'mind{self.min_delay}', f'maxd{self.max_delay}']


class DelayReward(namedtuple('DelayReward', ['delay_scale'])):
    __slots__ = ()

    def __new__(cls, delay_scale=100.0):
        return super(DelayReward, cls).__new__(cls, delay_scale)

    def command(self, ns2):
        return f'new Reward/Delay {self.delay_scale}'

    @property
    def short_rep(self):
        return [f'del', f'ds{self.delay_scale}']


class PowerReward(namedtuple('PowerReward',
                             ['min_bw_fraction', 'max_bw_fraction',
                              'min_delay', 'max_delay', 'delta'])):
    __slots__ = ()

    def __new__(cls, min_bw_fraction=0.1, max_bw_fraction=0.9,
                min_delay=Interval('1ms'), max_delay=Interval('100ms'),
                delta=1.0):
        return super(PowerReward, cls).__new__(
            cls, min_bw_fraction, max_bw_fraction, min_delay, max_delay, delta)

    def command(self, ns2):
        return f'new Reward/Power {float(ns2.bottleneck.rate)}'\
               f' {self.min_bw_fraction} {self.max_bw_fraction}'\
               f' {float(self.min_delay)} {float(self.max_delay)}' \
               f' {float(self.delta)}'

    @property
    def short_rep(self):
        return [f'pow', 
                f'minbwf{self.min_bw_fraction}', f'maxbwf{self.max_bw_fraction}',
                f'mind{self.min_delay}', f'maxd{self.max_delay}',
                f'dlt{self.delta}']


class LearningAlgorithm(ParameterizedBase):
    options = {}

    def __init_subclass__(cls, name, **kwargs):
        super().__init_subclass__(**kwargs)
        LearningAlgorithm.options[name] = cls

    @property
    def short_rep(self):
        raise NotImplementedError

    def json(self):
        raise NotImplementedError

class UCBLearningAlgorithm(LearningAlgorithm, name='ucb'):
    @property
    def short_rep(self):
        return ['laucb']

    def json(self):
        return {
            "type": "ucb",
            "parameters": {
                "average": {
                    "type": "keep_all"
                },
                "ksi": 2.0,
                "restricted_exploration": False
            }
        }

    @classmethod
    def from_params(cls, params):
        return UCBLearningAlgorithm()

class UCBVLearningAlgorithm(LearningAlgorithm, name='ucb_v'):
    @property
    def short_rep(self):
        return ['laucb_v']

    def json(self):
        return {
            "type": "ucb_v",
            "parameters": {
                "average": {
                    "type": "keep_all"
                }
            }
        }

    @classmethod
    def from_params(cls, params):
        return UCBVLearningAlgorithm()

class UCBTunedLearningAlgorithm(LearningAlgorithm, name='ucb_tuned'):
    @property
    def short_rep(self):
        return ['laucb_tuned']

    def json(self):
        return {
            "type": "ucb_tuned",
            "parameters": {
                "average": {
                    "type": "keep_all"
                }
            }
        }

    @classmethod
    def from_params(cls, params):
        return UCBTunedLearningAlgorithm()

class UniformExploreLearningAlgorithm(LearningAlgorithm, name='exp_unif'):
    def __init__(self, time_limit):
        self.time_limit = time_limit

    @property
    def short_rep(self):
        return [f'laexp_unif-tl{self.time_limit}']

    @classmethod
    def from_params(cls, params):
        return UniformExploreLearningAlgorithm(int(params['tl']))

    def json(self):
        return {'type': 'explore_exploit',
                'parameters': {
                    'time_limit': self.time_limit,
                    'base': {
                        'type': 'uniform',
                        'parameters': {}
                    }
                }
               }

class SuccessiveRejectsLearningAlgorithm(LearningAlgorithm, name='sr'):
    def __init__(self, time_limit):
        self.time_limit = time_limit

    @property
    def short_rep(self):
        return [f'lasr']

    @classmethod
    def from_params(cls, params):
        return SuccessiveRejectsLearningAlgorithm(int(params['tl']))

    def json(self):
        return {'type': 'explore_exploit',
                'parameters' : {
                    'time_limit': self.time_limit,
                    'base' : {
                        'type': 'successive_rejects',
                        'parameters': {}
                    }
                }
               }

class LocalGreedyLearningAlgorithm(LearningAlgorithm, name='local_greedy'):
    @property
    def short_rep(self):
        return [f'lalocal_greedy']

    @classmethod
    def from_params(cls, params):
        return LocalGreedyLearningAlgorithm()

    def json(self):
        return {'type': 'local_greedy',
                'parameters': {}}


class LearningParams(namedtuple('LearningParams',
                                ['start_time', 'algo', 'reward', 'interval_selector'])):
    __slots__ = ()

    def __new__(cls, start_time=0, algo=UCBLearningAlgorithm(),
                reward=ThroughputReward(), 
                interval_selector=FixedIntervalSelector(),
                num_submeasurements=0):
        return super(LearningParams, cls).__new__(
            cls, start_time, algo, reward, interval_selector)

    @property
    def algo_json(self):
        return self.algo.json()

    @property
    def short_rep(self):
        return (_optional(self.start_time > 0, f'lst{self.start_time}')
                + self.algo.short_rep
                + self.interval_selector.short_rep
                + self.reward.short_rep)


class NodeType(Enum):
    GATEWAY = auto()
    FTP = auto()
    WEB = auto()
    CBR = auto()


class CodelResult(namedtuple('CodelResult',
                             ['drops', 'throughput',
                              'delay_mean', 'delay_var'])):

    __slots__ = ()

    @staticmethod
    def from_str(s):
        drops, throughput, delay_mean, delay_var = s.split()
        return CodelResult(
            drops=int(drops), throughput=int(throughput),
            delay_mean=float(delay_mean), delay_var=float(delay_var))


class Bottleneck(namedtuple('Bottleneck', ['rate', 'dynamic'])):
    __slots__ = ()

    def __new__(cls, rate=Rate('10Mb'), dynamic=False):
        return super(Bottleneck, cls).__new__(cls, rate, dynamic)


class DelayParams(namedtuple('DelayParams', ['access', 'bottleneck'])):
    __slots__ = ()

    def __new__(cls, access=Interval('20ms'), bottleneck=Interval('10ms')):
        return super(DelayParams, cls).__new__(cls, access, bottleneck)


class NS2(namedtuple('NS2', ['ftp', 'web', 'cbr', 'learning', 'bottleneck', 'delay',
                             'algorithms', 'duration'])):
    __slots__ = ()

    NS_PATH = os.environ['NS_PATH']

    def __new__(cls, ftp=FTPParams(), web=WebParams(), cbr=CBRParams(),
                learning=LearningParams(),
                bottleneck=Bottleneck(),
                delay=DelayParams(),
                algorithms=(SFQCodelQueueManagement(
                    interval=Interval('100ms'), target=Interval('5ms')),),
                duration=300):
        return super(NS2, cls).__new__(cls, ftp, web, cbr, learning,
                                       bottleneck, delay, algorithms,
                                       duration)

    def run(self, seed, trace_dir, trace_nam, show_std):
        common_args = [str(self.duration), trace_dir,
                       self._flag(trace_nam), str(seed)]
        ftp_args = [str(self.ftp.num), str(self.ftp.start_time),
                    str(self.ftp.file_size), self._flag(self.ftp.greedy), 
                    str(self.ftp.count_rev)]
        web_args = [str(self.web.rate), str(self.web.start_time)]
        cbr_args = [str(self.cbr.num), str(self.cbr.rate),
                    str(self.cbr.packet_size)]
        link_args = [str(self.bottleneck.rate),
                     self._flag(self.bottleneck.dynamic)]
        delay_args = [str(float(x) * 10 ** 3) for x in
                      (self.delay.access, self.delay.bottleneck)]
        algorithms = [x.command(self) for x in self.algorithms]

        # TODO: this should be cleaner, perhaps...
        algo_json_fname = os.path.join(trace_dir, "algo.json")
        with open(algo_json_fname, 'w') as f:
            json.dump(self.learning.algo_json, f)

        learning_args = [str(self.learning.start_time), 
                         str(algo_json_fname),
                         str(self.learning.interval_selector.command()),
                         str(self.learning.reward.command(self))]

        args = list(chain(common_args, ftp_args, web_args, cbr_args, link_args, delay_args,
                          learning_args, algorithms))

        out = None if show_std else subprocess.PIPE
        ns2_result = subprocess.run([self.NS_PATH, 'codel.tcl'] + args,
                                    stderr=out, stdout=out)

        if ns2_result.returncode != 0:
            print('the NS2 simulator has failed:', file=sys.stderr)
            print('args: ', *(f'"{arg}"' for arg in args), file=sys.stderr)
            if not show_std:
                print('====== STDIN ======', ns2_result.stdout.decode('ascii'),
                      '====== STDERR =====', ns2_result.stderr.decode('ascii'),
                      sep='\n', file=sys.stderr)
            raise RuntimeError('NS2 has failed')

    @property
    def short_rep(self):
        return [f'd{self.duration}']\
               + self.ftp.short_rep + self.web.short_rep + self.cbr.short_rep\
               + [f'b{self.bottleneck.rate}',
                  f'adl{self.delay.access}', f'bdl{self.delay.bottleneck}']\
               + list(chain(*(x.short_rep for x in self.algorithms)))\
               + self.learning.short_rep

    @staticmethod
    def _flag(b):
        return '1' if b else '0'

    @property
    def num_webs(self):
        return 1 if self.web.rate > 0 else 0

    @property
    def num_nodes(self):
        return 2 * (1 + self.num_webs + self.ftp.num + self.cbr.num)

    def get_nodes(self, node_type):
        return [i for i in range(self.num_nodes)
                if self.get_node_type(i) == node_type]

    def get_snd_recv_pairs(self, node_type):
        return zip(islice(self.get_nodes(node_type), 0, None, 2),
                   islice(self.get_nodes(node_type), 1, None, 2))

    def get_node_type(self, n):
        if n < 2:
            return NodeType.GATEWAY
        elif n < 2 * (1 + self.num_webs):
            return NodeType.WEB
        elif n < 2 * (1 + self.num_webs + self.ftp.num):
            return NodeType.FTP
        elif n < 2 * (1 + self.num_webs + self.ftp.num + self.cbr.num):
            return NodeType.CBR
        else:
            raise ValueError('No such node')
