#!/usr/bin/env python2
# -*- coding: utf-8 -*-

# program for traybar icone: start and stop ./prod record daemon

# Copyright (C) 2010 Thomas Riccardi

# Author: Thomas Riccardi <riccardi.thomas@gmail.com>
# URL: http://github.com/antoine-levitt/prodmonitor/tree/sqlite

# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.


import pygtk
pygtk.require('2.0')
import gtk
import keybinder
import subprocess
import os

instance = None

class Service():
    """Abstract class representing a service: Start, Stop."""
    def __init__(self):
        """Init."""
        pass
    
    def Start(self):
        """Start the service."""
        assert not self.isStarted()

    def Stop(self):
        """Stop the service."""
        assert self.isStarted()

    def isStarted(self):
        """Is the service started."""
        pass


class ProdMonService(Service):
    """ProdMon service."""
    def __init__(self, program):
        """Init."""

        if type(program) is str:
            program_tmp = [program]
        elif type(program) is list:
            program_tmp = program
        else:
            assert False, ("program is of wrong type", program)
        
        # compute real program path
        program_real_path = os.path.expandvars(program_tmp[0])
        program_real_path = os.path.expanduser(program_real_path)
        program_real_path = os.path.abspath(program_real_path)
        self.program = [program_real_path]
        self.program.extend(program_tmp[1:])
        assert os.access(program_real_path, os.X_OK), ("program is not executable", program_real_path)

        self.p = None
    
    def Start(self):
        """Start the service."""
        Service.Start(self)
        # start the process
        #self.p = subprocess.Popen(self.program, shell=True)
        #use no shell, because p.pid is the pid of the shell..

        print "Starting process", self.program
        self.p = subprocess.Popen(self.program, shell=False)

        
    def Stop(self):
        """Stop the service."""
        Service.Stop(self)
        # stop the process
        #self.p.terminate() # only since python 2.6
        # hack for python < 2.6
        s = "kill %s" % (self.p.pid)

        subprocess.Popen('kill %s' % self.p.pid, shell=True)
        self.p = None

    def isStarted(self):
        """Is the service started."""
        return self.p != None



class ProdMonTray():
    """ProdMonTray main class. Creates the tray icon, and handle global shortcut."""
    def __init__(self):
        # options
        self.keystr = "<Super>W"
        self.blink = True
        self.prodmon_program = ["prod", "-q"]

        # main info
        self.running = True

        # trayicon
        img_file_running = 'prodmon-tray.png'
        img_file_away = 'prodmon-away-tray.png'
        self.tray_img_running = gtk.gdk.pixbuf_new_from_file(img_file_running)
        self.tray_img_away = gtk.gdk.pixbuf_new_from_file(img_file_away)

        self.tray_icon = gtk.status_icon_new_from_pixbuf(self.tray_img_running)
        self.tray_icon.set_tooltip('ProdMon')
        # self.tray_icon.connect('popup-menu', self.show_menu)
        # self.tray_icon.connect('activate', self.show_hide)

        # bind key
        keybinder.bind(self.keystr, toggleProdMonTray, None)

        # register the services
        self.services = []
        self.services.append(ProdMonService(self.prodmon_program))

    def setAway(self):
        """Set away."""
        self.running = False
        self.tray_icon.set_from_pixbuf(self.tray_img_away)
        self.tray_icon.set_blinking(self.blink)
        # stop the services
        for s in self.services:
            if s.isStarted():
                s.Stop()

    def setRunning(self):
        """Set running."""
        self.running = True
        self.tray_icon.set_from_pixbuf(self.tray_img_running)
        self.tray_icon.set_blinking(False)
        # start the services
        for s in self.services:
            if not s.isStarted():
                s.Start()

    def toggle(self):
        """Toggle between away and running."""
        if self.running:
            self.setAway()
        else:
            self.setRunning()

            
def toggleProdMonTray(data):
    instance.toggle()

def main():
    """Parse command line, and start everything."""
    global instance
    # cd script directory
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    instance = ProdMonTray()
    instance.setRunning()

if __name__ == '__main__':
    main()
    gtk.main()
