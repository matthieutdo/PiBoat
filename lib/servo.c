/*************************************************************************
 *	Copyright (C) 2018-2024  TERNISIEN d'OUVILLE Matthieu
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

#include <math.h>
#include <syslog.h>
#include <unistd.h>

#include "pwm.h"
#include "shared_data.h"
#include "rpc.h"
#include "servo.h"

#define POS_MODIFIER 1

#define DEG_0    180
#define DEG_180  520

void _set_servo_pos(shared_data_t *data, int servo, int pos)
{
	int pwm_value;

	pwm_value = (int)roundf((float)((DEG_180) - (DEG_0)) * ((float)fabs(pos) / 180.0));
	pwm_value += DEG_0;

	syslog(LOG_DEBUG, "New servo %i PWM: on=0 off=%i\n", servo, pwm_value);

	set_pwm(data, servo, 0, pwm_value);
}

void set_servo_pos(shared_data_t *data, int servo, struct rpc_cmd_list *cmd_list, int pos)
{
	int pos_diff, pos_mod, pos_new, cur_pos;

	cur_pos = get_servo_pos(data, servo);
	pos_diff = fabs(cur_pos - pos);
	pos_mod = POS_MODIFIER;
	if (cur_pos > pos)
		pos_mod = -POS_MODIFIER;

	syslog(LOG_INFO, "set servo %i pos %i->%i diff = %i mode = %i",
	       servo, cur_pos, pos, pos_diff, pos_mod);

	while (pos_diff > 0) {
		if (pos_diff < POS_MODIFIER)
			pos_mod = (pos_mod > 0)? pos_diff: -pos_diff;

		pos_new = cur_pos + pos_mod;

		syslog(LOG_INFO, "set pos %i pos %i (%i%c%i)", servo, pos_new,
		       cur_pos, pos_mod > 0? '+' : '-', (int)fabs(pos_mod));

		_set_servo_pos(data, servo, pos_new);
		cur_pos = get_servo_pos(data, servo);

		pos_diff -= POS_MODIFIER;

		usleep(25000);

		/* Interrupt command if a new one has been received. */
		if (!TAILQ_EMPTY(cmd_list))
			break;
	}
}

int get_servo_pos(shared_data_t *data, int servo)
{
	int cur_pos;
	int on, off;

	get_pwm(data, servo, &on, &off);
	cur_pos = (int)roundf(((float)(off - DEG_0) / (float)(DEG_180 - DEG_0)) * 180.0);

	return cur_pos;
}
