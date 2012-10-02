/*
 * connection.c
 * Author: vikram
 */


#include "connection.h"


/********************************************************************************************************
 *Add the new connection in list
 ******************************************************************************************************/
int add_conn(my_conn **start,my_socket *sock,int conn_no)
{
	my_conn **temp = start;
	my_conn *new = NULL;
	while((*temp))
	{  	//verfiy the duplicate and self connection
		if(strcasecmp((*temp)->sock_info.host_name,sock->host_name) == 0)
		return (1);
		temp = &(*temp)->next;
	}
	new = (my_conn *) malloc(sizeof(my_conn));
	new->conn_no = conn_no;
	new->sock_info.sock_type = sock->sock_type;     //Added the type of socket and ip address
	strcpy(new->sock_info.ip_addr,sock->ip_addr);
	strcpy(new->sock_info.host_name,sock->host_name);
	new->sock_info.port_no = sock->port_no;
	new->sock_info.sock_fd = sock->sock_fd;
	new->next = NULL;
	*temp = new;
	return (0);
}
/*******************************************************************************************************
 * Remove the connection
 ******************************************************************************************************/
int remove_conn(my_conn **start,int conn_no)
{
	my_conn **temp = start;
	int fd_;
	while((*temp))
	{
		if((*temp)->conn_no == conn_no)
		{
			fd_ = (*temp)->sock_info.sock_fd;
			*temp = (*temp)->next;
			return (fd_);
		}
		temp = &(*temp)->next;
	}

	return (0);
}

/********************************************************************************************************
 * Search for the duplicate connection
 * return int : 1 if entry found
 * 			  : 0 if entry not found
 ******************************************************************************************************/

int search_conn(my_conn **conn_list,const char* conn_name)
{
	my_conn **temp = conn_list;
	while((*temp))
	{
		//TODO : add "localhost" ,"127.0.0.1" for the comparison
		if(strcasecmp((*temp)->sock_info.host_name,conn_name) == 0)
		{

			return (1);
		}
		temp = &(*temp)->next;
	}
	return (0);
}

/*****************************************************************************************************
 * List all the connection
 ****************************************************************************************************/
void list_all_connections(my_conn **conn_list)
{
	int is_connection = 0;
	printf("All the connection info :\n");
	my_conn **conn = conn_list;
	printf("\n");
	conn = &(*conn)->next ;
	while((*conn))
	{
		printf("%d : (%s :%d) fd:( %d )\n",(*conn)->conn_no,
				(*conn)->sock_info.host_name,(*conn)->sock_info.port_no,(*conn)->sock_info.sock_fd);
		conn = &(*conn)->next ;
		is_connection ++;
	}

	if(is_connection == 0)
	{
		printf("Yet there is no connection\n");
	}
}
/***************************************************************************************************
 * return
 * int : descriptor of the connection number
 *************************************************************************************************/
int get_conn_fd(my_conn **conn_list,int conn_no_)
{
	my_conn **temp = conn_list;
	while((*temp))
	{
		if((*temp)->conn_no == conn_no_)
		{
			return ((*temp)->sock_info.sock_fd);
		}
		temp = &(*temp)->next;
	}
	return (0);
}

/****************************************************************************************************
 * return
 * int : connection number
 ************************************************************************************************/
int get_conn_no(my_conn **conn_list,int _fd)
{
	my_conn **temp = conn_list;
	while((*temp))
	{
		if((*temp)->sock_info.sock_fd == _fd)
		{
			return ((*temp)->conn_no);
		}
		temp = &(*temp)->next;
	}
	return (0);
}

/****************************************************************************************************
 * return
 * const char* : connection name
 ************************************************************************************************/
const char* get_conn_name(my_conn **conn_list,int _no)
{
	my_conn **temp = conn_list;
	while((*temp))
	{
		//printf("%d \n",(*temp)->conn_no);
		if((*temp)->conn_no == _no)
		{
			//printf("%s \n",(*temp)->sock_info.host_name);
			return ((*temp)->sock_info.host_name);
		}
		temp = &(*temp)->next;
	}
	return (0);
}
