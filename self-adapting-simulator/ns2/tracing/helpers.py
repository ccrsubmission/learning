import numpy as np

import tracing
import run_codel


def do_print_basic_stats(trace):
    all_pairs, inverse = np.unique(trace[['src', 'dst']], return_inverse=True)

    for i in range(all_pairs.size):
        num_departures = trace['size'][
            (inverse == i) &
            (trace['event'] == tracing.QueueEventKind.DEQUEUE.value)
        ].sum()
        mean_delay = trace['delay'][
            (inverse == i) &
            (trace['event'] == tracing.QueueEventKind.DEQUEUE.value)
        ].mean()
        print(f'flow {all_pairs[i]}: {num_departures}, {mean_delay}')


def load_stats_for(ns2, run=None):
    if run is not None:
        ns2.codel_variants = run
        return load_stats_for(ns2)

    return np.stack(list(tracing.StatsTrace.load_all_seeds(ns2, run_codel.PERSISTENCE)))


def load_all_learning(ns2, field):
    return np.stack(list(
        x[field] for x in tracing.LearningTrace.load_all_seeds(ns2, run_codel.PERSISTENCE)
    ))
