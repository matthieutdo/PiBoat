/*************************************************************************
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

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include "pwm.h"
#include "shared_data.h"
#include "rpc.h"

static const int SPEED_LOW = 0;
static const int SPEED_HIGH = 4095;
static const int SPEED_LIM = 1<<12|0;	/* For full on or full off(pwm value) */

struct thruster {
	int gpio_enable;
	int gpio_dir;
	int pwm_channel;
	int cur_speed;
	int adjust;
};

static struct thruster thruster = {22, 24, 8, 0, 0};

/* TODO test système de reglage... */

/**************************************************************
 *	Modification de la vitesse d'un moteur
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param *m		Pointeur sur les donnees du moteur
 *	@param speed		Nouvelle vitesse du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int thruster_speed(shared_data_t *data, struct thruster *m, int speed)
{
	int pwm_value; /* , on, off; */

	/* Low speed */
	if (speed == 0) {
		set_pwm(data, m->pwm_channel, 0, SPEED_LIM);
		return 0;
	}
	/* High speed */
	if (speed == 1000 || speed == -1000) {
		set_pwm(data, m->pwm_channel, SPEED_LIM, 0);
		return 0;
	}

	/* Other speed */
	pwm_value = (float)((SPEED_HIGH) - (SPEED_LOW)) * ((float)fabs(speed)/1000.0);
	pwm_value += SPEED_LOW;

	syslog(LOG_DEBUG, "Speed value: %i\n", pwm_value);

	set_pwm(data, m->pwm_channel, 0, pwm_value);

	m->cur_speed = speed;

	return 0;
}

/**************************************************************
 *	Modification du sens d'un moteur
 *
 *	@param m		Donnees du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int thruster_switch_direction(struct thruster m)
{
	if (m.cur_speed > 0) {
		digitalWrite(m.gpio_enable, LOW);
		digitalWrite(m.gpio_dir, HIGH);
	} else {
		digitalWrite(m.gpio_enable, HIGH);
		digitalWrite(m.gpio_dir, LOW);
	}

	return 0;
}

static int set_thruster_speed(shared_data_t *data, int speed)
{
	if (speed < 0)
		speed += thruster.adjust;
	else
		speed -= thruster.adjust;

	syslog(LOG_DEBUG, "Real speed: %i\n", speed);

	/* Switch direction */
	if ((thruster.cur_speed < 0 && speed > 0) ||
	    (thruster.cur_speed > 0 && speed < 0))
		thruster_switch_direction(thruster);

	/* Update speed */
	thruster_speed(data, &thruster, speed);

	return 0;
}

static int init_thruster(shared_data_t *data)
{
	pinMode(thruster.gpio_enable, OUTPUT);
	pinMode(thruster.gpio_dir, OUTPUT);

	digitalWrite(thruster.gpio_enable, LOW);
	digitalWrite(thruster.gpio_dir, HIGH);

	set_thruster_speed(data, 0);
	thruster_switch_direction(thruster);

	return 0;
}

static void deinit_thruster(shared_data_t *data)
{
	set_thruster_speed(data, 0);
	digitalWrite(thruster.gpio_enable, HIGH);
	digitalWrite(thruster.gpio_dir, LOW);
}

static int set_thruster_adjust_arg(int argc, char *argv[], shared_data_t *data)
{
	long int adjust;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Thruster adjust RPC: too few arguments *ma <-100-100>*\n");
		return -1;
	}

	adjust = strtol(argv[2], &end, 10);
	if (adjust < -30 || adjust > 30 || *end != '\0') {
		syslog(LOG_ERR, "Thruster adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	thruster.adjust = adjust;

	return 0;
}

static int set_thruster_speed_arg(int argc, char *argv[], shared_data_t *data)
{
	long int speed;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Thruster speed RPC: too few arguments *ms <-1000-1000>*\n");
		return -1;
	}

	speed = strtol(argv[1], &end, 10);
	if (speed < -1000 || speed > 1000 || *end != '\0') {
		syslog(LOG_ERR, "Thruster speed RPC: invalid argument 1 %s",
		       argv[1]);
		return -1;
	}

	return set_thruster_speed(data, (int)speed);
}

static rpc_t thruster_speed_rpc = {
	.cmd_name = "ms",
	.init = init_thruster,
	.cmd_set = set_thruster_speed_arg,
	.deinit = deinit_thruster,
};

static rpc_t thruster_adjust_rpc = {
	.cmd_name = "ma",
	.init = NULL,
	.cmd_set = set_thruster_adjust_arg,
	.deinit = NULL,
};

static void init_thruster_rpc(void) __attribute__((constructor));
void init_thruster_rpc(void)
{
	register_rpc(&thruster_speed_rpc);
	register_rpc(&thruster_adjust_rpc);
}
