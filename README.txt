rmw (ReMove to Waste) functions as a command line recycle bin/trash can
utility. Optionally, it can ReMove files to Desktop trash.

rmw will read the system wide configuration file if $HOME/.config/rmw/config
isn't present. (./configure --sysconfdir=...) If sysconfdir isn't specified
rmw defaults to /usr/local/etc

A generic configuration file looks like this:

WASTE = $HOME/.trash.rmw
#WASTE = $HOME/.local/share/Trash
purgeDays = 90

If purging is 'on', rmw will permanently delete files from the folders
specified in the configuration file after 'x' number of days. Purging
can be disabled by using 'purgeDays = 0' in configuration file.

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

Web site: http://rmw.sf.net/

IRC: Quakenet IRC network - #rmw
http://webchat.quakenet.org/?channels=rmw

This file was last updated Aug 04, 2016

