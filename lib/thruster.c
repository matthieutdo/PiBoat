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

#include <syslog.h>
#include <unistd.h>
#include <sys/queue.h>

#include "thruster.h"
#include "pwm.h"

#define SPEED_MODIFIER 5
#define DIR_FORWARD 0
#define DIR_BACKWARD 1

static void _set_thruster_direction(thruster_t *t, int dir)
{
	syslog(LOG_DEBUG, "Thruster %p switch direction %s -> %s", t,
	       t->cur_dir == DIR_FORWARD? "FORWARD": "BACKWARD",
	       dir == DIR_FORWARD? "FORWARD": "BACKWARD");

	t->cur_dir = dir;

	if (dir == DIR_FORWARD)
		digitalWrite(t->gpio_dir, LOW);
	else
		digitalWrite(t->gpio_dir, HIGH);
}

static int _set_thruster_speed(thruster_t *t, int speed)
{
	int pwm_value; /* , on, off; */

	if (t->cur_dir == DIR_FORWARD && speed < 0)
		_set_thruster_direction(t, DIR_BACKWARD);
	else if (t->cur_dir == DIR_BACKWARD && speed > 0)
		_set_thruster_direction(t, DIR_FORWARD);

	/* Low speed */
	if (speed == 0) {
		set_pwm(t->data, t->pwm_channel, 0, SPEED_LIM);
		goto end;
	}
	/* High speed */
	if (speed == 1000 || speed == -1000) {
		set_pwm(t->data, t->pwm_channel, SPEED_LIM, 0);
		goto end;
	}

	/* Other speed */
	pwm_value = (float)((SPEED_HIGH) - (SPEED_LOW)) * ((float)fabs(speed)/1000.0);
	pwm_value += SPEED_LOW;

	syslog(LOG_DEBUG, "Speed value: %i\n", pwm_value);

	set_pwm(t->data, t->pwm_channel, 0, pwm_value);

end:
	t->cur_speed = speed;

	return 0;
}

void set_thruster_speed(thruster_t *t, struct rpc_cmd_list *cmd_list, int speed)
{
	int speed_diff, speed_mod, speed_new;

	if (speed < 0)
		speed += t->adjust;
	else
		speed -= t->adjust;

	speed_diff = fabs(t->cur_speed - speed);
	speed_mod = SPEED_MODIFIER;
	if (t->cur_speed > speed)
		speed_mod = -SPEED_MODIFIER;

	syslog(LOG_DEBUG, "set thruster speed %i->%i diff = %i mode = %i",
	       t->cur_speed, speed, speed_diff, speed_mod);

	while (speed_diff > 0) {
		if (speed_diff < SPEED_MODIFIER)
			speed_mod = (speed_mod > 0)? speed_diff: -speed_diff;

		speed_new = t->cur_speed + speed_mod;

		syslog(LOG_INFO, "set thruster %p speed %i (%i%c%i)", t, speed_new,
		       t->cur_speed, speed_mod > 0? '+' : '-', (int)fabs(speed_mod));

		_set_thruster_speed(t, speed_new);

		speed_diff -= SPEED_MODIFIER;
		if (speed_diff < 0)
			speed_diff = 0;

		usleep(250000);

		/* Interrupt command if a new one has been received. */
		if (!TAILQ_EMPTY(cmd_list))
			break;
	}
}

void init_thruster(thruster_t *t)
{
	pinMode(t->gpio_enable, OUTPUT);
	pinMode(t->gpio_dir, OUTPUT);

	digitalWrite(t->gpio_enable, HIGH);

	t->adjust = 0;
	_set_thruster_speed(t, 0);
	_set_thruster_direction(t, DIR_FORWARD);
}

void deinit_thruster(void *arg)
{
	thruster_t *t;

	t = (thruster_t *)arg;
	syslog(LOG_DEBUG, "Thruster %p cleanup...", t);

	_set_thruster_speed(t, 0);
	_set_thruster_direction(t, DIR_FORWARD);
	t->adjust = 0;
	digitalWrite(t->gpio_enable, LOW);
}
