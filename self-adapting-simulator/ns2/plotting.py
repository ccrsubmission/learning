import functools
from itertools import islice, repeat, chain
from math import log

from os import path

import numpy as np
import matplotlib.pyplot as plt

import codel
from codel import NS2
import tracing
import tracing.helpers as helpers
import run_codel

MARKERS = ['s', '*', 'o', 'x']


class Lens:
    def __init__(self, mods):
        self._mods = mods

    def __call__(self, obj):
        return functools.reduce(
            lambda obj, lens_value: 
                self._lens_set(obj, lens_value[0], lens_value[1]),
            self._mods.items(), obj)


    @classmethod
    def _lens_set(cls, obj, path, value):
        if not path:
            return value
        else:
            return obj._replace(**{path[0]: cls._lens_set(
                getattr(obj, path[0]), path[1:], value
            )})

    def __add__(self, other):
        if not isinstance(other, Lens):
            raise TypeError('only a lens can be added to a lens')
        new_mods = self._mods.copy()
        new_mods.update(other._mods)
        return Lens(new_mods)


def lens(*path_value):
    return Lens({path_value[:-1]: path_value[-1]})


def mod(xlabel):
    def wrap(f):
        f._xlabel = xlabel
        return f
    return wrap


@mod('codel\'s interval, ms')
def int_mod(ns2, interval):
    return lens('algorithms', run_codel.generate_codel_variants(
        intervals=[codel.Interval(f'{interval}ms')],
        targets=[codel.Interval('5ms')]
    ))(ns2)


@mod('reward max delay, ms')
def rdelay_mod(ns2, delay):
    return lens(
        'learning', 'reward', 'max_delay', codel.Interval(f'{delay}ms')
    )(ns2)


@mod('number of ftps')
def ftp_mod(ns2, ftp):
    return lens('ftp', 'num', ftp)(ns2)


@mod('bandwidth, Mbps')
def bw_mod(ns2, bw):
    return lens('bottleneck', 'rate', codel.Rate(f'{bw}Mb'))(ns2)


@mod('web rate')
def web_mod(ns2, rate):
    return lens('web', 'rate', rate)(ns2)

@mod('learning interval, ms')
def lint_mod(ns2, interval):
    return lens(
        'learning', 'interval_selector', 'interval', codel.Interval(f'{interval}ms')
        )(ns2)


def plot_delay_n_throughput_as_function_of(xs, ns2, codel_mod, ax_delay, ax_thr,
                                           label=None):
    data = np.array([
        (delay, delay_std, thr, thr_std)
        for (delay, delay_std), (thr, thr_std) in (
           (_get_throughput_w_std(s), _get_delay_w_std(s))
           for s in _load_multiple_points(ns2, codel_mod, xs, helpers.load_stats_for)
        )
    ])

    ax_delay.errorbar(xs, data[0], yerr=data[1], label=label, capsize=5, fmt='-o')
    ax_thr.errorbar(xs, data[2], yerr=data[3], label=label, capsize=5, fmt='-x')


def delay_n_throughput_as_function_of(xs, ns2s, codel_mod):
    fig, ax1 = plt.subplots(figsize=(15, 5))
    ax1.set_xlabel(codel_mod._xlabel)
    ax1.set_xticks(xs)
    ax1.set_xlim(xmin=xs.min(), xmax=xs.max())
    ax1.set_ylabel('delay, s', color='blue')
    ax1.grid()
    ax2 = ax1.twinx()
    ax2.set_ylabel('throughput, bps', color='orange')
    ax2.set_ylim(ymin=0)

    for name, ns2 in ns2s.items():
        plot_delay_n_throughput_as_function_of(xs, ns2, codel_mod, ax1, ax2, label=name)


def _get_delay_w_std(stats):
    return stats['all_delay'].mean(), np.sqrt(stats['all_delay_var']).mean()


def _get_throughput_w_std(stats):
    return stats['all_throughput'].mean(), stats['all_throughput'].std()


def _load_learning_reward(ns2: NS2):
    return helpers.load_all_learning(ns2, 'reward')


def _load_rtt(ns2: NS2):
    return tracing.TCPRTTTrace.load_all_seeds(ns2, run_codel.PERSISTENCE)


def _load_multiple_points(base: NS2, codel_mod, xs, loader):
    for x in xs:
        try:
            yield loader(codel_mod(base, x))
        except tracing.NoDataError:
            print(f'Warning: value {x} is not available')


def plot_reward_as_function_of(xs, ns2, codel_mod, label=None):
    data = np.array([
        (reward.mean(), reward.std())
        for reward in _load_multiple_points(ns2, codel_mod, xs, _load_learning_reward)
    ])

    plt.errorbar(xs, data[0], yerr=data[1], capsize=5, label=label)


def reward_as_function_of(xs, ns2s, codel_mod):
    plt.figure(figsize=(15, 5))
    plt.xlabel(codel_mod._xlabel)
    plt.ylabel('reward')
    plt.xticks(xs)
    plt.xlim(xmin=xs.min(), xmax=xs.max())
    plt.ylim(ymin=0, ymax=1)

    for name, ns2 in ns2s.items():
        plot_reward_as_function_of(xs, ns2, codel_mod, label=name)

    plt.legend()


def plot_rtt_as_function_of(xs, ns2, codel_mod, use_excess_rtt=False, label=None):
    rtts = np.array([
        _apply_use_excess(np.concatenate(list(rtt)), use_excess_rtt).mean()
        for rtt in _load_multiple_points(ns2, codel_mod, xs, _load_rtt)
    ])
    plt.plot(xs, rtts, label=label)


def rtt_as_function_of(xs, ns2s, codel_mod, use_excess_rtt=False):
    plt.figure(figsize=(15, 5))
    plt.ylabel('avg RTT, ms')
    plt.xlabel(codel_mod._xlabel)
    plt.xticks(xs)
    plt.xlim(xmin=xs.min(), xmax=xs.max())

    for name, ns2 in ns2s.items():
        plot_rtt_as_function_of(xs, ns2, codel_mod, 
                                use_excess_rtt=use_excess_rtt,
                                label=name)

    plt.legend()


def fct_as_function_of(xs, ns2s, codel_mod):
    plt.figure(figsize=(15, 5))
    plt.ylabel('average FCT, ms')
    plt.xlabel(codel_mod._xlabel)
    plt.xticks(xs)
    plt.xlim(xmin=xs.min(), xmax=xs.max())

    for name, ns2 in ns2s.items():
        plot_fct_as_function_of(codel_mod, ns2, xs, label=name)

    plt.legend()


def plot_fct_as_function_of(codel_mod, ns2, xs, label=None):
    data = np.array([
        (cts.mean(), cts.std()) for
        cts in _load_multiple_points(ns2, codel_mod, xs, _load_all_web_cts)
    ])

    plt.errorbar(xs, data[0], yerr=data[1], capsize=5, label=label)


def _load_all_web_cts(ns2):
    ith_cts = [x['ct'] for x in tracing.WebTrace.load_all_seeds(ns2, run_codel.PERSISTENCE)]
    return np.concatenate(ith_cts)


def _calc_per_flow_throughput(web, flow_ids):
    time_on, response_size = _calc_per_flow_time_and_size(web, flow_ids)
    return response_size / time_on


def _calc_per_flow_time_and_size(web, flow_ids):
    web = web[np.in1d(web['flow_id'], flow_ids)]
    indices = np.searchsorted(flow_ids, web['flow_id'])
    time_on = np.bincount(indices, web['ct'])
    response_size = np.bincount(indices, web['resp_size'])
    return time_on, response_size


def _apply_use_excess(tcp_rtt, use_excess_rtt):
    if use_excess_rtt:
        return tcp_rtt['rtt'] - tcp_rtt['min_rtt']
    else:
        return tcp_rtt['rtt']


def _calc_per_flow_rtt(tcp_rtt, flow_ids, use_excess_rtt):
    tcp_rtt = tcp_rtt[np.argsort(tcp_rtt['flow_id'])]
    tcp_rtt = tcp_rtt[np.searchsorted(tcp_rtt['flow_id'], flow_ids)]
    return _apply_use_excess(tcp_rtt, use_excess_rtt)


def _filter_rtt_by_node_type(ns2, tcp_rtt, node_type):
    receivers = [recv for _, recv in ns2.get_snd_recv_pairs(node_type)]
    return tcp_rtt[np.isin(tcp_rtt['dst'], receivers)]


def _calc_web_power(ns2, tcp_rtt, web, delta, use_excess_rtt):
    web_tcp_rtt = _filter_rtt_by_node_type(ns2, tcp_rtt, codel.NodeType.WEB)

    # for some (probably not finished) rtt is not being saved
    web_flow_ids = np.intersect1d(web['flow_id'], web_tcp_rtt['flow_id'])

    web_throughput = _calc_per_flow_throughput(web, web_flow_ids)
    web_rtt = _calc_per_flow_rtt(web_tcp_rtt, web_flow_ids, use_excess_rtt)
    return web_throughput / np.power(web_rtt, delta)


def _calc_ftp_power(ns2, tcp_rtt, delta, use_excess_rtt):
    ftp_tcp_rtt = _filter_rtt_by_node_type(ns2, tcp_rtt, codel.NodeType.FTP)
    assert(np.all(ftp_tcp_rtt['rtt'] == ftp_tcp_rtt['rtt']))
    return ftp_tcp_rtt['throughput'] / np.power(_apply_use_excess(ftp_tcp_rtt, use_excess_rtt), delta)


class PowerPlotter:
    def __init__(self, codel_mod, xs, delta=1.0, node_type=None, use_excess_rtt=False):
        self.codel_mod = codel_mod
        self.xs = xs
        self.node_type = node_type
        self.use_excess_rtt = use_excess_rtt
        self.delta = delta

    def plot(self, ns2, label):
        powers = np.array(list(
            _load_multiple_points(ns2, self.codel_mod, self.xs, self.load_power)
        ))

        plt.plot(self.xs, powers, label=label)

    def load_power(self, ns2):
        power = []
        tcps_and_webs = zip(
                _load_rtt(ns2),
                tracing.WebTrace.load_all_seeds(ns2, run_codel.PERSISTENCE)
                if ns2.web.rate > 0 else repeat(None))

        for tcp_rtt, web in tcps_and_webs:
            per_flow_power = np.array([])

            if ns2.web.rate > 0 and (self.node_type is None or self.node_type == codel.NodeType.WEB):
                per_flow_power = np.concatenate([
                    per_flow_power,
                    _calc_web_power(ns2, tcp_rtt, web, self.delta, self.use_excess_rtt)
                ])

            if self.node_type is None or self.node_type == codel.NodeType.FTP:
                per_flow_power = np.concatenate([
                    per_flow_power,
                    _calc_ftp_power(ns2, tcp_rtt, self.delta, self.use_excess_rtt)
                ])

            power.append(np.log(per_flow_power[per_flow_power > 0]).mean())

        return np.array(power).mean()


def power_as_function_of(xs, ns2s, codel_mod, delta=1.0, node_type=None, use_excess_rtt=False):
    plt.figure(figsize=(15, 5))
    plt.ylabel('average log(power)')
    plt.xlabel(codel_mod._xlabel)
    plt.xticks(xs)
    plt.xlim(xmin=xs.min(), xmax=xs.max())

    plotter = PowerPlotter(codel_mod, xs, delta, node_type, use_excess_rtt)
    
    for name, ns2 in ns2s.items():
        plotter.plot(ns2, name)

    plt.legend()


def plot_win(ns2, runs, field):
    ns2.algorithms = runs['codel']
    x = helpers.load_stats_for(ns2)
    baseline = x[field]
    seeds = x['seed']
    values = {}
    for run_id, run in runs.items():
        if run_id != 'codel':
            ns2.algorithms = run
            x = helpers.load_stats_for(ns2)
            x = x[np.argsort(x['seed'])]
            indices = np.searchsorted(x['seed'], seeds)
            values[run_id] = x[field][indices] / baseline
    plt.figure(figsize=(15, 5))
    plt.hist(list(values.values()), label=list(values.keys()), bins=20)
    plt.xlabel(field)
    plt.ylabel('count')
    plt.legend()


def plot_bins(ylabel, ns2, time, runs, filter_fn, value_fn):
    plt.figure(figsize=(15, 5))
    plt.ylabel(ylabel)
    plt.xlabel('time, s')

    for (run_id, run), marker in zip(runs.items(), MARKERS):
        ns2.algorithms = run
        values = []
        for x in tracing.QueueTrace.load_all_seeds(ns2, run_codel.PERSISTENCE):
            x = filter_fn(x)
            values.append(value_fn(x, time))
        values = np.mean(np.stack(values), axis=0)
        plt.plot(time[1:], values, label=run_id)
    plt.legend()


def plot_time(ns2, time, runs):
    plot_bins('delay', ns2, time, runs,
              lambda x: x[x['event'] == tracing.QueueEventKind.DEQUEUE.value],
              lambda x, time: avg_by_bins(x, 'delay', time))
    plot_bins('throughput', ns2, time, runs,
              lambda x: x[x['event'] == tracing.QueueEventKind.DEQUEUE.value],
              lambda x, time: sum_by_bins(x, 'size', time))


def _get_learning_label(runs, i):
    real = [run_id for run_id, run in runs.items()
            if len(run) == 1 and run[0] == runs['learning'][i]]
    return real[0] if real else str(runs['learning'][i])


def get_learning_plot_data(ns2, runs):
    arms = helpers.load_all_learning(ns2._replace(algorithms=tuple(runs.values())), 'arm')
    xs = np.arange(arms.shape[1])
    lines = [(arm_id, (arms == i).mean(axis=0) * 100) for i, arm_id in enumerate(runs.keys())]

    return xs, lines


def plot_learning(ns2, runs, stack=True):
    num_arms = len(runs)

    xs, lines = get_learning_plot_data(ns2, runs)
    plt.figure(figsize=(15, 5))
    plt.xlabel('learning interval')
    plt.ylabel('policy used, %')
    plt.xlim(xmin=xs.min(), xmax=xs.max())
    plt.yticks(np.arange(0, 101, 25))

    if stack:
        plt.stackplot(xs, [x[1] for x in lines], labels=[x[0] for x in lines], step='mid')
    else:
        for label, ys in enumerate(lines):
            plt.plot(xs, ys, label=label)
    plt.legend(loc='lower right')
    plt.grid(axis='y')

def save_learning(ns2, runs, learning_label=''):
    xs, lines = get_learning_plot_data(ns2, runs)
    labels = ['x']
    data = [xs]
    for label, ys in lines:
        labels.append(label)
        data.append(ys)
    data = np.stack(data, axis=1)
    np.savetxt(
            path.join('plots', f'd{ns2.duration}_w{ns2.web.rate}_f{ns2.ftp.num}_li{ns2.learning.interval_selector.interval}_learning_{learning_label}.tsv'), 
            data, delimiter='\t', header='\t'.join(labels), comments='')


def get_reward_plot_data_one(ns2, norm):
    rs = helpers.load_all_learning(ns2, 'reward')
    if norm:
        reward = ns2.learning.reward
        norm_factor = (
            log(reward.max_bw_fraction) - log(reward.min_bw_fraction)
            -log(float(reward.min_delay)) + log(float(reward.max_delay))
        )
        rs *= norm_factor
    xs = list(range(rs.shape[1]))
    ys = rs.mean(axis=0)
    yerr = rs.std(axis=0)
    return xs, ys, yerr

def get_reward_plot_data(ns2, runs, algos, norm):
    xs = None
    lines = []
    
    for run_id in runs.keys():
        run_ns2 = ns2._replace(algorithms=(runs[run_id],))
        for algo in algos: 
            try:
                xs, ys, yerr = get_reward_plot_data_one(lens('learning', 'algo', algo)(run_ns2), norm)
                lines.append((run_id, ys, yerr))
            except tracing.NoDataError:
                pass
            else:
                break

    learning_ns2 = ns2._replace(algorithms=tuple(runs.values()))
    for algo in algos:
        try:
            xs, ys, yerr = get_reward_plot_data_one(lens('learning', 'algo', algo)(learning_ns2), norm)
            lines.append((f'learning-{"".join(algo.short_rep)}', ys, yerr))
        except tracing.NoDataError:
            pass
    
    return xs, lines


def save_reward(ns2, runs, algos, norm=False, reward_label=''):
    xs, lines = get_reward_plot_data(ns2, runs, algos, norm)
    labels = ['x']
    data = [xs]

    for label, ys, yerr in lines:
        labels.extend([f'{label}_y', f'{label}_yerr'])
        data.extend([ys, yerr])
    data = np.stack(data, axis=1)
    np.savetxt(
            path.join('plots', f'd{ns2.duration}_w{ns2.web.rate}_f{ns2.ftp.num}_li{ns2.learning.interval_selector.interval}_reward_{reward_label}.tsv'), 
            data, delimiter='\t', header='\t'.join(labels), comments='')


def plot_reward(ns2, runs, algos, ymin=None, ymax=None, errorbar=False, norm=False):
    plt.figure(figsize=(15, 5))
    plt.xlabel('learning interval')
    plt.ylabel('reward')

    xs, lines = get_reward_plot_data(ns2, runs, algos, norm)
    
    for label, ys, yerr in lines:
        if errorbar:
            plt.errorbar(xs, ys, yerr=yerr, fmt='o', label=label)
        else:
            plt.plot(xs, ys, 'o', label=label)
    if xs is not None:
        plt.xlim(xmin=0, xmax=len(xs))

    plt.ylim(ymin=ymin, ymax=ymax)
    plt.legend()

def plot_reward_dist_combined(ns2, runs, bins=10):
    algorithms = runs['learning']
    num_arms = len(algorithms)

    ns2_learning = ns2._replace(algorithms=algorithms)
    arms = helpers.load_all_learning(ns2_learning, 'arm').flatten()
    rs = helpers.load_all_learning(ns2_learning, 'reward').flatten()
    pure_rewards = []
    learnt_rewards = []
    labels = []
    for i, var in enumerate(algorithms):
        ns2_pure = ns2._replace(algorithms=[var])
        pure_rewards.append(helpers.load_all_learning(ns2_pure, 'reward').flatten())
        learnt_rewards.append(np.compress(arms == i, rs))
        labels.append(_get_learning_label(runs, i))
    fig, (pure, learnt) = plt.subplots(ncols=2, figsize=(15, 5))
    pure.hist(pure_rewards, label=labels, density=False, bins=bins)
    learnt.hist(learnt_rewards, label=labels, density=False, bins=bins)
    for ax in (pure, learnt):
        ax.legend()
        ax.set_xlabel('reward')
        ax.set_ylabel('fraction')
        ax.set_xlim(xmin=0, xmax=1)


def plot_reward_dist(ns2, runs, bins=10):
    algorithms = runs['learning']
    num_arms = len(algorithms)

    fig, axes = plt.subplots(ncols=num_arms, figsize=(15, 5))

    ns2_learning = ns2._replace(algorithms=algorithms)
    arms = helpers.load_all_learning(ns2_learning, 'arm').flatten()
    rs = helpers.load_all_learning(ns2_learning, 'reward').flatten()
    for axis, (i, var) in zip(axes, enumerate(algorithms)):
        ns2_pure = ns2._replace(algorithms=[var])
        pure_rs = helpers.load_all_learning(ns2_pure, 'reward').flatten()
        learn_rs = np.compress(arms == i, rs)
        axis.hist([pure_rs, learn_rs], label=['pure', 'learnt'],
                  density=True, bins=bins)
        axis.legend()
        axis.set_title(_get_learning_label(runs, i))
        axis.set_xlabel('reward')
        axis.set_ylabel('fraction')
        axis.set_xlim(xmin=0, xmax=1)


def plot_hist(name, indices, ns2, runs,
              xmin=None, xmax=None, bins=20, scale=None):
    plt.figure(figsize=(15, 5))
    columns = [helpers.load_stats_for(ns2, run)[i]
               for run in runs.values() for i in indices.values()]
    if scale is not None:
        for i in range(len(columns)):
            columns[i] = columns[i] * scale
    plt.hist(columns, bins=bins, label=list(
        f'{run} ({n})' for run in runs.keys() for n in indices.keys()
    ), density=True)
    if xmin is not None:
        plt.xlim(xmin=xmin)
    if xmax is not None:
        plt.xlim(xmax=xmax)
    plt.xlabel(name)
    plt.legend()


def plot_stats(runs, ns2, what=('all',)):
    plot_hist('throughput, B', {n: f'{n}_throughput' for n in what},
              ns2, runs)
    plot_hist('delay, s', {n: f'{n}_delay' for n in what}, ns2, runs)


def calc_by_bins(x, field, bins, func):
    pos = np.searchsorted(x['ts'], bins)
    result = []

    for i in range(pos.size - 1):
        result.append(func(
            x[field][pos[i]:pos[i + 1]] if field else x[pos[i]:pos[i + 1]]
        ))
    return np.array(result)


def avg_by_bins(x, field, bins):
    return calc_by_bins(x, field, bins,
                        lambda x: 0 if x.size == 0 else x.sum() / x.size)


def sum_by_bins(x, field, bins):
    return calc_by_bins(x, field, bins, lambda x: x.sum())


def max_by_bins(x, field, bins):
    return calc_by_bins(x, field, bins,
                        lambda x: 0 if x.size == 0 else x.max())


def min_by_bins(x, field, bins):
    return calc_by_bins(x, field, bins,
                        lambda x: 0 if x.size == 0 else x.min())


def count_by_bins(x, bins):
    return calc_by_bins(x, None, bins, lambda x: x.size)
