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
#include <sys/queue.h>
#include <string.h>

#include "rpc.h"

struct rpc_entry {
	LIST_ENTRY(rpc_entry) next;
	rpc_t *rpc;
};

LIST_HEAD(rpc_list, rpc_entry);
struct rpc_list rpc_list = LIST_HEAD_INITIALIZER(rpc_list);

static void strtoarg(char *cmd, int *argc, char *argv[])
{
	char *buff;

	*argc = 0;

	for (buff = strtok(cmd, " ");
	     buff != NULL && *argc < PIBOAT_CMD_MAXARG;
	     buff = strtok(NULL, " "), (*argc)++) {
		argv[*argc] = buff;
	}

	argv[*argc] = NULL;

	if (buff != NULL)
		syslog(LOG_ERR, "RPC command %s has too many arguments!",
		       argv[0]);
}

int register_rpc(rpc_t *rpc)
{
	struct rpc_entry *e;

	if (rpc->cmd_set == NULL) {
		printf("RPC command function cannot be NULL!\n");
		return -1;
	}

	LIST_FOREACH(e, &rpc_list, next) {
		if (strcmp(e->rpc->cmd_name, rpc->cmd_name) == 0) {
			printf("RPC command '%s' already exists\n",
			       rpc->cmd_name);
			return -1;
		}
	}

	e = malloc(sizeof(struct rpc_entry));
	if (e == NULL) {
		printf("Failed to allocate new PiBoat RPC entry\n");
		return -1;
	}

	e->rpc = rpc;

	LIST_INSERT_HEAD(&rpc_list, e, next);

	return 0;
}

int init_rpc(shared_data_t *data)
{
	struct rpc_entry *rpc_e;

	LIST_FOREACH(rpc_e, &rpc_list, next) {
		if (rpc_e->rpc->init == NULL) {
			rpc_e->rpc->initialized = 1;
			continue;
		}

		syslog(LOG_DEBUG, "Initialize '%s' RPC...\n",
		       rpc_e->rpc->cmd_name);

		if (rpc_e->rpc->init(data))
			goto fail;

		rpc_e->rpc->initialized = 1;
	}

	return 0;
fail:
	deinit_rpc(data);
	return -1;
}

int exec_rpc(char *cmd_line, shared_data_t *data)
{
	char *cmd_argv[PIBOAT_CMD_MAXARG + 1];
	int cmd_argc;
	struct rpc_entry *rpc_e;

	strtoarg(cmd_line, &cmd_argc, cmd_argv);

	LIST_FOREACH(rpc_e, &rpc_list, next) {
		if (strcmp(rpc_e->rpc->cmd_name, cmd_argv[0]))
			continue;

		return rpc_e->rpc->cmd_set(cmd_argc, cmd_argv,
					   data);
	}

	syslog(LOG_ERR, "No RPC found for command %s", cmd_argv[0]);
	return -1;
}

void deinit_rpc(shared_data_t *data)
{
	struct rpc_entry *rpc_e;

	LIST_FOREACH(rpc_e, &rpc_list, next) {
		if (rpc_e->rpc->deinit == NULL || rpc_e->rpc->initialized == 0)
			continue;

		rpc_e->rpc->deinit(data);
	}
}
