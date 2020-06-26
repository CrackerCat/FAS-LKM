===========================
 File Access with Sessions
===========================

  Written and maintained by Andrea Fioraldi <andreafioraldi@gmail.com>

  Copyright 2019 Andrea Fioraldi. All rights reserved.
  Released under terms and conditions of the GNU General Public License,
  Version 2.0. A copy of the license is provided in the LICENSE.txt file.

A Linux Kernel subsystem which allows to access files in a specific directory
of the virtual file system using sessions.

Project made for the Advanced Operating Systems and Virtualization class of
Sapienza University of Rome.

See docs/REPORT.txt for details.

1) Building
-----------

$ make

To build the module and libfas in debug mode. Note that this will log a lot
of data in dmesg.

$ RELEASE=1 make

To build the module and libfas without debug logs.

$ cd test ; make

To build the test binaries.

The FAS kernel module is known to compile and work for kernel versions >= 5.0.

2) Running
----------

Load the module with

# insmod src/fas.ko

Unload it with

# rmmod fas

The easy-peasy way to have session files in you l33t Linux box without doing
almost nothing is to preload test/test_preloader.so into an existing
application like bash.

Note that you have to write / into /sys/kernel/fas/initial_path to allow
session files on the enire system.
A requirement is that the application that uses FAS must have the permission to
write into initial_path, so if you want to se initial_path to / you have to
run the applications as root (be careful!!!).

The commands are:

# echo / > /sys/kernel/fas/initial_path
# LD_PRELOAD=/full/path/to/test_preloader.so /bin/bash

