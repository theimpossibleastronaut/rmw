#
# Regular cron jobs for the rmw package
#
0 4	* * *	root	[ -x /usr/bin/rmw_maintenance ] && /usr/bin/rmw_maintenance
