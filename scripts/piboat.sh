#!/bin/sh
#
# Copyright 2014-2018 TERNISIEN d'OUVILLE Matthieu
# Start Piboat....
#

prog="piboat"
cmd=$(command -v $prog)
RET=0

if [ ! -x $cmd ]
then
	echo "$0: cannot find $prog at $cmd"
	exit -1
fi

case "$1" in
	start)
		echo "Starting Piboat: $cmd $@"
		${cmd} || RET=1
		if [ $RET -eq 1 ]
		then
			echo "$0: cannot start $cmd."
			exit $RET
		fi
	;;
	stop)
		echo "Stopping Piboat: "
		kill -2 `cat /var/run/piboat.pid`
	;;
	restart)
		echo "Restart service..."
		${0} stop
		${0} start
	;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 1
esac
