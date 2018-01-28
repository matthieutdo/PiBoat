/*************************************************************************
 *	Copyright (C) 2014  TERNISIEN d'OUVILLE Matthieu
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

#include "direction.h"

static const int PIN_SERVO = 0;

static const int DEG_0 = 160;
static const int DEG_180 = 600;

static const int MIN = 40;
static const int MAX = 140;

static int deg_adjust = 0;

void init_direction(shared_data_t *data)
{
	set_direction(data, 90); /*  pwm_off = 380 */
}

void deinit_direction(shared_data_t *data)
{
	set_direction(data, 90); /*  pwm_off = 380 */
}

void set_direction(shared_data_t *data, int pos)
{
	int pwm_value;

	pos += deg_adjust;

	if (pos<MIN) pos = MIN;
	else if (pos>MAX) pos = MAX;

	print_debug(stdout, "new_pos: %i\n", pos);

	pwm_value = (float)((DEG_180) - (DEG_0)) * ((float)fabs(pos)/180.0);
	pwm_value += DEG_0;
	set_pwm(data, PIN_SERVO, 0, pwm_value);
}

void get_direction(shared_data_t *data, int *pos)
{
	int on, off;

	get_pwm(data, PIN_SERVO, &on, &off);
	*pos = ((float)(off-DEG_0) / (float)(DEG_180-DEG_0)) * 180.0;
	*pos -= deg_adjust;
}

int set_dir_adjust(shared_data_t *data, int new_adj)
{
	int cur_pos;

	get_direction(data, &cur_pos);
	deg_adjust = new_adj;
	set_direction(data, cur_pos);

	return 0;
}

void get_dir_adjust(shared_data_t *data, int *adj)
{
	*adj = deg_adjust;
}
