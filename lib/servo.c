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

#include "pwm.h"
#include "shared_data.h"
#include "servo.h"

#define DEG_0    180
#define DEG_180  520

void set_servo_pos(shared_data_t *data, int servo, int new_pos)
{
	int pwm_value;

	pwm_value = (float)((DEG_180) - (DEG_0)) * ((float)fabs(new_pos) / 180.0);
	pwm_value += DEG_0;

	syslog(LOG_DEBUG, "New servo PWM: on=0 off=%i\n", pwm_value);

	set_pwm(data, servo, 0, pwm_value);
}

int get_servo_pos(shared_data_t *data, int servo)
{
	int cur_pos;
	int on, off;

	get_pwm(data, servo, &on, &off);
	cur_pos = ((float)(off - DEG_0) / (float)(DEG_180 - DEG_0)) * 180.0;

	return cur_pos;
}
