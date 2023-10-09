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
	char cmd[BUFSIZ];
	int speed1;
	int redo;
	int len;

	do {
		redo = 0;

		printf("Enter integer between -100 and 100 (0 to stop engine)\n");
		scanf("%i", &speed1);

		if (speed1 < -100 || speed1 > 100) {
			printf("Invalid entry\n");
			redo = 1;
		}
	} while (redo);

	len = snprintf(cmd, BUFSIZ, "ms %i", speed1);
	printf("send command: %s\n", cmd);

	errno = 0;
	if (send(sock, cmd, len + 1, 0) < len + 1) {
		printf("send error: %s\n", strerror(errno));
	}
}

static void set_pod_pos(socket_t sock)
{
	int pod_pos_right, pod_pos_left;
	char cmd[BUFSIZ];
	int redo;
	int len;

	do {
		redo = 0;

		printf("Enter one integer between 0 and 100\n");
		scanf("%i", &pod_pos);

		if (pod_pos < 0 || pod_pos > 100) {
			printf("Invalid entry\n");
			redo = 1;
		}
	} while (redo);

	len = snprintf(cmd, BUFSIZ, "ds %i %i", pod_pos_right + 40,
		       pod_pos_left + 40);
	printf("send command: %s\n", cmd);

	errno = 0;
	if (send(sock, cmd, len + 1,  0) < len + 1) {
		printf("send error: %s\n", strerror(errno));
	}
}

static void set_cam_pos(socket_t sock)
{
	char cmd[BUFSIZ];
	int cam_pos_x;
	int cam_pos_y;
	int redo;
	int len;

	do {
		redo = 0;

		printf("Enter two integers (X axis and Y axis)\n");
		scanf("%i %i", &cam_pos_x, &cam_pos_y);

		if (cam_pos_x < 0 || cam_pos_x > 170) {
			printf("Invalid X entry\n");
			redo = 1;
		}

		if (cam_pos_y < 60 || cam_pos_y > 170) {
			printf("Invalid Y entry\n");
			redo = 1;
		}
	} while (redo);

	len = snprintf(cmd, BUFSIZ, "c %i %i", cam_pos_x, cam_pos_y);
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
		printf("1 - Set engine speed\n");
		printf("2 - Set pods position\n");
		printf("3 - Set cam position\n");
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
		case 3:
			set_cam_pos(sock);
			break;
		case 0:
			/* nothing to do */
			break;
		}
	} while (choice != 0);

	close_sock(sock);

	return 0;
}
