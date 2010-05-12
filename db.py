# -*- coding: utf-8 -*-

import sqlite3


def connect(databasename, tablename):
    """Connect to database."""
    global c, table

    table = tablename

    conn = sqlite3.connect(databasename)
    #conn.row_factory = sqlite3.Row
    c = conn.cursor()


def select_all():
    """Return all entries."""

    c.execute("select * from %s" % table)

    return c.fetchall()


def select_stats(titles_name, titles_match, start, stop):
    """Return stats entries grouped by titles."""

    print "start: %d, stop: %d" % (start, stop)
    sql_titles_like = ""
    sql_dict = {"start": start, "stop": stop}
    for i, title in enumerate(titles_match):
        if i > 0:
            sql_titles_like += ", "
        titlename = "title%d" % i
        sql_titles_like += "title like :%s" % titlename
        sql_dict[titlename] = title

    sql = "select count(*), sum(stop-start), %s, title from %s where start >= :start and stop <= :stop group by %s" % (sql_titles_like, table, sql_titles_like)

    c.execute(sql, sql_dict)
    result = c.fetchall()

    if len(result) == 0:
        return {}

    # we now have a boolean list on each row that says witch title it matched, preceded by the stats data
    # build the clean results as dict
    count = 0
    result_dict = {}
    # first row is "other", unless every entry has been matched by titles
    if sum(result[count][2:-1]) == 0:
        result_dict["Other"] = (result[count][0], result[count][1])
        count += 1
    # other rows are in reverse order of titles
    i_title = list(enumerate(titles_name))
    i_title.reverse()
    for i, title in i_title:
        if count >= len(result):
            # finished, so remaining titles were not matched
            break
        print count, i, title, result[count]
        # but maybe there was no match for a title, in which case sqlite returns nothing
        if result[count][2+i] == 0:
            # this is the case for this title
            continue
        # we have our title match
        assert result[count][2+i] == 1, (count, result[count]) # check this fact
        result_dict[title] = (result[count][0], result[count][1])
        count += 1

    assert len(result) == len(result_dict), (len(result), len(result_dict), result, result_dict)
    #TODO Fix this: if one real title match two given titles, this breaks everything... should we count them for each match title?

    print result_dict

    return result_dict


def close():
    """Close the connexion."""
    c.close()
