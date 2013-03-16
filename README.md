ProdMon
=======

ProdMon is a productivity monitor in that it monitors time spent on
programs, by window titles.

It is composed of several parts:

prod
----

A C program that monitors the windows changes, and stores them with
date in a sqlite database.

prodMonTray.py
--------------

A python program that handles start and stop of prod: it provides a
keyboard shortcut <super>-x to toggle prod, and displays a tray icon
accordingly.

prod.py
-------

A python program that parses the stored data and generates html/js
graphs.

Help: ./prod.py -h


Requirements
============

* libwnck
* sqlite
* pygtk
* python-keybinder

Debian based distributions
--------------------------

apt-get install libwnck-dev libsqlite3-dev python-gtk2 python-keybinder

Archlinux
---------

pacman -S libwnck sqlite pygtk
aur/python-keybinder


Build
=====

make


Contact
=======

Thomas Riccardi <riccardi.thomas@gmail.com>