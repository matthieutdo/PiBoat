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

struct motor {
	int gpio_enable;
	int gpio_dir;
	int pwm_channel;
	int cur_speed;
	int adjust;
};

static struct motor motor = {22, 24, 8, 0, 0};

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
static int motor_speed(shared_data_t *data, struct motor *m, int speed)
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
static int motor_switch_direction(struct motor m)
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

static int set_motor_speed(shared_data_t *data, int speed)
{
	if (speed < 0)
		speed += motor.adjust;
	else
		speed -= motor.adjust;

	syslog(LOG_DEBUG, "Real speed: %i\n", speed);

	/* Switch direction */
	if ((motor.cur_speed < 0 && speed > 0) ||
	    (motor.cur_speed > 0 && speed < 0))
		motor_switch_direction(motor);

	/* Update speed */
	motor_speed(data, &motor, speed);

	return 0;
}

static int init_motor(shared_data_t *data)
{
	pinMode(motor.gpio_enable, OUTPUT);
	pinMode(motor.gpio_dir, OUTPUT);

	digitalWrite(motor.gpio_enable, LOW);
	digitalWrite(motor.gpio_dir, HIGH);

	set_motor_speed(data, 0);
	motor_switch_direction(motor);

	return 0;
}

static void deinit_motor(shared_data_t *data)
{
	set_motor_speed(data, 0);
	digitalWrite(motor.gpio_enable, HIGH);
	digitalWrite(motor.gpio_dir, LOW);
}

static int set_motor_adjust_arg(int argc, char *argv[], shared_data_t *data)
{
	long int adjust;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Motor adjust RPC: too few arguments *ma <-100-100>*\n");
		return -1;
	}

	adjust = strtol(argv[2], &end, 10);
	if (adjust < -30 || adjust > 30 || *end != '\0') {
		syslog(LOG_ERR, "Motor adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	motor.adjust = adjust;

	return 0;
}

static int set_motor_speed_arg(int argc, char *argv[], shared_data_t *data)
{
	long int speed;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Motor speed RPC: too few arguments *ms <-1000-1000>*\n");
		return -1;
	}

	speed = strtol(argv[1], &end, 10);
	if (speed < -1000 || speed > 1000 || *end != '\0') {
		syslog(LOG_ERR, "Motor speed RPC: invalid argument 1 %s",
		       argv[1]);
		return -1;
	}

	return set_motor_speed(data, (int)speed);
}

static rpc_t motor_speed_rpc = {
	.cmd_name = "ms",
	.init = init_motor,
	.cmd_set = set_motor_speed_arg,
	.deinit = deinit_motor,
};

static rpc_t motor_adjust_rpc = {
	.cmd_name = "ma",
	.init = NULL,
	.cmd_set = set_motor_adjust_arg,
	.deinit = NULL,
};

static void init_motors_rpc(void) __attribute__((constructor));
void init_motors_rpc(void)
{
	register_rpc(&motor_speed_rpc);
	register_rpc(&motor_adjust_rpc);
}
