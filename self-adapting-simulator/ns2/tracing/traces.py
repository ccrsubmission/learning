import shutil
import logging
import sqlite3
from collections import namedtuple
from itertools import chain, count
from enum import Enum, auto
import os
from os import path
import re
import pickle

import numpy as np

import libtools
import codel


class NoDataError(Exception):
    def __init__(self, kind, ns2, fname_base, seed=None):
        super(Exception, self).__init__(
            f'unable to load {kind} for {ns2}' +
            f' with seed {seed}' if seed is not None else '')
        self.kind = kind
        self.ns2 = ns2
        self.seed = seed
        self.fname_base = fname_base


class PersistenceError(Exception):
    def __init__(self):
        super(Exception, self).__init__('Persistence failed')


class TraceDirPersistence:
    def __init__(self, trace_dir):
        self._trace_dir = trace_dir

    def _get_trace_dir(self, trace_path):
        return path.join(self._trace_dir, trace_path)

    def load(self, trace_path, seed):
        try:
            return np.load(path.join(self._get_trace_dir(trace_path), f'{seed}.npy'))
        except OSError as exc:
            raise PersistenceError() from exc

    def save(self, trace_path, seed, value):
        if not path.exists(self._get_trace_dir(trace_path)):
            os.makedirs(self._get_trace_dir(trace_path), exist_ok=True)
        np.save(path.join(self._get_trace_dir(trace_path), str(seed)), value)

class SQLPersistence:
    def __init__(self, dbname):
        self._dbname = dbname
        self._ensure_exists()

    def _ensure_exists(self):
        with sqlite3.connect(self._dbname) as con:
            con.execute('create table if not exists data '
                       '(path text, '
                       ' seed integer, '
                       ' value blob, '
                       ' primary key ("path", "seed"));')
            con.commit()

    def save(self, trace_path, seed, value):
        with sqlite3.connect(self._dbname) as con:
            con.execute('insert into data values (:path, :seed, :value) '
                        'on conflict (path,seed) do '
                        'update set value=:value;',
                        {
                            'path': trace_path, 
                            'seed': seed, 
                            'value': pickle.dumps(value)}
                        )
            con.commit()

    def load(self, trace_path, seed):
        with sqlite3.connect(self._dbname) as con:
            result = con.execute(
                    f'select value from data where '
                    f' path == "{trace_path}" and seed == {seed};'
                    ).fetchone()
            if result is None:
                raise PersistenceError()
            return pickle.loads(result[0])

class SequencePersistence:
    def __init__(self, *persistences):
        self._persistences = persistences

    def save(self, trace_path, seed, value):
        for p in self._persistences:
            try:
                return p.save(trace_path, seed, value)
            except PersistenceError:
                pass
        raise PersistenceError

    def load(self, trace_path, seed):
        for p in self._persistences:
            try:
                return p.load(trace_path, seed)
            except PersistenceError:
                pass
        raise PersistenceError

class Trace:
    DTYPE = None
    name = None

    ALL_TRACES = {}

    @classmethod
    def process_trace(cls, ns2, seed, temp_dir, persistence):
        cls.save(ns2, seed, persistence, cls.parse(ns2, temp_dir))

    @classmethod
    def __init_subclass__(cls, **kwargs):
        Trace.ALL_TRACES[cls.name] = cls

    @classmethod
    def get_trace_path(cls, ns2):
        return f'{cls._get_filename_base(ns2)}.{cls.name}'

    @classmethod
    def _get_trace_source(cls, temp_dir):
        return path.join(temp_dir, f'{cls.name}.tr')

    @classmethod
    def load(cls, ns2, seed, persistence):
        try:
            return persistence.load(cls.get_trace_path(ns2), seed)
        except PersistenceError as exc:
            fname_base = cls._get_filename_base(ns2)
            raise NoDataError(cls.name, ns2, fname_base, seed) from exc

    @classmethod
    def load_all_seeds(cls, ns2, persistence):
        for seed in count(1):
            try:
                yield cls.load(ns2, seed, persistence)
            except NoDataError as err:
                if seed == 1:
                    logging.warning(f'Trace {err.kind} not found: {err.fname_base}')
                    raise
                else:
                    break

    @classmethod
    def save(cls, ns2, seed, persistence, value):
        persistence.save(cls.get_trace_path(ns2), seed, value)

    @classmethod
    def parse(cls, ns2, temp_dir):
        raise NotImplementedError

    @staticmethod
    def _get_filename_base(ns2: codel.NS2, sep='-'):
        return sep.join(ns2.short_rep)


class QueueEventKind(Enum):
    ENQUEUE = auto()
    DEQUEUE = auto()
    DROP = auto()


class QueueTrace(Trace):
    DTYPE = np.dtype([('event', 'u2'),
                      ('size', 'u2'),
                      ('src', 'u2'), ('dst', 'u2'),
                      ('ts', 'f8'),
                      ('id', 'u8'),
                      ('flow_id', 'u8'),
                      ('delay', 'f8')], align=True)

    name = 'queue'

    @classmethod
    def parse(cls, ns2, temp_dir):
        return libtools.parse_queue_trace(cls._get_trace_source(temp_dir))


class CodelTrace(Trace):
    DTYPE = np.dtype([('kind', 'b'),
                      ('ts', 'f8'),
                      ('value', 'f8')])

    name = 'codel'

    @classmethod
    def parse(cls, ns2, temp_dir):
        return np.loadtxt(cls._get_trace_source(temp_dir),
                          dtype=cls.DTYPE,
                          converters={0: lambda s: ord(s)})


class WebTrace(Trace):
    DTYPE = np.dtype([
        ('ts', 'f8'), ('flow_id', 'u4'),
        ('req_size', 'u4'), ('resp_size', 'u4'), ('ct', 'f8'),
        ('dst', 'u2'), ('dst_port', 'u2')])

    name = 'web'

    @staticmethod
    def _fix_ms_to_s(result):
        result['ct'] /= 1000.0

    @classmethod
    def parse(cls, ns2, temp_dir):
        result = np.genfromtxt(cls._get_trace_source(temp_dir), dtype=cls.DTYPE)
        cls._fix_ms_to_s(result)
        return result


class CodelDropReason(Enum):
    OVERFLOW = auto()
    SCHEDULED = auto()


class CodelDropTrace(Trace):
    DTYPE = np.dtype([('ts', 'f8'),
                      ('reason', 'u2')], align=True)

    name = 'codel_drop'

    @classmethod
    def parse(cls, ns2, temp_dir):
        return libtools.parse_codel_drop_trace(cls._get_trace_source(temp_dir))


class TCPTrace(Trace):
    DTYPE = np.dtype([('ts', 'f8'),
                      ('src', 'u2'), ('src_port', 'u2'),
                      ('dst', 'u2'), ('dst_port', 'u2'),
                      ('cwnd', 'f8')])

    name = 'tcp'

    @classmethod
    def parse(cls, ns2, temp_dir):
        return np.genfromtxt(cls._get_trace_source(temp_dir),
                             dtype=cls.DTYPE,
                             usecols=(0, 1, 2, 3, 4, 6))


class LearningTrace(Trace):
    name = 'learning'

    @classmethod
    def parse(cls, ns2, temp_dir):
        shutil.copyfile(cls._get_trace_source(temp_dir), './a')
        return np.loadtxt(cls._get_trace_source(temp_dir), dtype=cls.create_dtype(ns2))

    @classmethod
    def create_dtype(cls, ns2):
        return np.dtype([
            ('arm', 'u1'), 
            ('reward', 'f8'), 
            ('subrewards', 'f8', ns2.learning.interval_selector.num_subintervals)
            ])


_Stats = namedtuple('_Stats', ['departures', 'delays'])


class TCPRTTTrace(Trace):
    DTYPE = np.dtype([('node_type', 'u1'), ('flow_id', 'u4'),
                      ('dst', 'u2'), ('dst_port', 'u2'),
                      ('rtt', 'f8'), ('min_rtt', 'f8'),
                      ('throughput', 'f8'), ('samples', 'u4')])

    name = 'tcp_rtt'

    @classmethod
    def parse(cls, ns2, temp_dir):
        result = []

        for node_type in codel.NodeType:
            result.extend(cls._get_entries_for_node_type(temp_dir, node_type))

        return np.array(result, dtype=cls.DTYPE)

    @classmethod
    def _get_entries_for_node_type(cls, temp_dir, node_type):
        fname = TCPRTTTrace._get_tcp_rtt_trace_source(temp_dir, node_type)

        if not path.exists(fname):
            return

        with open(cls._get_tcp_rtt_trace_source(temp_dir, node_type)) as tr_f:
            for line in tr_f:
                match = re.search(r"^(?P<fid>\w+): .*"
                                  r"daddr=(?P<dst>[^ ]+), .*"
                                  r"dport=(?P<dst_port>[^ ]+), .*"
                                  r"tp=(?P<throughput>[^ ]+) mbps, .*" 
                                  r"del=(?P<rtt>[^ ]+) ms, .*"
                                  r"mindel=(?P<minrtt>[^ ]+) ms, .*"
                                  r"samples=(?P<samples>[^ ]+), .*", line)
                if match is None:
                    raise AssertionError(line)
                
                yield node_type.value, int(match['fid']),\
                    int(match['dst']), int(match['dst_port']),\
                    float(match['rtt']) / 1000.0, float(match['minrtt']) / 1000.0,\
                    float(match['throughput']) * 10.0**6,\
                    int(match['samples'])

    @staticmethod
    def _get_tcp_rtt_trace_source(temp_dir, node_type: codel.NodeType):
        return path.join(temp_dir, f'{node_type.name.lower()}_tcp.tr')


class StatsTrace(Trace):
    DTYPE = np.dtype([
        ('seed', 'u8'),
        ('all_drops', 'u8'), ('all_throughput', 'u8'),
        ('all_delay', 'f8'), ('all_delay_var', 'f8'),
        ('ftp_throughput', 'u8'), ('ftp_delay', 'f8'),
        ('ftp_50_delay', 'f8'), ('ftp_75_delay', 'f8'), ('ftp_95_delay', 'f8'),
        ('web_throughput', 'u8'), ('web_delay', 'f8'),
        ('web_50_delay', 'f8'), ('web_75_delay', 'f8'), ('web_95_delay', 'f8'),
        ('cbr_throughput', 'u8'), ('cbr_delay', 'f8'),
        ('cbr_50_delay', 'f8'), ('cbr_75_delay', 'f8'), ('cbr_95_delay', 'f8'),
    ])

    name = 'stats'

    @classmethod
    def parse(cls, ns2, temp_dir):
        raise NotImplementedError('Please, use parse stats for stats')

    @classmethod
    def generate_stats(cls, temp_dir, ns2, seed, queue_trace):
        stats = {}
        for nt in [codel.NodeType.FTP, codel.NodeType.WEB, codel.NodeType.CBR]:
            mask = np.isin(queue_trace['src'], np.array(ns2.get_nodes(nt)))
            stats[nt] = cls._get_stats(queue_trace, mask)

        th_delay = chain(*((
            stats[nt].departures.sum(),
            stats[nt].delays.mean()
            if stats[nt].delays.size > 0 else float('NaN'),
            np.median(stats[nt].delays)
            if stats[nt].delays.size > 0 else float('NaN'),
            np.percentile(stats[nt].delays, 75)
            if stats[nt].delays.size > 0 else float('NaN'),
            np.percentile(stats[nt].delays, 95)
            if stats[nt].delays.size > 0 else float('NaN')
        ) for nt in [codel.NodeType.FTP, codel.NodeType.WEB, codel.NodeType.CBR]))

        result = cls._get_result(temp_dir)

        return np.array((seed, *result, *th_delay), dtype=cls.DTYPE)

    @staticmethod
    def _get_stats(queue_trace, mask):
        cur_trace = queue_trace[
            (queue_trace['event'] == QueueEventKind.DEQUEUE.value) & mask]

        return _Stats(departures=cur_trace['size'], delays=cur_trace['delay'])

    @staticmethod
    def _get_result(temp_dir):
        with open(path.join(temp_dir, 'stats'), 'r') as stats:
            return codel.CodelResult.from_str(stats.read())


class RewardTrace(Trace):
    DTYPE = np.dtype([
        ('interval_id', 'u8'), ('flow_id', 'u8'),
        ('bytes_transmitted', 'u8'), ('active_interval', 'f8'), ('delay', 'f8')
    ])

    name = "reward"

    @classmethod
    def parse(cls, ns2, temp_dir):
        return np.loadtxt(cls._get_trace_source(temp_dir), dtype=cls.DTYPE)

