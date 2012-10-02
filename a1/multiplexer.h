/*
 * multiplexer.h
 *
 *  Created on: Feb 3, 2012
 *      Author: vikram
 */

#ifndef MULTIPLEXER_H_
#define MULTIPLEXER_H_
#define MAX_CONNECTION 7
#define CONV_FACTOR_TIME 1000000


#include "boot.h"
#include "connection.h"
#include "filetask.h"


struct {
	char *option;
	int no;
} commands [] = {
		{"CONNECT", 1},
		{"UPLOAD", 2},
		{"DOWNLOAD",3},
		{"TERMINATE",4},
		{"MYIP",5},
		{"MYPORT",6},
		{"CREATOR",7},
		{"HELP",8},
		{"EXIT",9},
		{"LIST",10},
		{0,0} };


void perform_select ();
static void accept_new_connection();
static int compare_options(char *);
static void create_server_select();
static void initialize_my_connection();
static void select_options(int ,void *);
void create_client_socket ();
static int peer_connect(char *,const char*);
static void get_local_ip();
static void show_creator();
static void show_help();
static void show_invalid_msg();
static void show_dup_msg();
static int get_local_fd(int);
static int terminate_local_conn(int);
static void passive_terminate(int);
void conn_manipulation();
static void start_timer();
static double get_time_diff();
static double calulate_rate(int,double);
static char** tokenize(const char*,int);
static void upload_wrapper();
static void download_wrapper();
static void send_wrapper(int);
static void show_arg_err_msg();
static int conn_no = 0;
static int local_port;
static char port[16];
static const char local_host[40];
static const char local_ip[40];
int family = AF_INET; //IPv4
int type = SOCK_STREAM; //stream socket
int protocol = IPPROTO_TCP; //TCP transport protocol
int clients[MAX_CONNECTION] = {0x0};
int clients_write[MAX_CONNECTION] = {0x0};
int is_source_download = 0;
int is_source_upload = 0;
int is_req_download = 0;
int download_count = 0;
int listen_fd;
int client_fd;
int total_bytes_accept;
int maxfd;
int is_got_header = 0;
int is_send_header = 0;
const int std_fd = 0;
char *result = NULL;
char delim[] = " \n"; //space and newline
char file_to_download[30] ={0x0};
char **args;
fd_set read_set;
fd_set write_set;
my_conn *conn_list;
my_conn *conn = NULL;
my_socket *sock_info;
struct timeval timeout;
struct timeval start_time;
struct timeval end_time;
typedef struct file_struct
{
	char file_name[15];
	int req_fd;
	int is_complete;
}file_struct;
file_struct file_request[3];
#endif /* MULTIPLEXER_H_ */

