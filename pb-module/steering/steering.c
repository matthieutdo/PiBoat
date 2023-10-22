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

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "pwm.h"
#include "shared_data.h"
#include "rpc.h"
#include "servo.h"

static const int PIN_SERVO = 0;

static const int MIN = 40;
static const int MAX = 140;

static int deg_adjust = 0;

static int set_steer(shared_data_t *data, int pos)
{
	pos += deg_adjust;

	if (pos < MIN)
		pos = MIN;
	else if (pos > MAX)
		pos = MAX;

	syslog(LOG_DEBUG, "new_pos: %i\n", pos);

	servo_set_pos(data, PIN_SERVO, pos);

	return 0;
}

static int init_steer(shared_data_t *data)
{
	set_steer(data, 90); /*  pwm_off = 380 */
	return 0;
}

static void deinit_steer(shared_data_t *data)
{
	set_steer(data, 90); /*  pwm_off = 380 */
}

static int set_steer_arg(int argc, char *argv[], shared_data_t *data)
{
	int pos;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Steer RPC: too few arguments *ds <40-140>*\n");
		return -1;
	}

	pos = strtol(argv[1], &end, 10);
	if (pos < MIN || pos > MAX || *end != '\0') {
		syslog(LOG_ERR, "Steer RPC: invalid argument %s",
		       argv[1]);
		return -1;
	}

	return set_steer(data, pos);
}

static void get_steer(shared_data_t *data, int *pos)
{
	*pos = servo_get_pos(data, PIN_SERVO);
	*pos -= deg_adjust;
}

static int set_steer_adjust_arg(int argc, char *argv[], shared_data_t *data)
{
	int cur_pos;
	int new_adj;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "Steer adjust RPC: too few arguments *da <-100-100>*\n");
		return -1;
	}

	new_adj = strtol(argv[2], &end, 10);
	if (new_adj < -30 || new_adj > 30 || *end != '\0') {
		syslog(LOG_ERR, "Steer adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	get_steer(data, &cur_pos);
	deg_adjust = new_adj;
	set_steer(data, cur_pos);

	return 0;
}

static rpc_t steer_rpc = {
	.cmd_name = "ds",
	.init = init_steer,
	.cmd_set = set_steer_arg,
	.deinit = deinit_steer,
};

static rpc_t steer_adjust_rpc = {
	.cmd_name = "da",
	.init = NULL,
	.cmd_set = set_steer_adjust_arg,
	.deinit = NULL,
};

static void init_steer_rpc(void) __attribute__((constructor));
void init_steer_rpc(void)
{
	register_rpc(&steer_rpc);
	register_rpc(&steer_adjust_rpc);
}
