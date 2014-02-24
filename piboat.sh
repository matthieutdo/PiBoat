#!/bin/sh
#
# Start Piboat....
#
  
 
case "$1" in
	start)
		echo -n "Starting Piboat: "
		echo "Service start" 1>/root/piboat/prog_test/stdout.log 2>/root/piboat/prog_test/stderr.log
		/usr/bin/piboat 1>>/root/piboat/prog_test/stdout.log 2>>/root/piboat/prog_test/stderr.log &
	;;
	stop)
		echo -n "Stopping Piboat: "
		kill -2 `pgrep piboat`
	;;
	restart|reload)
		echo "Restart service..."
		"$0" stop
		"$0" start
	;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 1
esac
  
exit $?

