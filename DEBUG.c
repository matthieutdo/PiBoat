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
 *	The #define DEBUG_MODE directive must be actif for use print_debug.
 ************************************************************************/

#include "DEBUG.h"

/**************************************************************
 *	Translate int to ASCII
 *
 *	@param val			The value to translate.
 *	@param res			The result of the translate.
 *
 *	@return void
 **************************************************************/
#ifdef DEBUG_MODE
static void itoa(int val, char* res)
{
	int buff, i=0, j=0;
	char c;

	while (val>0){
		buff = val%10;
		val /= 10;

		res[i] = buff+0x30;
		i++;
	}

	res[i] = '\0';
	i--;

	for(j=0 ; j<i ; j++){
		c = res[j];
		res[j] = res[i];
		res[i] = c;
		i--;
	}
}
#endif

void print_debug(struct _IO_FILE* stdio, const char *msg, ...)
{
#ifdef DEBUG_MODE
	va_list pile;
	int i=0, j=0, n;
	char v[50];

	/* list initialisation */
	va_start(pile, msg);

	/* travel all char of the string */
	while(*(msg+i) != '\0'){
		switch(*(msg+i)){
		case '%': /* if '%' find print value of the var */
			i++;
			switch(*(msg+i)){
			case 'c' : /*  Print a char */
				fputc(va_arg(pile, int), stdio);
				break;
			case 'd' : /*  Print an integer */
			case 'i' : /*  Print an integer */
				n = va_arg(pile, int);
				itoa(n, v);
				for(j=0 ; j<strlen(v) ; j++) fputc(v[j], stdio);
				break;
			case 's' : /*  Print the string */
				strcpy(v, va_arg(pile, char *));

				for(j=0 ; j<strlen(v) ; j++) fputc(v[j], stdio);
				break;
			}
			break;
		default :fputc(*(msg+i), stdio);
		}
		i++;
	}

	va_end(pile);
#endif
}
