/*************************************************************************
 *	PiBoat project: this programm is used for control a RC boat with a
 *	Raspberry Pi.
 *
 *	Copyright (C) 2018  TERNISIEN d'OUVILLE Matthieu
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
 *	See also piboat.blogspot.fr
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "connect_tcp.h"

static void set_motors_speed(socket_t sock)
{
	int speed1, speed2;
	char cmd[BUFSIZ];
	int redo;
	int len;

	do {
		redo = 0;

		printf("Enter 2 integers between -100 and 100 (0 for stop motors)\n");
		scanf("%i %i", &speed1, &speed2);

		if (speed1 < -100 || speed1 > 100
		    || speed2 < -100 || speed2 > 100) {
			printf("Invalid entry\n");
			redo = 1;
		}
	} while (redo);

	len = snprintf(cmd, BUFSIZ, "ms %i %i", speed1, speed2);
	printf("send command: %s\n", cmd);

	errno = 0;
	if (send(sock, cmd, len + 1, 0) < len + 1) {
		printf("send error: %s\n", strerror(errno));
	}
}

static void set_rudders_pos(socket_t sock)
{
	char cmd[BUFSIZ];
	int rudder_pos;
	int redo;
	int len;

	do {
		redo = 0;

		printf("Enter one integer between 0 and 100\n");
		scanf("%i", &rudder_pos);

		if (rudder_pos < 0 || rudder_pos > 100) {
			printf("Invalid entry\n");
			redo = 1;
		}
	} while (redo);

	len = snprintf(cmd, BUFSIZ, "ds %i", rudder_pos + 40);
	printf("send command: %s\n", cmd);

	errno = 0;
	if (send(sock, cmd, len + 1,  0) < len + 1) {
		printf("send error: %s\n", strerror(errno));
	}
}

int main(int argc, char *argv[])
{
	socket_t sock;
	int choice;

	sock = init_socket_client(4000);
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
		printf("1 - Set motors speed\n");
		printf("2 - Set rudders position\n");
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
			set_rudders_pos(sock);
			break;
		case 0:
			/* nothing to do */
			break;
		}
	} while (choice != 0);

	close_sock(sock);

	return 0;
}
