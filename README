rmw (ReMove to Waste) functions as a command line recycle bin/trash can 
utility. Optionally, it can ReMove files to Desktop trash and has been 
tested on GNOME, KDE, and Xfce. This is the C source code, not the 
bash script

rmw will read the system wide configuration file if $HOME/config/rmw
isn't present. (./configure --sysconfdir=...) If sysconfdir isn't specified
rmw defaults to /usr/local/etc

A generic configuration file looks like this:

WASTE = $HOME/.trash.rmw
#WASTE = $HOME/.local/share/Trash
purgeDays = 90

Purging can be disabled by using 'purgeDays = 0' in configuration file.

The C version of rmw can remove files in desktop trash automatically
after 'x' number of days.

rmw options:

-c, --config filename     use an alternate configuration
-l, --list                list waste directories
-a, --add-waste dir       add waste directory to configuration file
-p, --pause               wait for a keypress before exiting
-g, --purge               run purge even if it's been run today
-z, --restore <wildcard filename(s) pattern>
-s, --select              select files from list to restore
-u, --undo-last           undo last ReMove
-B, --bypass              bypass directory protection
-v, --verbose             increase output messages
-w, --warranty            display warranty
-V, --version             display version and license information

This file was last updated Jan 22, 2016

