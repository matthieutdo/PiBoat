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

#ifndef _DEBUG_h
#define _DEBUG_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/**************************************************************
 *	Print the message msg if DEBUG mode is actif.
 *
 *	@param stdio		Standard io to write the message.
 *	@param msg			Message to write.
 *	@param ...			All values to write with the message.
 *
 *	@return void
 **************************************************************/
void print_debug(struct _IO_FILE* stdio, const char *msg, ...);

#endif
