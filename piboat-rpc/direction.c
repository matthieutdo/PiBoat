/*************************************************************************
 *	Copyright (C) 2014-2018  TERNISIEN d'OUVILLE Matthieu
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
#include <syslog.h>

#include "pwm.h"
#include "shared_data.h"
#include "receive_rc.h"
#include "servo.h"

static const int PIN_SERVO_RIGHT = 0;
static const int PIN_SERVO_LEFT  = 1;

static const int MIN = 40;
static const int MAX = 140;

static int deg_adjust_right = 0;
static int deg_adjust_left  = 0;

static int set_direction(shared_data_t *data, int pos_right, int pos_left)
{
	pos_right += deg_adjust_right;
	pos_left += deg_adjust_left;

	if (pos_right < MIN)
		pos_right = MIN;
	else if (pos_right > MAX)
		pos_right = MAX;

	if (pos_left < MIN)
		pos_left = MIN;
	else if (pos_left > MAX)
		pos_left = MAX;

	syslog(LOG_DEBUG, "New engine right pos: %i\n", pos_right);
	syslog(LOG_DEBUG, "New engine left pos: %i\n", pos_left);

	servo_set_pos(data, PIN_SERVO_RIGHT, pos_right);
	servo_set_pos(data, PIN_SERVO_LEFT, pos_left);

	return 0;
}

static int init_direction(shared_data_t *data)
{
	set_direction(data, 90, 90); /*  pwm_off = 380 */
	return 0;
}

static void deinit_direction(shared_data_t *data)
{
	set_direction(data, 90, 90); /*  pwm_off = 380 */
}

static int set_direction_arg(int argc, char *argv[], shared_data_t *data)
{
	int pos_right, pos_left;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "Direction RPC: too few arguments *ds <40-140> <40-140>*\n");
		return -1;
	}

	pos_right = strtol(argv[1], &end, 10);
	if (pos_right < MIN || pos_right > MAX || *end != '\0') {
		syslog(LOG_ERR, "Direction RPC: invalid argument #1 %s",
		       argv[1]);
		return -1;
	}

	pos_left = strtol(argv[2], &end, 10);
	if (pos_left < MIN || pos_left > MAX || *end != '\0') {
		syslog(LOG_ERR, "Direction RPC: invalid argument #2 %s",
		       argv[2]);
		return -1;
	}

	return set_direction(data, pos_right, pos_left);
}

static void get_direction(shared_data_t *data, int *pos_right, int *pos_left)
{
	*pos_right = servo_get_pos(data, PIN_SERVO_RIGHT);
	*pos_left = servo_get_pos(data, PIN_SERVO_LEFT);
}

static int set_dir_adjust_arg(int argc, char *argv[], shared_data_t *data)
{
	int cur_pos_right, cur_pos_left;
	int new_adj_right, new_adj_left;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "Direction adjust RPC: too few arguments *da <-100-100> <-100-100>*\n");
		return -1;
	}

	new_adj_right = strtol(argv[1], &end, 10);
	if (new_adj_right < -30 || new_adj_right > 30 || *end != '\0') {
		syslog(LOG_ERR, "Motor adjust RPC: invalid argument #1 %s",
		       argv[1]);
		return -1;
	}

	new_adj_left = strtol(argv[2], &end, 10);
	if (new_adj_left < -30 || new_adj_left > 30 || *end != '\0') {
		syslog(LOG_ERR, "Motor adjust RPC: invalid argument #2 %s",
		       argv[2]);
		return -1;
	}

	get_direction(data, &cur_pos_right, &cur_pos_left);

	cur_pos_right -= deg_adjust_right;
	cur_pos_left -= deg_adjust_left;
	deg_adjust_right = new_adj_right;
	deg_adjust_left = new_adj_left;

	set_direction(data, cur_pos_right, cur_pos_left);

	return 0;
}

static piboat_rpc_t direction_rpc = {
	.cmd_name = "ds",
	.init = init_direction,
	.cmd_set = set_direction_arg,
	.deinit = deinit_direction,
};

static piboat_rpc_t direction_adjust_rpc = {
	.cmd_name = "da",
	.init = NULL,
	.cmd_set = set_dir_adjust_arg,
	.deinit = NULL,
};

static void init_piboat_direction(void) __attribute__((constructor));
void init_piboat_direction(void)
{
	register_piboat_rpc(&direction_rpc);
	register_piboat_rpc(&direction_adjust_rpc);
}
