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

#include <syslog.h>
#include <sys/queue.h>
#include <string.h>
#include <errno.h>

#include "receive_rc.h"

#define PIBOAT_CMD_MAXARG    32
#define PIBOAT_CMD_MAXLEN    128

struct piboat_rpc_entry {
	LIST_ENTRY(piboat_rpc_entry) next;
	piboat_rpc_t *rpc;
};

LIST_HEAD(piboat_rpc_list, piboat_rpc_entry);
struct piboat_rpc_list piboat_rpc_list = LIST_HEAD_INITIALIZER(piboat_rpc_list);

int register_piboat_rpc(piboat_rpc_t *rpc)
{
	struct piboat_rpc_entry *e;

	if (rpc->cmd_set == NULL) {
		printf("RPC command function cannot be NULL!\n");
		return -1;
	}

	LIST_FOREACH(e, &piboat_rpc_list, next) {
		if (strcmp(e->rpc->cmd_name, rpc->cmd_name) == 0) {
			printf("RPC command '%s' already exists\n",
			       rpc->cmd_name);
			return -1;
		}
	}

	e = malloc(sizeof(struct piboat_rpc_entry));
	if (e == NULL) {
		printf("Failed to allocate new PiBoat RPC entry\n");
		return -1;
	}

	e->rpc = rpc;

	LIST_INSERT_HEAD(&piboat_rpc_list, e, next);

	return 0;
}

static void deinit_piboat_rpc(shared_data_t *data)
{
	struct piboat_rpc_entry *rpc_e;

	LIST_FOREACH(rpc_e, &piboat_rpc_list, next) {
		if (rpc_e->rpc->deinit == NULL || rpc_e->rpc->initialized == 0)
			continue;

		rpc_e->rpc->deinit(data);
	}
}

static int init_piboat_rpc(shared_data_t *data)
{
	struct piboat_rpc_entry *rpc_e;

	LIST_FOREACH(rpc_e, &piboat_rpc_list, next) {
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
	deinit_piboat_rpc(data);
	return -1;
}

static const int CONNECT_PORT = 4000;

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

void* receive_rc_thread(void *p)
{
	char cmd[PIBOAT_CMD_MAXLEN + 1];
	char *cmd_argv[PIBOAT_CMD_MAXARG + 1];
	int cmd_argc;
	socket_t sock, sock_cli;
	struct sockaddr_in csin = { 0 };
	shared_data_t *data;
	int nb;

	data = (shared_data_t*)p;

	/* Server initialisation */
	sock = init_socket_serv(CONNECT_PORT, 1);
	switch (sock){
		case SOCK_CREATE:
			syslog(LOG_EMERG, "MAIN server initialisation      [FAILED]\n");
			syslog(LOG_EMERG, "MAIN server initiation: socket not create\n");
			pthread_exit(NULL);
			return NULL;
		case SOCK_BIND:
			syslog(LOG_EMERG, "MAIN server initialisation      [FAILED]\n");
			syslog(LOG_EMERG, "MAIN server initiation: bind error\n");
			pthread_exit(NULL);
			return NULL;
		default: syslog(LOG_INFO, "MAIN server initialisation      [  OK  ]\n");
	}

	if (init_piboat_rpc(data) < 0) {
		syslog(LOG_EMERG, "RPC initialization              [FAILED]\n");
		return NULL;
	}

	syslog(LOG_INFO, "RPC initialization              [  OK  ]\n");

	/* Main loop: only one client can use the boat. */
	while (true) {
		/* MAIN thread */
		/* Wait a connection on port 4000, then recv and execute client's command. */

		/* Client connection */
		nb = sizeof(csin);
		if ((sock_cli = accept(sock, (struct sockaddr*)&csin, (socklen_t *)&nb)) < 0)
			syslog(LOG_ERR, "Accept connection error: %i\n", sock_cli);

		syslog(LOG_DEBUG, "connection accepted\n");

		/* Commands recv */
		do {
			struct piboat_rpc_entry *rpc_e;
			int err;

			errno = 0;
			nb = recv(sock_cli, cmd, PIBOAT_CMD_MAXLEN, 0);
			if (nb < 0) {
				syslog(LOG_ERR, "Recv error: %s",
				       strerror(errno));
				break;
			}

			/* TODO check errno! */

			/* Connection lost! */
			if (nb == 0) {
				syslog(LOG_ERR, "Connection terminated.");
				break;
			}

			cmd[nb] = '\0';

			syslog(LOG_DEBUG, "Recv command: %s\n", cmd);

			strtoarg(cmd, &cmd_argc, cmd_argv);

			LIST_FOREACH(rpc_e, &piboat_rpc_list, next) {
				if (strcmp(rpc_e->rpc->cmd_name, cmd_argv[0]))
					continue;

				err = rpc_e->rpc->cmd_set(cmd_argc, cmd_argv,
							  data);
				break;
			}

			if (err)
				syslog(LOG_ERR, "Error will applying %s command\n",
				       cmd_argv[0]);

			if (rpc_e == NULL && strcmp(rpc_e->rpc->cmd_name, "exit"))
				syslog(LOG_ERR, "No RPC found for command %s",
				       cmd_argv[0]);
		} while (strcmp(cmd, "exit") != 0);

		close_sock(sock_cli);
	}

	deinit_piboat_rpc(data);

	/* close_sock(sock); */
	pthread_exit(NULL);
	return NULL;
}
