#! /usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import db

# config
db_file = './test.db'
db_table = 'stats'


# parse arguments
if len(sys.argv) < 2:
    print "Usage : stats.py 'title' [<start date> [<stop date>]]"
    sys.exit(1)

title = sys.argv[1]

if len(sys.argv) > 2:
    start_date = sys.argv[2]
if len(sys.argv) > 3:
    stop_date = sys.argv[3]


# start the work
db.connect(db_file, db_table)

allentries = db.select_all()
print "All entries, count=%d" % len(allentries)

entries = db.select(title)
time_spent = 0
switchs_count = len(entries)
for row in entries:
    time_spent += row["stop"] - row["start"]

print "Total time spent on '%s': %ds (%d switchs, average: %ds per view)" % (title, time_spent, switchs_count, time_spent/switchs_count)


db.close()
