# -*- coding: utf-8 -*-


def stats(entries):
    """Do stats on given entries, returns switchs_count and total time spent."""

    time_spent = 0
    switchs_count = len(entries)

    for row in entries:
        time_spent += row["stop"] - row["start"]

    return switchs_count, time_spent
