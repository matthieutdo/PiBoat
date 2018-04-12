/*************************************************************************
 *	Copyright (C) 2018  TERNISIEN d'OUVILLE Matthieu
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

#include <stdlib.h>
#include <errno.h>
#include <syslog.h>

#include "receive_rc.h"
#include "servo.h"

/*         y (horizon (max) = 170, min = 60)
 *         ^
 *         |
 *         |
 *   <-----------> x (center = 85, min = 0, max = 170)
 */

#define PIN_SERVO_X 2
#define PIN_SERVO_Y 3

static int init_cam(shared_data_t *data)
{
	servo_set_pos(data, PIN_SERVO_X, 85);
	servo_set_pos(data, PIN_SERVO_Y, 170);
	return 0;
}

static void deinit_cam(shared_data_t *data)
{
	servo_set_pos(data, PIN_SERVO_X, 85);
	servo_set_pos(data, PIN_SERVO_Y, 170);
}

static int set_cam_arg(int argc, char *argv[], shared_data_t *data) {
	int pos_x, pos_y;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "cam RPC: too few/many arguments *c <0-170> <60-170>*\n");
		return -1;
	}

	pos_x = strtol(argv[1], &end, 10);
	if (pos_x < 0 || pos_x > 170 || *end != '\0') {
		syslog(LOG_ERR, "Direction RPC: invalid horizontal axis argument %s",
		       argv[1]);
		return -1;
	}

	pos_y = strtol(argv[2], &end, 10);
	if (pos_y < 60 || pos_y > 170 || *end != '\0') {
		syslog(LOG_ERR, "Direction RPC: invalid vertical axis argument %s",
		       argv[2]);
		return -1;
	}

	servo_set_pos(data, PIN_SERVO_X, pos_x);
	servo_set_pos(data, PIN_SERVO_Y, pos_y);

	return 0;
}

static piboat_rpc_t cam_rpc = {
	.cmd_name = "c",
	.init = init_cam,
	.cmd_set = set_cam_arg,
	.deinit = deinit_cam,
};

static void init_piboat_cam(void) __attribute__((constructor));
void init_piboat_cam(void)
{
	register_piboat_rpc(&cam_rpc);
}
