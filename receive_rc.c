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

#include "connect_tcp.h"
#include "thread_manager.h"
#include "rpc.h"

static const int CONNECT_PORT = 4000;

static void* main_loop(void *p)
{
	char cmd[PIBOAT_CMD_MAXLEN + 1];
	socket_t sock, sock_cli;
	struct sockaddr_in csin = { 0 };
	shared_data_t *data;
	int nb;

	data = (shared_data_t*)p;

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

	if (init_rpc(data) < 0) {
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

			/* leave loop if command exit is received. */
			if (strcmp(cmd, "exit") == 0)
				break;

			syslog(LOG_DEBUG, "Recv command: %s\n", cmd);

			err = exec_rpc(cmd, data);
			if (err)
				syslog(LOG_ERR, "Error will applying %s command\n",
				       cmd);
		} while (true);

		close_sock(sock_cli);
	}

	deinit_rpc(data);

	/* close_sock(sock); */
	pthread_exit(NULL);
	return NULL;
}

static module_t main_module = {
	.name = "main",
	.loop = main_loop,
};

static void init_main_module(void) __attribute__((constructor));
void init_main_module(void)
{
	register_module(&main_module);
}
