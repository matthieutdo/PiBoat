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

#include "shared_data.h"

#define PIBOAT_CMD_MAXARG    32
#define PIBOAT_CMD_MAXLEN    128

typedef struct {
	char *cmd_name;
	int (*init)(shared_data_t*);
	int (*cmd_set)(int, char**, shared_data_t*);
	void (*deinit)(shared_data_t*);

	/* private */
	int initialized;
} rpc_t;

int register_rpc(rpc_t *rpc);
int init_rpc(shared_data_t *data);
int exec_rpc(char *cmd_line, shared_data_t *data);
void deinit_rpc(shared_data_t *data);
#endif
