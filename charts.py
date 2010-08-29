# -*- coding: utf-8 -*-

# Copyright (C) 2010 Thomas Riccardi

# Author: Thomas Riccardi <riccardi.thomas@gmail.com>
# URL: http://github.com/antoine-levitt/prodmonitor/tree/sqlite

# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.


class Chart:
    """Class handling charts (build and write to file)."""

    def __init__(self, width):
        """Create a Chart object with width entries."""
        self.series = {}
        self.series_label = []
        self.width = width

    def add(self, title, start, stop, switchs_count, time_spent):
        """Add entries to chart."""
        if not title in self.series:
            self.series_label.append(title)
            self.series[title] = ""
        else:
            self.series[title] += ", "

        self.series[title] += "[%d, %d]" % (start * 1000 + 2*60*60*1000, time_spent) # fix time with local time
        #TODO use other values


    def write_file(self, filename):
        """Write the charts file."""
        datasets = "var datasets = {\n"
        output = ""
        count = 0
        for title in self.series_label:
            count += 1
            # add data
            output += "var d%d = [ %s ];" % (count, self.series[title])
            # add series
            # TODO escape " from title, or do it when retrieve it?
            if count > 1:
                datasets += ",\n"
            datasets += "  \"%s\": { label: \"%s\", data: d%s }" % (title, title, count)

        datasets += "\n};"
        barWidth = "var barWidth = %d;" % (self.width*1000)

        f = open(filename, 'w')
        f.write(output + "\n\n" + datasets + "\n\n" + barWidth)
        f.close()

    
