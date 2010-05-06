# -*- coding: utf-8 -*-

import sqlite3


def connect(databasename, tablename):
    """Connect to database."""
    global c, table

    table = tablename

    conn = sqlite3.connect(databasename)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()


def select_all():
    """Return all entries."""

    c.execute("select * from %s" % table)

    return c.fetchall()


def select(title):
    """Return entries matching title."""

    c.execute("select title, start, stop from %s where title like :title" % table, {"title": title})

    return c.fetchall()


def close():
    """Close the connexion."""
    c.close()
