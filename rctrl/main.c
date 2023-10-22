/*************************************************************************
 *	PiBoat project: this programm is used for control a RC boat with a
 *	Raspberry Pi.
 *
 *	Copyright (C) 2018-2024  TERNISIEN d'OUVILLE Matthieu
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
#include <unistd.h>

#include "connect_tcp.h"

#define STEER_SET_POS_CMD "steer_set_pos"
#define STEER_ADJ_POS_CMD "steer_adj_pos"

static void send_command(socket_t sock, const char *cmd_name, int speed)
{
	char cmd[BUFSIZ];
	int len;

	len = snprintf(cmd, BUFSIZ, "%s %i", cmd_name, speed);
	printf("send command: %s\n", cmd);

	errno = 0;
	if (send(sock, cmd, len + 1, 0) < len + 1) {
		printf("send error: %s\n", strerror(errno));
	}
}

static void set_motors_speed(socket_t sock)
{
	int speed1;
	int redo;

	do {
		redo = 0;

		printf("Enter integer between -1000 and 1000 (0 to stop engine)\n");
		scanf("%i", &speed1);

		if (speed1 < -1000 || speed1 > 1000) {
			printf("Invalid entry\n");
			redo = 1;
		}
	} while (redo);

	send_command(sock, "ms", speed1);
}

static void set_pod_pos(socket_t sock)
{
	int pod_pos;
	int redo;

	do {
		redo = 0;

		printf("Enter one integer between 0 and 100\n");
		scanf("%i", &pod_pos);

		if (pod_pos < 0 || pod_pos > 100) {
			printf("Invalid entry\n");
			redo = 1;
		}
	} while (redo);

	send_command(sock, STEER_SET_POS_CMD, pod_pos + 40);
}

static void run_test(socket_t sock)
{
	int i;

	sleep(2);

	/* speed test */
	for (i = 0; i <= 50; i += 2) {
		printf("Speed %i%%\n", i);
		send_command(sock, "ms", i);
		usleep(500000);
	}

	for (i = 50; i >= 0; i -= 2) {
		printf("Speed %i%%\n", i);
		send_command(sock, "ms", i);
		usleep(500000);
	}

	/* direction test */
	send_command(sock, STEER_SET_POS_CMD, 40);
	sleep(1);
	send_command(sock, STEER_SET_POS_CMD, 90);
	sleep(1);
	send_command(sock, STEER_SET_POS_CMD, 140);
	sleep(1);
}

int main(int argc, char *argv[])
{
	char *hostname;
	socket_t sock;
	int choice;

	if (argc > 1)
		hostname = argv[1];
	else
		hostname = "piboat"; 

	sock = init_socket_client(4000, hostname);
	if (sock < 0) {
		switch (sock) {
		case SOCK_NO_HOST:
			printf("Cannot connect to PiBoat (no host)\n");
			return -1;
		case SOCK_CREATE:
			printf("Cannot connect to PiBoat (socket not created)\n");
			return -1;
		case SOCK_CONNECT:
			printf("Cannot connect to PiBoat (connection)\n");
			return -1;
		case SOCK_BIND:
			printf("Cannot connect to PiBoat (bind)\n");
			return -1;
		default:
			printf("Cannot connect to PiBoat (unknown)\n");
			return -1;
		}
	}

	do {
		printf("Piboat menu\n");
		printf("\n");
		printf("1 - Set engine speed\n");
		printf("2 - Set pod position\n");
		printf("9 - Run test mode\n");
		printf("\n");
		printf("0 - exit\n");
		printf("\n");

		printf("Your choice: ");
		scanf("%i", &choice);

		switch (choice) {
		case 1:
			set_motors_speed(sock);
			break;
		case 2:
			set_pod_pos(sock);
			break;
		case 9:
			run_test(sock);
			break;
		case 0:
			/* nothing to do */
			break;
		}
	} while (choice != 0);

	close_sock(sock);

	return 0;
}
