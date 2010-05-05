#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sqlite3

conn = sqlite3.connect('./test.db')
c = conn.cursor()

c.execute('select * from stats')
for row in c:
    print row

c.close()
