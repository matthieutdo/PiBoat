/*************************************************************************
 *	PiBoat project: this programm is used for control a RC boat with a 
 *	Raspberry Pi.
 *
 *	Note:	this programm use the WiringPi library (LGPLv3)
 *			see https://github.com/WiringPi/WiringPi
 *
 *	Copyright (C) 2014-2024  TERNISIEN d'OUVILLE Matthieu
 *	
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *	Author: TERNISEN d'OUVILLE Matthieu <matthieu.tdo@gmail.com>
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

#include <wiringPi.h>

#include "shared_data.h"
#include "pwm.h"
#include "thread_manager.h"

#define PIDFILE    "/var/run/piboat.pid"

static const char piboat_cmd_opt[] =
	"h"  /* help */
	"v"  /* version */
	"d:" /* set log level */
	"f"  /* run in foreground */
	;

static void init_data(shared_data_t *d)
{
	d->pwm = -1;
	pthread_mutex_init(&(d->pwm_mutex), NULL);
}

static void licence()
{
	/* GPLv3 licence */
	printf("PiBoat Copyright (C) 2014-2024 TERNISIEN d'OUVILLE Matthieu\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY. \n");
	printf("This is free software, and you are welcome to redistribute it under certain\n");
	printf("conditions; see https://www.gnu.org/copyleft/gpl.html for details.\n");
	printf("\n");
}

static void help(const char *prog_name)
{
	licence();
	printf("Usage: %s [-%s]\n", prog_name, piboat_cmd_opt);
	printf("  -h (help)           Display this help\n");
	printf("  -v (version)        Display version\n");
	printf("  -d LEVEL (debug)    Set log level\n");
	printf("  -f (foreground)     Run in foreground\n");
	printf("\n");
	printf("LEVEL:\n");
	printf("  0      No log\n");
	printf("  1      EMERGENCY only\n");
	printf("  2      ALERT and upper\n");
	printf("  3      CRITICAL and upper\n");
	printf("  4      ERROR and upper (the default)\n");
	printf("  5      WARNING and upper\n");
	printf("  6      NOTICE and upper\n");
	printf("  7      INFO and upper\n");
	printf("  8      DEBUG and upper\n");
	printf("\n");
}

static void version()
{
	licence();
	printf("PiBoat v%s\n", VERSION);
	printf("\n");
}

static int daemonize(int foreground)
{
	FILE *f;

	if (foreground)
		return 0;

	errno = 0;
	if (daemon(0, 0)) {
		fprintf(stderr, "Cannot daemonize error: %s\n",
			strerror(errno));
		return -1;
	}

	f = fopen(PIDFILE, "w");
	if (f == NULL) {
		fprintf(stderr, "Cannot write pidfile\n");
		return 0; /* ignore error */
	}

	fprintf(f, "%u", getpid());
	fclose(f);
	return 0;
}

int main(int argc, char* argv[])
{
	int err, opt;
	shared_data_t data;
	int log_mask;
	int foreground;

	log_mask = (1 << 4) - 1; /* Display error and upper logs only. */
	foreground = 0;

	while ((opt = getopt(argc, argv, piboat_cmd_opt)) != -1) {
		switch (opt) {
		case 'h':
			help(argv[0]);
			return 0;
		case 'v':
			version();
			return 0;
		case 'd':
		{
			unsigned long int lvl;
			char *end;

			errno = 0;
			lvl = strtoul(optarg, &end, 10);
			if (lvl > 8 || errno == ERANGE || *end != '\0') {
				fprintf(stderr, "Invalid log level '%s'\n",
					optarg);
				help(argv[0]);
				return -1;
			}

			log_mask = (1 << lvl) - 1;
			break;
		}
		case 'f':
			foreground = 1;
			break;
		default:
			help(argv[0]);
			return -1;
		}
	}

	if (daemonize(foreground))
		return -1;

	openlog(argv[0], LOG_NDELAY | LOG_PID, LOG_DAEMON);
	setlogmask(log_mask);

	init_data(&data);

	licence();

	syslog(LOG_INFO, "Initialisation...\n");

	/* Initialisation PWM board */
	data.pwm = init_pwm();

	if (data.pwm < 0) {
		syslog(LOG_EMERG, "PWM initialized                [FAILED]\n");
		return -1;
	}

	syslog(LOG_INFO, "PWM initialized                 [  OK  ]\n");

	/* initialisation GPIO */
	errno = 0;
	err = wiringPiSetupGpio();
	if (err == -1){
		syslog(LOG_EMERG, "GPIO setup error: %s\n", strerror(errno));
		syslog(LOG_EMERG, "GPIO initialized               [FAILED]\n");
		return -1;
	}

	syslog(LOG_INFO, "GPIO initialized                [  OK  ]\n");

	/* Create and execute thread */
	err = module_start(&data);
	if (err != 0) {
		syslog(LOG_EMERG, "Thread initialisation          [FAILED]\n");
		return -1;
	}

	syslog(LOG_INFO, "Thread initialisation           [  OK  ]\n");
	/* Wait thread termination */
	piboat_wait(&data);

	return 0;
}
