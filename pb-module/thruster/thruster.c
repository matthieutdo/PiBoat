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
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include "shared_data.h"
#include "rpc.h"
#include "thread_manager.h"
#include "thruster.h"

static pthread_mutex_t rpc_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rpc_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t rpc_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct rpc_cmd_list rpc_cmd_list = TAILQ_HEAD_INITIALIZER(rpc_cmd_list);

static thruster_t thruster = {22, 24, 8, 0, 0};

/* TODO test système de reglage... */

#define THRUSTER_ADJ_SPEED_CMD "thruster_adj_speed"
static int set_thruster_adjust_arg(int argc,
			char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN],
			shared_data_t *data)
{
	long int adjust;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Thruster adjust RPC: too few arguments *ma <-100-100>*\n");
		return -1;
	}

	adjust = strtol(argv[2], &end, 10);
	if (adjust < -30 || adjust > 30 || *end != '\0') {
		syslog(LOG_ERR, "Thruster adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	thruster.adjust = adjust;

	return 0;
}

#define THRUSTER_SET_SPEED_CMD "thruster_set_speed"
static int set_thruster_speed_arg(int argc,
			char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN],
			shared_data_t *data)
{
	long int speed;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Thruster speed RPC: too few arguments *ms <-1000-1000>*\n");
		return -1;
	}

	speed = strtol(argv[1], &end, 10);
	if (speed < -1000 || speed > 1000 || *end != '\0') {
		syslog(LOG_ERR, "Thruster speed RPC: invalid argument 1 %s",
		       argv[1]);
		return -1;
	}

	return set_thruster_speed(&thruster, data, (int)speed);
}

static rpc_t thruster_speed_rpc = {
	.cmd_name = THRUSTER_SET_SPEED_CMD,
	.cmd_list = &rpc_cmd_list,
	.wait_cond = &rpc_wait_cond,
	.queue_mutex = &rpc_queue_mutex,
};

static rpc_t thruster_adjust_rpc = {
	.cmd_name = THRUSTER_ADJ_SPEED_CMD,
	.cmd_list = &rpc_cmd_list,
	.wait_cond = &rpc_wait_cond,
	.queue_mutex = &rpc_queue_mutex,
};

static void init_thruster_rpc(void) __attribute__((constructor));
void init_thruster_rpc(void)
{
	register_rpc(&thruster_speed_rpc);
	register_rpc(&thruster_adjust_rpc);
}

static void* thruster_loop(void *p)
{
	struct rpc_cmd_entry *rpc_cmd_e;
	shared_data_t *data;
	int ret;

	data = (shared_data_t *)p;

	init_thruster(&thruster, data);

	while (true) {
		rpc_cmd_e = dequeue_rpc_cmd(&rpc_cmd_list, &rpc_queue_mutex,
					    &rpc_wait_mutex, &rpc_wait_cond);
		if (rpc_cmd_e == NULL)
			continue;

		if (strcmp(rpc_cmd_e->cmd.argv[0],
			   THRUSTER_SET_SPEED_CMD) == 0) {
			ret = set_thruster_speed_arg(rpc_cmd_e->cmd.argc,
						     rpc_cmd_e->cmd.argv,
						     data);
		} else if (strcmp(rpc_cmd_e->cmd.argv[0],
				THRUSTER_ADJ_SPEED_CMD) == 0) {
			ret = set_thruster_adjust_arg(rpc_cmd_e->cmd.argc,
						      rpc_cmd_e->cmd.argv,
						      data);
		}

		if (ret != 0)
			syslog(LOG_ERR, "Error occured during %s rpc exec",
			       rpc_cmd_e->cmd.argv[0]);

		free(rpc_cmd_e);
	}

	deinit_thruster(&thruster, data);

	return NULL;
}

static module_t thruster_module = {
	.name = "thruster",
	.loop = thruster_loop,
};

static void init_thruster_module(void) __attribute__((constructor));
void init_thruster_module(void)
{
	register_module(&thruster_module);
}
