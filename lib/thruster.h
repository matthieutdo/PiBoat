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

#ifndef _thruster_h
#define _thruster_h

#include <shared_data.h>
#include <rpc.h>

#define SPEED_LOW 0
#define SPEED_HIGH 4095
#define SPEED_LIM 1<<12|0	/* For full on or full off(pwm value) */

typedef struct {
	int gpio_enable;
	int gpio_dir;
	int pwm_channel;
	int cur_speed;
	int cur_dir;
	int adjust;
} thruster_t;

void set_thruster_speed(thruster_t *t, shared_data_t *data,
			struct rpc_cmd_list *cmd_list, int speed);
void init_thruster(thruster_t *t, shared_data_t *data);
void deinit_thruster(thruster_t *t, shared_data_t *data);
#endif
