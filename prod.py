#! /usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import time
import db
from charts import Chart
from optparse import OptionParser

# config
db_file = './test.db'
db_table = 'stats'

db.connect(db_file, db_table)

# parse arguments
parser = OptionParser()
parser.set_defaults(start=db.get_first_date(), end=time.time(), step_number=96)
parser.add_option("-s", "--start",
                  action="store", type="int", dest="start",
                  help="The starting date")
parser.add_option("-e", "--end",
                  action="store", type="int", dest="end",
                  help="The ending date")
parser.add_option("-n", "--step-number",
                  action="store", type="int", dest="step_number",
                  help="The number of steps on the chart")
#TODO add verbose, quiet?

(options, args) = parser.parse_args()

# TODO titles as option?
titles = [("Google Chrome", "%Google Chrome"),
          ("Iceweasel", "%Iceweasel"),
          ("XChat", "XChat%"),
          ("Guake!", "Guake!"),
          ("emacs", "%emacs@kiwi-pc2"),
          ("SMPlayer", "%SMPlayer"),
          ("Sauerbraten", "%Sauerbraten")]


print "Start date : %d" % options.start
print "End date   : %d" % options.end
print "Step number: %d" % options.step_number

# start the work

# warm up
allentries = db.select_all()
print "All entries, count=%d" % len(allentries)


# lets go
options.step = (options.end - options.start)/options.step_number
chart = Chart(options.step)
#TODO sync on round times?

titles_name = map(lambda t: t[0], titles)
titles_match = map(lambda t: t[1], titles)
titles_name2 = titles_name[:]
titles_name2.append("Other")

for date in range(options.start, options.end, options.step):

    data = db.select_stats(titles_name, titles_match, date, date+options.step-1)
    for title in titles_name2:
        chart.add(title, date, date+options.step-1, data[title]["count"], data[title]["time"])



chart.write_file('html/data.js')


db.close()
