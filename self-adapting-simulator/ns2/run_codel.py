#! /usr/bin/env python3

from collections import namedtuple
from itertools import product
from concurrent.futures import ProcessPoolExecutor
from tempfile import TemporaryDirectory
import sys
import os
import os.path as path
import asyncio

import click

import codel
import tracing
import tracing.helpers

COUNTER = None
TOTAL = None
EXECUTOR = None
PERSISTENCE = tracing.SequencePersistence(
        tracing.SQLPersistence('testik.db'),
        tracing.TraceDirPersistence('traces')
        )



TracingConfig = namedtuple('TracingConfig', ['nam', 'trace'])


def run_codel_process(ns2: codel.NS2, seed, config: TracingConfig, show_std=False):
    with TemporaryDirectory() as temp_dir:
        ns2.run(seed, temp_dir, config.nam, show_std)

        queue_trace = tracing.QueueTrace.parse(ns2, temp_dir)

        for trace in config.trace:
            if trace is tracing.QueueTrace:
                trace_result = queue_trace
            elif trace is tracing.StatsTrace:
                trace_result = trace.generate_stats(temp_dir, ns2, seed, queue_trace)
            else:
                trace_result = trace.parse(ns2, temp_dir)

            trace.save(ns2, seed, PERSISTENCE, trace_result)

        if tracing.StatsTrace in config.trace and show_std:
            tracing.helpers.do_print_basic_stats(queue_trace)


def generate_codel_variants(intervals, targets):
    return tuple(codel.SFQCodelQueueManagement(int, tar)
                 for int, tar in product(intervals, targets))


async def run_codel(ns2, config, seed):
    await asyncio.get_event_loop().run_in_executor(
        EXECUTOR, run_codel_process, ns2, seed, config, TOTAL == 1
    )

    global COUNTER
    COUNTER += 1

    if TOTAL != 1:
        print(f'{COUNTER}/{TOTAL}', end='\r')
        if COUNTER == TOTAL:
            print()


@click.command()
@click.option('--duration', default=300, type=int,
              help='Run duration (in seconds)')
@click.option('--num-runs', default=100, type=int, help='number of runs')
@click.option('--start-run', default=0, type=int, help='starting run')
@click.option('--num-ftps', default=1, type=int, help='number of FTPs')
@click.option('--ftp-start-time', default=0, type=int, 
              help='FTP start time (in seconds)')
@click.option('--web-rate', default=0, type=int, help='rate of Web requests')
@click.option('--web-start-time', default=0, type=int,
              help='WEB start time (in seconds)')
@click.option('--num-cbrs', default=0, type=int, help='number of CBRs')
@click.option('--cbr-rate', default='0.064Mb', type=codel.Rate,
              help='CBR rate')
@click.option('--cbr-packet-size', default=100, type=int,
              help='CBR packet size')
@click.option('--algo', default='codel-i100ms-t5ms', type=str, multiple=True,
              help='A queue management algorithm')
@click.option('--bottleneck', default='10Mb', type=codel.Rate,
              help='Bottleneck\'s bandwidth')
@click.option('--access-delay', default='20ms', type=codel.Interval,
              help='Access delay')
@click.option('--bottleneck-delay', default='10ms', type=codel.Interval,
              help='Bottleneck delay')
@click.option('--seed', default=None, type=int, help='Random seed')
@click.option('--num-threads', default=None, type=int,
              help='Number of threads to use')
@click.option('--greedy-ftp', is_flag=True,
              help='Make one of the ftp connections infinite')
@click.option('--nam', is_flag=True, help='Enable  NAM tracing')
@click.option('--trace', multiple=True, help='Enable specific tracing',
              type=click.Choice(tracing.Trace.ALL_TRACES.keys()),
              default=(tracing.StatsTrace.name,))
@click.option('--file-size', default=10_000_000, type=int,
              help='FTP file size')
@click.option('--learning-start-time', default=0, type=int,
              help='Times learning starts (in seconds)')
@click.option('--learning-algorithm', default='ucb', type=str,
              help='Learning Algorithm')
@click.option('--learning-interval-selector', default='fixed-i100ms', type=str,
              help='Learning interval selection mechanism')
@click.option('--trace-dir', type=str, default=None,
              help='Name of the trace directory')
@click.option('--trace-db', type=str, default=None,
              help='Name of the trace database')
@click.option('--reward', type=click.Choice(['throughput', 'delay', 'power']),
              default='throughput', help='A reward to use')
@click.option('--reward-min-delay', default=None, type=codel.Interval,
              help='Maximal delay to get a reward '
                   '[throughput(10ms),power(1ms)]')
@click.option('--reward-max-delay', default=None, type=codel.Interval,
              help='Maximal delay to get a reward '
                   '[throughput(10ms),power(100ms)]')
@click.option('--reward-min-bw-fraction', default=0.1, type=float,
              help='Minimal allowed bandwidth fraction [power]')
@click.option('--reward-max-bw-fraction', default=0.9, type=float,
              help='Maximal allowed bandwidth fraction [power]')
@click.option('--reward-delta', default=1.0, type=float,
              help='Relative importance of delay [power]')
@click.option('--reward-delay-scale', default=100.0, type=float,
              help='Scale to apply to a reward [delay]')
def run(num_runs, start_run, num_ftps, ftp_start_time, web_rate, web_start_time, 
        num_cbrs, cbr_rate, cbr_packet_size,
        algo, seed, bottleneck, num_threads, greedy_ftp, duration,
        learning_start_time, learning_algorithm, learning_interval_selector,
        access_delay, bottleneck_delay,
        # reward-related arguments
        reward, reward_min_delay, reward_max_delay,
        reward_min_bw_fraction, reward_max_bw_fraction,
        reward_delta,
        reward_delay_scale,
        nam, trace, trace_dir, trace_db, file_size):

    config = TracingConfig(
        nam=nam, trace=tuple(tracing.Trace.ALL_TRACES[t] for t in trace))

    if config.nam and num_runs != 1:
        print('setting `num-runs` to 1 since tracing is enabled',
              file=sys.stderr)
        num_runs = 1

    if tracing.WebTrace in config.trace and web_rate == 0:
        print('No web to trace')
        exit(1)

    if tracing.RewardTrace in config.trace and reward != 'power':
        print("reward trace is only supported for power reward")
        exit(1)

    if seed is not None and num_runs != 1:
        print('setting `num-runs` to 1 since seed is fixed', file=sys.stderr)
        num_runs = 1

    if reward == 'throughput':
        if reward_max_delay is None:
            reward_max_delay = codel.Interval('10ms')
        if reward_min_delay is None:
            reward_min_delay = reward_max_delay
        reward = codel.ThroughputReward(min_delay=reward_min_delay,
                                        max_delay=reward_max_delay)
    elif reward == 'delay':
        reward = codel.DelayReward(delay_scale=reward_delay_scale)
    elif reward == 'power':
        if reward_max_delay is None:
            reward_max_delay = codel.Interval('100ms')
        if reward_min_delay is None:
            reward_min_delay = codel.Interval('1ms')
        reward = codel.PowerReward(
            min_bw_fraction=reward_min_bw_fraction,
            max_bw_fraction=reward_max_bw_fraction,
            min_delay=reward_min_delay, max_delay=reward_max_delay,
            delta=reward_delta)
    else:
        assert False

    ns2 = codel.NS2(
        ftp=codel.FTPParams(num=num_ftps, start_time=ftp_start_time,
                            file_size=file_size, greedy=greedy_ftp, 
                            count_rev=0),
        web=codel.WebParams(rate=web_rate, start_time=web_start_time),
        cbr=codel.CBRParams(num=num_cbrs, rate=cbr_rate,
                            packet_size=cbr_packet_size),
        delay=codel.DelayParams(access=access_delay, bottleneck=bottleneck_delay),
        learning=codel.LearningParams(
            start_time=learning_start_time,
            algo=codel.LearningAlgorithm.from_string(learning_algorithm),
            interval_selector=codel.IntervalSelector.from_string(learning_interval_selector), 
            reward=reward),
        duration=duration,
        algorithms=tuple(codel.QueueManagement.from_string(a) for a in algo),
        bottleneck=codel.Bottleneck(rate=bottleneck),
    )

    global TOTAL, COUNTER, EXECUTOR, PERSISTENCE
    TOTAL = num_runs
    COUNTER = 0
    EXECUTOR = ProcessPoolExecutor(max_workers=num_threads)

    if trace_db is not None and trace_dir is not None:
        print('only one of "trace_db" and "trace_dir" can be specified',
              file=sys.stderr)
        exit(1)

    if trace_db is not None:
        PERSISTENCE = tracing.SQLPersistence(trace_db)
    if trace_dir is not None:
        PERSISTENCE = tracing.TraceDirPersistence(trace_dir)

    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(asyncio.gather(*(
            run_codel(ns2, config, seed or (run_id + 1))
            for run_id in range(start_run, start_run + num_runs)
        )))
    finally:
        EXECUTOR.shutdown()


if __name__ == '__main__':
    run()
