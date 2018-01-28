/*************************************************************************
 *	Copyright (C) 2014  TERNISIEN d'OUVILLE Matthieu
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

#include "receive_rc.h"

/* Changer la vitesse des moteurs
 * m <vitesse moteur 1> <vitesse moteur 2>
 */
#define CMD_MOTOR		"m"
/* Changer reglage des moteurs
 * mri <pourcentage add/sub>
 */
#define CMD_MOTOR_R1	"mr1"	
#define CMD_MOTOR_R2	"mr2"
/* Changer position gouvernails
 * g <degre entre 40 et 140>
 */
#define CMD_DIRECTION	"g"
/* Changer reglage gouvernails
 * gr <degre add/sub>
 */
#define CMD_DIR_REG		"gr"

static const int CONNECT_PORT =	4000;

/* retourne le nom de la commande */
void value_cmd(char *cmd, char *cmd_val)
{
	int i;

	i = 0;
	while (cmd[i] != ' ' && cmd[i] != '\0'){
		cmd_val[i] = cmd[i];
		
		++i;
	}

	cmd_val[i] = '\0';
}

/* retourne la valeur entier d'un parametre */
int value_param(char *cmd, int pnum)
{
	int i, j, y, cur_pnum;
	char param[10];

	i=0;
	cur_pnum = 0;
	for (y=0 ; y<strlen(cmd) && cur_pnum!=pnum ; ++y){
		while (cmd[i] != ' ')
			++i;

		++i;
		++cur_pnum;
	}

	j = 0;
	while (cmd[i] != ' ' && i < strlen(cmd)){
		param[j] = cmd[i];

		++i;
		++j;
	}

	param[j] = '\0';

	return atoi(param);
}

void* receive_rc_thread(void *p)
{
	socket_t sock, sock_cli;
	struct sockaddr_in csin = {0};
	char cmd[21], cmd_val[5];
	int nb, err;
	shared_data_t *data;

	data = (shared_data_t*)p;

	/* Server initialisation */
	sock = init_socket_serv(CONNECT_PORT, 1);
	switch (sock){
		case SOCK_CREATE:
			printf("MAIN server initialisation\t\t[Failed]\n");
			fprintf(stderr, "MAIN server initiation: socket not create\n");
			kill(getpid(), SIGINT);
			pthread_exit(NULL);
			return NULL;
		case SOCK_BIND:
			printf("MAIN server initialisation\t\t[Failed]\n");
			fprintf(stderr, "MAIN server initiation: bind error\n");
			kill(getpid(), SIGINT);
			pthread_exit(NULL);
			return NULL;
		default: printf("MAIN server initialisation\t\t[OK]\n");
	}

	/* Main loop: only one client can use the boat. */
	while (true) {
		/* MAIN thread */
		/* Wait a connection on port 4000, then recv and execute client's command. */

		/* Client connection */
		nb = sizeof(csin);
		if ((sock_cli = accept(sock, (struct sockaddr*)&csin, (socklen_t *)&nb)) < 0)
			fprintf(stderr, "Accept connection error: %i\n", sock_cli);

		/* fprintf(stdin, "connection accepted\n"); */
		print_debug(stdout, "connection accepted\n");

		/* Commands recv */
		do{
			fflush(stdin);
			nb = recv(sock_cli, cmd, 20, 0);
			/* printf("nb: %i\n", nb); */
			if (nb <= 0){
				perror("recv error");
				break;
			}

			cmd[nb] = '\0';

			/* DEBUG */
			print_debug(stdout, "msg rcv: %s\n", cmd);
			value_cmd(cmd, cmd_val);

			if (strcmp(cmd_val, CMD_MOTOR) == 0){
				/* DEBUG */
				print_debug(stdout, "cmd value: %i %i\n", value_param(cmd, 1), value_param(cmd, 2));
				set_motor_speed(data, value_param(cmd, 1), value_param(cmd, 2));
			}
			else if (strcmp(cmd_val, CMD_DIRECTION) == 0){
				/* DEBUG */
				print_debug(stdout, "cmd value: %i\n", value_param(cmd, 1));
				set_direction(data, value_param(cmd, 1));
			}
			else if (strcmp(cmd_val, CMD_DIR_REG) == 0){
				/* DEBUG */
				print_debug(stdout, "cmd value: %i\n", value_param(cmd, 1));
				set_dir_adjust(data, value_param(cmd, 1));
			}
			else if (strcmp(cmd_val, CMD_MOTOR_R1) == 0){
				/* DEBUG */
				print_debug(stdout, "cmd value: %i\n", value_param(cmd, 1));
				err = set_motor_adjust(data, 1, value_param(cmd, 1));
				if (err < 0) fprintf(stderr, "Error: set_motor_adjust param incorrect.\n");
			}
			else if (strcmp(cmd_val, CMD_MOTOR_R2) == 0){
				/* DEBUG */
				print_debug(stdout, "cmd value: %i\n", value_param(cmd, 1));
				err = set_motor_adjust(data, 2, value_param(cmd, 1));
				if (err < 0) fprintf(stderr, "Error: set_motor_adjust param incorrect.\n");
			}
			else if (strcmp(cmd_val, "exit") != 0){
				fprintf(stderr, "cmd unknown: %s\n", cmd);
			}
		} while (strcmp(cmd, "exit") != 0);

		close_sock(sock_cli);
		/* Ended system */
		deinit_motor(data);
		deinit_direction(data);
	}

	/* close_sock(sock); */
	pthread_exit(NULL);
	return NULL;
}
