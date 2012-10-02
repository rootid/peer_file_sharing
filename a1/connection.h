/*
 * connection.h
 *
 *  Created on: Feb 4, 2012
 *      Author: vikram
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "boot.h"


typedef struct
{
	char ip_addr[40];
	char host_name[40];
	int port_no;
	int sock_fd;
	enum {SOURCE =1,DESTINATION = 1,SELF=3} sock_type; //source :create connection with destination
													    //destination :accept connection
														//self : local socket
}my_socket;

typedef struct socket_node
{
	my_socket sock_info;
	int conn_no;
	struct socket_node *next;
}my_conn;

extern int add_conn(my_conn **,my_socket *,int);
extern int remove_conn(my_conn **,int);
extern int search_conn(my_conn **,const char*);
extern void list_all_connections(my_conn **);
extern int get_conn_fd(my_conn **,int );
extern int get_conn_no(my_conn **,int );
extern const char* get_conn_name(my_conn **,int);
#endif /* CONNECTION_H_ */
