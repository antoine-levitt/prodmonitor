# -*- coding: utf-8 -*-


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

    
