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
 *
 *
 *	receive_rc.h
 *	receive and execute command from the remote control app.
 ************************************************************************/

#ifndef _receive_rc_h
#define _receive_rc_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>		/*  getpid() */
#include <unistd.h>		/*  getpid() */
#include <signal.h>		/*  kill(SIGINT) */

#include "connect_tcp.h"
#include "pwm.h"
#include "motor.h"
#include "direction.h"

#include "shared_data.h"

/**************************************************************
 *	Used for extract the command name.
 *
 *	@param cmd			The command with its parameters.
 *	@param cmd_val		Memory to load the command name.
 *
 *	@return void
 **************************************************************/
void value_cmd(char *cmd, char *cmd_val);

/**************************************************************
 *	Used for extract the value of a parameter type int.
 *
 *	@param cmd		The command with its parameters.
 *	@param pnum		The parameter number to extract.
 *
 *	@return int		The parameter value.
 **************************************************************/
int value_param(char *cmd, int pnum);

/**************************************************************
 *	MAIN thread.
 *	Receive and execute command from the remote control app.
 *
 *	@param sock_cli		Client socket
 *	@param data			Shared data between all thread
 *
 *	@return socket_t	Socket communication 
 *				avec le client
 **************************************************************/
void* receive_rc_thread(void *p);
#endif
