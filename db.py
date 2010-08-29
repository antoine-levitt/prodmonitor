# -*- coding: utf-8 -*-

# Copyright (C) 2010 Thomas Riccardi

# Author: Thomas Riccardi <riccardi.thomas@gmail.com>
# URL: http://github.com/antoine-levitt/prodmonitor/tree/sqlite

# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.


import sqlite3


def connect(databasename, tablename):
    """Connect to database."""
    global c, table

    table = tablename

    conn = sqlite3.connect(databasename)
    #conn.row_factory = sqlite3.Row
    c = conn.cursor()


def get_first_date():
    """Return the first date available."""

    c.execute("select start from %s order by start asc limit 1" % table)

    result = c.fetchall()

    return result[0][0]


def select_all():
    """Return all entries."""

    c.execute("select * from %s" % table)

    return c.fetchall()


def select_stats(titles_name, titles_match, start, stop):
    """Return stats entries grouped by titles."""

    print "start: %d, stop: %d" % (start, stop)
    sql_titles_like = ""
    sql_dict = {"start": start, "stop": stop}
    result_dict = {"Other": {"count": 0, "time": 0}}
    for i, title in enumerate(titles_match):
        if i > 0:
            sql_titles_like += ", "
        titlename = "title%d" % i
        sql_titles_like += "title like :%s" % titlename
        sql_dict[titlename] = title
        result_dict[titles_name[i]] = {"count": 0, "time": 0}


    # get stats for duration and switchs_count for each case regarding the given time range.
    # switchs_count are the number of entries starting in the time range

    # entries in the time range
    sql = "select count(*), sum(stop-start), %s, title from %s where start >= :start and stop <= :stop group by %s" % (sql_titles_like, table, sql_titles_like)

    # entries starting before, and ending in the time range
    sql += " UNION "
    sql += "select 0, sum(stop-:start), %s, title from %s where start < :start and stop > :start and stop <= :stop group by %s" % (sql_titles_like, table, sql_titles_like)

    # entries starting in the time range, and ending after
    sql += " UNION "
    sql += "select count(*), sum(:stop-start), %s, title from %s where start >= :start and start < :stop and stop > :stop group by %s" % (sql_titles_like, table, sql_titles_like)

    # entries starting before and ending after the time range
    sql += " UNION "
    sql += "select 0, sum(:stop-:start), %s, title from %s where start < :start and stop > :stop group by %s" % (sql_titles_like, table, sql_titles_like)


    # and now, execute!
    c.execute(sql, sql_dict)
    result = c.fetchall()

    if len(result) == 0:
        return result_dict


    # we now have a boolean list on each row that says witch title it matched, preceded by the stats data
    # build the clean results as dict
    for row in result:
        assert len(titles_name) == -2+len(row)-1, (len(titles_name), len(row), titles_name, row)

        # TODO optimize order if needed (and if we handle only one match per result)
        # the order is a loop(4 times) on:
        #  first row is "other", unless every entry has been matched by titles
        #  other rows are in reverse order of titles
        gotOne = False
        for i in range(len(titles_name)):
            if row[2+i] == 1:
                # got a match
                gotOne = True
                result_dict[titles_name[i]]["count"] += row[0]
                result_dict[titles_name[i]]["time"] += row[1]
        if not gotOne:
            # this is the "Other" results
            result_dict["Other"]["count"] += row[0]
            result_dict["Other"]["time"] += row[1]

    print result_dict

    return result_dict


def close():
    """Close the connexion."""
    c.close()
