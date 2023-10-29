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
#include <string.h>
#include <limits.h>

#include "pwm.h"
#include "shared_data.h"
#include "rpc.h"
#include "servo.h"
#include "thread_manager.h"

static pthread_mutex_t rpc_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rpc_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t rpc_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct rpc_cmd_list rpc_cmd_list = TAILQ_HEAD_INITIALIZER(rpc_cmd_list);

servo_t steering = { 0 };

static const int MIN = 0;
static const int MAX = 180;

static int deg_adjust = 0;

static void set_steer_pos(int pos)
{
	pos += deg_adjust;

	if (pos < MIN)
		pos = MIN;
	else if (pos > MAX)
		pos = MAX;

	syslog(LOG_DEBUG, "new_pos: %i\n", pos);

	set_servo_pos(&steering, &rpc_cmd_list, pos);
}

#define STEER_SET_POS_CMD "steer_set_pos"
static int steer_pos_parse_arg(int argc, char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN])
{
	int pos;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Steer RPC: too few arguments *ds <0-180>*\n");
		return INT_MAX;
	}

	pos = strtol(argv[1], &end, 10);
	if (pos < MIN || pos > MAX || *end != '\0') {
		syslog(LOG_ERR, "Steer RPC: invalid argument %s",
		       argv[1]);
		return INT_MAX;
	}

	return pos;
}

static void get_steer(int *pos)
{
	*pos = get_servo_pos(&steering);
	*pos -= deg_adjust;
}

#define STEER_ADJ_POS_CMD "steer_adj_pos"
static int set_steer_adjust_arg(int argc,
			char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN])
{
	int cur_pos;
	int new_adj;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "Steer adjust RPC: too few arguments *da <-30-30>*\n");
		return -1;
	}

	new_adj = strtol(argv[2], &end, 10);
	if (new_adj < -30 || new_adj > 30 || *end != '\0') {
		syslog(LOG_ERR, "Steer adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	get_steer(&cur_pos);
	deg_adjust = new_adj;
	set_steer_pos(cur_pos);

	return 0;
}

static rpc_t steer_rpc = {
	.cmd_name = STEER_SET_POS_CMD,
	.cmd_list = &rpc_cmd_list,
	.wait_cond = &rpc_wait_cond,
	.queue_mutex = &rpc_queue_mutex,
};

static rpc_t steer_adjust_rpc = {
	.cmd_name = STEER_ADJ_POS_CMD,
	.cmd_list = &rpc_cmd_list,
	.wait_cond = &rpc_wait_cond,
	.queue_mutex = &rpc_queue_mutex,
};

static void init_steer_rpc(void) __attribute__((constructor));
void init_steer_rpc(void)
{
	register_rpc(&steer_rpc);
	register_rpc(&steer_adjust_rpc);
}

static void* steering_loop(void *p)
{
	struct rpc_cmd_entry *rpc_cmd_e;
	int ret;

	steering.data = (shared_data_t *)p;
	init_servo(&steering);

	pthread_cleanup_push(deinit_servo, (void *)&steering);

	while (true) {
		// XXX wait new request
		rpc_cmd_e = dequeue_rpc_cmd(&rpc_cmd_list, &rpc_queue_mutex,
					    &rpc_wait_mutex, &rpc_wait_cond);
		if (rpc_cmd_e == NULL)
			continue;

		if (strcmp(rpc_cmd_e->cmd.argv[0], STEER_SET_POS_CMD) == 0) {
			int pos = steer_pos_parse_arg(rpc_cmd_e->cmd.argc, rpc_cmd_e->cmd.argv);

			if (pos == INT_MIN || pos == get_servo_pos(&steering))
				continue;

			set_steer_pos(pos);
		} else if (strcmp(rpc_cmd_e->cmd.argv[0],
				  STEER_ADJ_POS_CMD) == 0) {
			ret = set_steer_adjust_arg(rpc_cmd_e->cmd.argc, rpc_cmd_e->cmd.argv);
		}

		if (ret != 0)
			syslog(LOG_ERR, "Error occured during %s rpc exec",
			       rpc_cmd_e->cmd.argv[0]);

		free(rpc_cmd_e);
	}

	pthread_cleanup_pop(1);

	return NULL;
}

static module_t steering_module = {
	.name = "steering",
	.loop = steering_loop,
};

static void init_steering_module(void) __attribute__((constructor));
void init_steering_module(void)
{
	register_module(&steering_module);
}
