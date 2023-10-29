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

#ifndef _servo_h
#define _servo_h

#include "shared_data.h"

typedef struct {
	int channel;
	shared_data_t *data;
}servo_t;

void set_servo_pos(servo_t *s, struct rpc_cmd_list *cmd_list, int pos);
int get_servo_pos(servo_t *s);
void init_servo(servo_t *s);
void deinit_servo(void *arg);

#endif
