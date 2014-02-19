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

#include "connect_tcp.h"


//static const int CONNECT_PORT =	4000;
static const char *RPI_ADDR = "192.168.1.1";


socket_t init_socket_serv(int port, int max_wait){
	socket_t sock;
	struct sockaddr_in serv_addr;
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return SOCK_CREATE;
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	
	if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		return SOCK_BIND;
	
	listen(sock, max_wait);
	
	return sock;
}


socket_t init_socket_client(int port){
	struct hostent *server;
	int res;
	socket_t sock;
	struct sockaddr_in serv_addr;
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (socket_t)SOCK_CREATE;
	
	if ((server = gethostbyname(RPI_ADDR)) == NULL)
		return (socket_t)SOCK_NO_HOST;
	
	
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr, server->h_addr_list[0], server->h_length);
	serv_addr.sin_port = htons(port);
	
	
	if ((res=connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0){
		return (socket_t)SOCK_CONNECT;
	}

	return sock;
}


void close_sock(socket_t sock){
	close((int)sock);
}


