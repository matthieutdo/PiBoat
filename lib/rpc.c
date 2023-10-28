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
#include <time.h>

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

int enqueue_rpc_cmd(char *cmd_line, enum rpc_prio_e prio, shared_data_t *data)
{
	struct rpc_cmd_entry *cur_cmd_e, *new_cmd_e;
	struct rpc_entry *rpc_e;
	char *cmd_argv[PIBOAT_CMD_MAXARG + 1];
	int cmd_argc;
	int i;

	strtoarg(cmd_line, &cmd_argc, cmd_argv);

	LIST_FOREACH(rpc_e, &rpc_list, next) {
		if (strcmp(rpc_e->rpc->cmd_name, cmd_argv[0]) == 0)
			break;
	}


	if (rpc_e == NULL) {
		syslog(LOG_ERR, "No RPC found for command %s", cmd_argv[0]);
		return -1;
	}

	syslog(LOG_DEBUG, "%s: found RPC %s", __func__, rpc_e->rpc->cmd_name);

	new_cmd_e = malloc(sizeof(struct rpc_cmd_entry));
	if (new_cmd_e == NULL) {
		syslog(LOG_ERR, "Failed to allocate memory for rpc %s",
		       cmd_argv[0]);
		return -1;
	}

	new_cmd_e->cmd.argc = cmd_argc;
	for (i = 0; i < cmd_argc; i++)
		strncpy(new_cmd_e->cmd.argv[i], cmd_argv[i],
			PIBOAT_CMD_MAXLEN);
	new_cmd_e->cmd.prio = prio;

	syslog(LOG_DEBUG, "%s: lock %p", __func__, rpc_e->rpc->queue_mutex);
	pthread_mutex_lock(rpc_e->rpc->queue_mutex);
	syslog(LOG_DEBUG, "%s: %p locked", __func__, rpc_e->rpc->queue_mutex);

	TAILQ_FOREACH(cur_cmd_e, rpc_e->rpc->cmd_list, entries) {
		if (cur_cmd_e->cmd.prio < new_cmd_e->cmd.prio)
			break;
	}

	if (cur_cmd_e == NULL)
		TAILQ_INSERT_TAIL(rpc_e->rpc->cmd_list, new_cmd_e, entries);
	else
		TAILQ_INSERT_BEFORE(cur_cmd_e, new_cmd_e, entries);

	syslog(LOG_DEBUG, "%s: unlock %p", __func__, rpc_e->rpc->queue_mutex);
	pthread_mutex_unlock(rpc_e->rpc->queue_mutex);
	syslog(LOG_DEBUG, "%s: signal cond %p", __func__, rpc_e->rpc->wait_cond);
	pthread_cond_signal(rpc_e->rpc->wait_cond);

	return 0;
}

struct rpc_cmd_entry* dequeue_rpc_cmd(struct rpc_cmd_list *cmd_list, pthread_mutex_t *queue_mutex,
				      pthread_mutex_t *wait_mutex, pthread_cond_t *wait_cond)
{
	struct rpc_cmd_entry *cmd_e = NULL;
	struct timespec ts;

	/* Workaround:
	 * The receiver thread can execute enqueue_rpc_cmd between the
	 * TAILQ_EMPTY check and the pthread_cond_timedwait() call.
	 * If this happens, this thread will be locked until the next cmd was
	 * received. Let's add a timeout on the condition to relax the mutex
	 * and re-check the list state every second will workaround this case.
	 */
	clock_gettime(CLOCK_REALTIME, &ts);

	while (TAILQ_EMPTY(cmd_list)) {
		ts.tv_sec += 1;
		syslog(LOG_DEBUG, "%s: lock %p", __func__, wait_mutex);
		pthread_mutex_lock(wait_mutex);
		syslog(LOG_DEBUG, "%s: wait cond %p", __func__, wait_cond);
		pthread_cond_timedwait(wait_cond, wait_mutex, &ts);
		pthread_mutex_unlock(wait_mutex);
		syslog(LOG_DEBUG, "%s: cond %p relaxed", __func__, wait_cond);
	}

	syslog(LOG_DEBUG, "%s: lock %p", __func__, queue_mutex);
	pthread_mutex_lock(queue_mutex);
	syslog(LOG_DEBUG, "%s: locked %p", __func__, queue_mutex);

	cmd_e = TAILQ_FIRST(cmd_list);
	TAILQ_REMOVE(cmd_list, cmd_e, entries);

	syslog(LOG_DEBUG, "%s: unlock %p", __func__, queue_mutex);
	pthread_mutex_unlock(queue_mutex);

	return cmd_e;
}
