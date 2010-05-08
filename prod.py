#! /usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import time
import db
import stats
from charts import Chart

# config
db_file = './test.db'
db_table = 'stats'


# parse arguments
# TODO use getopt
if len(sys.argv) < 1:
    print "Usage : prod.py [<start date> [<stop date>]]"
    sys.exit(1)

titles = ["%Google Chrome", "%Iceweasel", "XChat%", "Guake!"]

# by default: select all
start_date = 0
stop_date = time.time()
if len(sys.argv) > 1:
    start_date = int(sys.argv[1])
    print "Start date : %d" % start_date
if len(sys.argv) > 2:
    stop_date = int(sys.argv[2])
    print "Stop date  : %d" % stop_date


# start the work
db.connect(db_file, db_table)

# warm up
allentries = db.select_all()
print "All entries, count=%d" % len(allentries)


# lets go
chart = Chart()
step = 15*60 # stats 15mins

for date in range(start_date, stop_date, step):
    for title in titles:
        entries = db.select(title, date, date+step-1)
        switchs_count, time_spent = stats.stats(entries)
        chart.add(title, date, date+step-1, time_spent, switchs_count)



chart.write_file('html/data.js')


db.close()
