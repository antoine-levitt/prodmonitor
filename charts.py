# -*- coding: utf-8 -*-


class Chart:
    """Class handling charts (build and write to file)."""

    def __init__(self):
        self.series = dict()
        self.series_label = []

    def add(self, title, start, stop, time_spent, switchs_count):
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
        #TODO keep the order of first creation... to keep same colors for same titles
        all_series = "var all_series = [\n"
        output = ""
        count = 0
        for title in self.series_label:
            count += 1
            # add data
            output += "var d%d = [ %s ];" % (count, self.series[title])
            # add series
            # TODO escape " from title, or do it when retrieve it?
            if count > 1:
                all_series += ",\n"
            all_series += "  { label: \"%s\", data: d%s }" % (title, count) 

        all_series += "\n];"

        f = open(filename, 'w')
        f.write(output + "\n\n" + all_series)
        f.close()

    
