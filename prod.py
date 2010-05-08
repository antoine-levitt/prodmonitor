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

titles = ["%Google Chrome", "%Iceweasel", "XChat%", "Guake!", "%emacs@kiwi-pc2", "%SMPlayer", "%Sauerbraten"]

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
#TODO sync on round times?
#TODO add to data entries that overlap one limit, or both

for date in range(start_date, stop_date, step):

    data = db.select_stats(titles, date, date+step-1)
    titles_other = list(titles)
    titles_other.append("Other")
    for title in titles_other:
        if title in data:
            switchs_count = data[title][0]
            time_spent = data[title][1]
        else:
            switchs_count = 0
            time_spent = 0
        chart.add(title, date, date+step-1, switchs_count, time_spent)



chart.write_file('html/data.js')


db.close()
