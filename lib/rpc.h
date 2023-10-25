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

#ifndef _rpc_h
#define _rpc_h

#include <sys/queue.h>

#include "shared_data.h"

#define PIBOAT_CMD_MAXARG    32
#define PIBOAT_CMD_MAXLEN    128

enum rpc_prio_e {
	RPC_PRIO_USER = 0,
	RPC_PRIO_SYS = 99,
};

typedef struct {
	int argc;
	char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN];
	enum rpc_prio_e prio;
} rpc_cmd_t;

struct rpc_cmd_entry {
	TAILQ_ENTRY(rpc_cmd_entry) entries;
	rpc_cmd_t cmd;
};
TAILQ_HEAD(rpc_cmd_list, rpc_cmd_entry);

typedef struct {
	char *cmd_name;

	struct rpc_cmd_list *cmd_list;
	pthread_cond_t *wait_cond;
	pthread_mutex_t *queue_mutex;
} rpc_t;

int register_rpc(rpc_t *rpc);
int enqueue_rpc_cmd(char *cmd_line, enum rpc_prio_e, shared_data_t *data);
struct rpc_cmd_entry* dequeue_rpc_cmd(struct rpc_cmd_list *cmd_list, pthread_mutex_t *queue_mutex,
				      pthread_mutex_t *wait_mutex, pthread_cond_t *wait_cond);
#endif
