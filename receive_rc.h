/*************************************************************************
 *	Copyright (C) 2014-2018  TERNISIEN d'OUVILLE Matthieu
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
 *	receive and execute command from the remote control app.
 ************************************************************************/

#ifndef _receive_rc_h
#define _receive_rc_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "connect_tcp.h"
#include "pwm.h"
#include "motor.h"
#include "direction.h"

#include "shared_data.h"

typedef struct {
	char *cmd_name;
	int (*init)(shared_data_t*);
	int (*cmd_set)(int, char**, shared_data_t*);
	void (*deinit)(shared_data_t*);

	/* private */
	int initialized;
} piboat_rpc_t;

int register_piboat_rpc(piboat_rpc_t *rpc);

/**************************************************************
 *	MAIN thread.
 *	Receive and execute command from the remote control app.
 *
 *	@param sock_cli		Client socket
 *	@param data		Shared data between all thread
 *
 *	@return NULL
 **************************************************************/
void* receive_rc_thread(void *p);
#endif
