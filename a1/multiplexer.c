/*
 * multiplexer.c
 *
 * Author: vikram
 */

#include "multiplexer.h"


void set_non_blocking(int sock_fd)
{
	int opts;

	opts = fcntl(sock_fd,F_GETFL);
	if (opts < 0) {
		perror("get error");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock_fd,F_SETFL,opts) < 0) {
		perror("set error)");
		exit(EXIT_FAILURE);
	}
	return;
}

void create_select_pool()
{
	int no_of_connection;
	FD_ZERO(&read_set);
	FD_SET(listen_fd,&read_set);
	FD_SET(std_fd,&read_set);
	//is_got_header = 0;

	for(no_of_connection = 0 ;no_of_connection < MAX_CONNECTION ;no_of_connection ++)
	{
		if(clients[no_of_connection] != 0)
		{
			//
			FD_SET(clients[no_of_connection],&read_set);
			if (clients[no_of_connection] > maxfd)
			{
				maxfd = clients[no_of_connection];
				//printf("IN CLIENT FD_SET %d = \n",maxfd);
				//break;
			}
		}
	}
}

static void accept_new_connection()
{
	int conn_fd;
	size_t len;
	int no_of_connection;
	char buff[MAX_LINE];
	char host[40];
	char service[20];
	const char *ip_addr;
	struct sockaddr_in client_addr;
	//FD_CLR(listen_fd,&read_set);
	len = sizeof(client_addr);
	conn_fd = accept(listen_fd,(struct sockaddr *)&client_addr,&len);

	if(conn_fd != -1)
	{
		ip_addr = inet_ntop(family, &client_addr.sin_addr, buff, sizeof(buff));
		getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, service, sizeof(service), 0);
		//Received connection
		sock_info = (my_socket *) malloc(sizeof(my_socket));
		int port =  atoi(service);
		sock_info->sock_type = DESTINATION;
		sock_info->port_no = port;
		strcpy(sock_info->host_name,host);
		strcpy(sock_info->ip_addr,ip_addr);
		sock_info->sock_fd = conn_fd;
		sock_info->port_no = ntohs(client_addr.sin_port);
		conn_no = conn_no + 1;
		if(add_conn(&conn,sock_info,conn_no) == 1)
		{
			//TODO : reomve the message as it is unnecessay
			show_dup_msg();
		}
		else {
			for (no_of_connection = 0; no_of_connection < MAX_CONNECTION; no_of_connection ++)
			{
				if (clients[no_of_connection] == 0) {
					//					printf("\nConnection accepted:   FD=%d; Slot=%d\n",
					//							conn_fd,no_of_connection);
					printf("Connection from <%s> accepted \n",host);
					clients[no_of_connection] = conn_fd;
					clients_write[no_of_connection] = conn_fd;
					conn_fd = -1;
					break;
				}
			}
		}
	}
	else
	{
		printf("\nNetwork is fully occupied.\n");
		//sock_puts(connection,"Sorry, this server is too busy. Try again later!\r\n");
		close(conn_fd);

	}
}

void accept_user_input()
{
	char input_line[MAX_LINE] = {0x0};
	char opt_line[MAX_LINE] = {0x0};
	//input is readable
	if(fgets(input_line,MAX_LINE,stdin) != NULL)
	{
		//write(fileno(stdout),send_line,strlen(send_line));
		strcpy(opt_line,input_line);
		result = strtok(opt_line,delim);
		if(result != NULL)
		{
			int i = compare_options(result);
			select_options(i,input_line);
		}
		else
		{
			show_invalid_msg();
		}
	}
}

void accept_data(int conn_no)
{
	request_header *local_header;
	//printf("data accept starts \n");
	int val_ = 0;
	double time_ = 0;
	char local_msg[128] = {0x0};
	char file_name_local[30] = {0x0};
	local_header = accept_header(clients[conn_no]);
	val_ = local_header->file_size;
	strcpy(file_name_local,local_header->file_name);
	if(val_ > -1)
	{
		start_timer();
		accept_content(clients[conn_no],local_header,&total_bytes_accept);
		time_ = get_time_diff();
		int f_size = local_header->file_size;
		sprintf(local_msg,
				"Rx(%s):\nReceived file: %s"
				"\nFile Size: %d Bytes,\n"
				"Time taken: %f sec,\n"
				"Rx rate: %f bps\n",local_host,local_header->file_name,f_size,time_,calulate_rate(f_size,time_));
		printf("%s \n",local_msg);
	}
	else if(val_ == -1){
		//Prepare for the download
		strcpy(file_to_download,local_header->file_name);
		FD_SET(clients[conn_no],&write_set);
		is_source_download = 1;
	}
	else if(val_ == -999){
		//Terminate the connection
		passive_terminate(clients[conn_no]);
	}
}

/*******************************************************************************************************
 *
 ******************************************************************************************************/
void execute_active_mode()
{
	int no_of_connection;
	if(FD_ISSET(listen_fd,&read_set))
	{
		accept_new_connection();
	}

	else if(FD_ISSET(std_fd,&read_set))
	{
		accept_user_input();
	}

	else {
		for(no_of_connection = 0;no_of_connection < MAX_CONNECTION;no_of_connection++)
		{
			if(FD_ISSET(clients[no_of_connection],&read_set))
			{
				accept_data(no_of_connection);
				//				printf("Accept bit cleared\n");
				//FD_CLR(clients[no_of_connection],&read_set);
			}
			if(FD_ISSET(clients[no_of_connection],&write_set))
			{
				if(is_source_upload == 1)
				{
					//printf("Uploading the data \n");
					send_wrapper(clients[no_of_connection]);
					is_source_upload = 0;
					FD_CLR(clients[no_of_connection],&write_set);
				}
				//}
				else if(is_req_download == 1)
				{
					//send the no of request i.e. run loop and check for descriptor
					int i = 1;
					for(i = 1;i <= 3;i++)  //Try to not to harcode this count
					{
						//printf("downloading request \n"); //args[2] is file
						if(clients[no_of_connection] == file_request[i-1].req_fd && file_request[i-1].is_complete == 0)
						{
							request_header_download(file_request[i-1].file_name,clients[no_of_connection]);
							download_count --;
							file_request[i-1].is_complete = 1;
							FD_CLR(clients[no_of_connection],&write_set);
							break;
						}
					}
					if(download_count == 0)	{
						is_req_download = 0;
					}
				}
				else if(is_source_download == 1){
					send_wrapper(clients[no_of_connection]);
					is_source_download = 0;
					FD_CLR(clients[no_of_connection],&write_set);
				}

			}
		}
	}
}
/*******************************************************************************************************
 * Execute the select call on the machine
 ****************************************************************************************************/

static void create_server_select()
{
	struct sockaddr_in server_addr;

	int is_read_availble;
	const int on = 1;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = family;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(local_port);
	listen_fd = socket(family,type,protocol);
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) == -1)
	{
		perror("unable to use the setsockopt");
	}

	set_non_blocking(listen_fd);
	maxfd = listen_fd;

	bzero(&clients,sizeof(clients));
	//memset((char *) &clients, 0, sizeof(clients));
	if(listen_fd >= 0) {
		if(bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr)) == 0 ) {
			if(listen(listen_fd,MAX_CONNECTION) == 0) {
				while(1) {
					create_select_pool();
					timeout.tv_sec = 10;
					timeout.tv_usec = 0;
					if ((is_read_availble = select(maxfd + 1,&read_set,&write_set,NULL,&timeout)) == -1){
						perror("Unable to execute the select call");
					}
					else if(is_read_availble == 0){
					}
					else {
						execute_active_mode();
					}
				}
			}
		}
	}
}

/******************************************************************************************************
 *
 ****************************************************************************************************/
static int get_local_fd(int no)
{
	return (get_conn_fd(&conn,no));
}


static void passive_terminate(int _fd)
{
	int _no = get_conn_no(&conn,_fd);
	terminate_local_conn(_no);
}

static int terminate_local_conn(int no)
{

	int _fd_ ,i;
	const char* del_host;
	del_host = get_conn_name(&conn,no);
	_fd_ = remove_conn(&conn,no);
	if(_fd_ != 0)
	{
		for(i =0;i<MAX_CONNECTION;i++)
		{
			if(clients[i] == _fd_)
			{
				//printf("client fd: %d  \n",clients[i]);
				printf("closed the connection <%s>\n",del_host); //TODO : show the closed connection
				FD_CLR(clients[i],&write_set);
				FD_CLR(clients[i],&read_set);
				close(clients[i]);
				clients[i] = 0;
				break;
			}
		}
	}
	return (1);
}
/*******************************************************************************************************
 * show argument error message
 *****************************************************************************************************/
static void show_arg_err_msg()
{
	printf("error: please provide correct no of arguments \n");
}
/*******************************************************************************************************
 * accepts the option_number and shows valid or invalid option
 *****************************************************************************************************/
static void select_options(int option_number,void *parameter)
{

	char *param = (char *) parameter;
	int is_terminated = 0;
	switch (option_number) {
	case 0:
		printf("invalid option please try help\n");
		break;
	case 1:
		//connect
		//determine the duplicate or self connection
		args = tokenize(param,3);
		if(args == (char **)1) {
			show_arg_err_msg();
		}
		else if(((strcasecmp(args[1],"localhost") == 0)) ||
				((strcasecmp(args[1],"127.0.0.1") == 0)) ) {
			show_dup_msg();
		}
		else if(peer_connect(args[1],args[2]) != SUCCESS) 	{
			//show_invalid_msg();
		}

		break;
	case 2:
		//upload
		args = tokenize(param,3);
		if(args == (char **)1)
		{
			show_arg_err_msg();
		}
		else
		{
			upload_wrapper();
		}
		break;
	case 3:
		//download
		args = tokenize(param,7);				//should support the 6 arguments
		if(args == (char **)1)
		{
			show_arg_err_msg();
		}
		else
		{
			download_wrapper();
		}
		break;
	case 4:
		//terminate
		//Use the shutdown
		args = tokenize(param,2);
		if(args == (char **)1)
		{
			show_arg_err_msg();
		}
		else
		{
			is_terminated = terminate_local_conn(atoi(args[1]));   //terminate <connection_no>
			//printf("client is terminated %d\n",is_terminated);
			//printf("closed the connection \n");
		}
		break;
	case 5:
		//myip
		printf("Machine ip is :%s\n",local_ip);
		break;
	case 6:
		//myport
		printf("Machine port is :%d\n",local_port);
		break;
	case 7:
		//creator
		show_creator();
		break;
	case 8:
		//Help
		show_help();
		break;
	case 9:
		//Exit
		exit(SUCCESS);
		break;
	case 10:
		//list all connection
		list_all_connections(&conn);
		break;
	default:
		break;
	}
	fflush(stdout);
}


static void send_wrapper(int _fd)
{
	double time_;
	char local_msg[128] = {0x0};
	request_header *local_header;

	if(is_source_upload == 1)
	{
		local_header = send_header(args[2],_fd);            //for upload send the header file size,file name
	}
	else if(is_source_download == 1) {
		printf("file :%s \n",file_to_download);
		local_header = send_header(file_to_download,_fd);
	}

	int f_size = local_header->file_size;
	start_timer();
	send_content(local_header,_fd);   //upload <connection_no> <file_name>
	time_ = get_time_diff();
	sprintf(local_msg,
			"Tx(%s):\nTranseferred file: %s"
			"\nFile Size: %d Bytes,\n"
			"Time taken: %f sec,\n"
			"Tx rate: %f bps\n",local_host,local_header->file_name,f_size,time_,calulate_rate(f_size,time_));
	printf("%s \n",local_msg);

}

static void upload_wrapper()
{
	printf("Uploading started......\n");
	int local_client_fd;
	local_client_fd = get_local_fd(atoi(args[1]));
	int i;
	for(i =0;i<MAX_CONNECTION;i++)
	{
		if(clients[i] == local_client_fd)
		{
			//			printf("client fd: %d  \n",clients[i]);
			//			printf("Upload bit set\n");
			FD_SET(clients[i],&write_set);
			is_source_upload = 1;
		}
	}

}


static void download_wrapper()
{
	printf("Downloading started......\n");
	int local_client_fd;
	int _iter = 1;
	int i;
	download_count = 0;
	for(_iter = 1;_iter <= 6;)
	{
		if(args[_iter] != NULL)
		{
			download_count = download_count + 1;					//1,2,3 not 0,1,2
			local_client_fd = get_local_fd(atoi(args[_iter]));		//args1,3,5 => fd's	//args 2,4,6 =>file names
			_iter = _iter + 1;
			strcpy(file_request[download_count - 1].file_name,args[_iter]);
			file_request[download_count - 1].req_fd = local_client_fd;
			file_request[download_count -1].is_complete = 0;
			_iter = _iter + 1;
			//accept the inputs i.e. maintain no of counts and files
			for(i =0;i<MAX_CONNECTION;i++)
			{
				if(clients[i] == local_client_fd)
				{
					//printf("client fd: %d  \n",clients[i]);
					//printf("download request \n");
					FD_SET(clients[i],&write_set);
					is_req_download = 1;
					break;
				}
			}
		}
		else {
			break;
		}

	}
}

/****************************************************************************************************
 * Compare the options
 ****************************************************************************************************/
static int compare_options(char *result)
{
	int i;
	char *option_value = (char*) malloc(sizeof(char) * strlen(result));
	memset(option_value,0,sizeof(option_value));
	strcpy(option_value,result);
	//printf("value %s and len :%d\n",option_value,strlen(option_value));
	for(i=0;commands[i].option != 0;i++) {
		if(strcasecmp(commands[i].option,option_value) == 0)
		{
			free(option_value);
			return (commands[i].no);
		}
	}
	return (0);
}

/*******************************************************************************************************
 *Connect to the peer
 ***************************************************************************************************/
static int peer_connect(char *conn_peer,const char* port_val)
{
	char *host_name;
	host_name = (char *)malloc(sizeof(char) * strlen(conn_peer));
	//printf("host name %s \n",host_name);
	struct addrinfo hints,*result_sock;
	struct sockaddr_in peer;
	int peer_len;
	int conn_fd;
	int err;
	int port_no_local;
	bzero((char*)host_name,sizeof(host_name));
	bzero(&hints,sizeof(hints));
	strcpy(host_name,conn_peer);
	hints.ai_family = family;
	hints.ai_socktype = type;
	hints.ai_protocol = protocol;
	port_no_local = atoi(port_val);
	if(search_conn(&conn,host_name) != 1 ) //search for the duplicate connection
	{
		if ((err = (getaddrinfo(host_name,port_val,&hints,&result_sock))) != 0){
			printf("Invalid host \n");
			return(FAILURE);
		}
		if((client_fd = socket(result_sock->ai_family,result_sock->ai_socktype,result_sock->ai_protocol)) != 0) {
			fflush(stdout);
			if((conn_fd = connect(client_fd,result_sock->ai_addr,result_sock->ai_addrlen)) == 0){
				printf("socket is created at port:%d\n",client_fd);
				peer_len = sizeof(peer);
				bzero((char*)&peer,sizeof(peer));

				if (getpeername(client_fd, &peer, &peer_len) == -1) {
					perror("Unable to get the peer address");
					return -1;
				}
				//printf("Peer's IP address is: %s\n", inet_ntoa(peer.sin_addr));
				//printf("Peer's port is: %d\n", (int) ntohs(peer.sin_port));
				//Sender connection
				sock_info = (my_socket *) malloc(sizeof(my_socket));;
				strcpy(sock_info->host_name,host_name);
				sock_info->sock_type = SOURCE;
				strcpy(sock_info->ip_addr,inet_ntoa(peer.sin_addr));
				sock_info->sock_fd = client_fd;
				sock_info->port_no = port_no_local; //ntohs(peer.sin_port);
				conn_no = conn_no + 1;
				if(add_conn(&conn,sock_info,conn_no) == 1) {
					//show_dup_msg();
				}
				else {
					int no_of_connection;
					for (no_of_connection = 0; no_of_connection < MAX_CONNECTION; no_of_connection ++)
					{
						if (clients[no_of_connection] == 0){
							//							printf("\nConnection accepted:   fd=%d; conn_no=%d\n",
							//									client_fd,no_of_connection);
							printf("Connection to <%s> created \n",host_name);
							clients[no_of_connection] = client_fd;
							clients_write[no_of_connection] = client_fd;
							client_fd = -1;
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		show_dup_msg();
	}
	return (SUCCESS);
}

/********************************************************************************************************
 * Creates a socket for the local machine
 *******************************************************************************************************/
static void initialize_my_connection()
{
	get_local_ip();
	sock_info = (my_socket *) malloc(sizeof(my_socket));
	//	printf("host name %s  and ip %s\n",local_host,local_ip);
	printf("%s started at : %d\n",local_host,local_port);
	sock_info->sock_type = SELF;
	strcpy(sock_info->host_name,local_host);
	strcpy(sock_info->ip_addr,local_ip);
	sock_info->port_no = local_port;
	sock_info->sock_fd = DEFAULT_SOCK_FD;
	if(add_conn(&conn,sock_info,conn_no) == 1)
	{
		show_dup_msg();
	}
}
/******************************************************************************************************
 * Get the local ip address the port
 ******************************************************************************************************/
static void get_local_ip()
{
	int sock_fd;
	int conn_fd;
	pid_t child;
	int status;
	struct sockaddr_in server_addr;
	struct sockaddr_in local_addr;
	socklen_t local_addr_len;
	char buf[IP_ADDR_LEN];

	int buflen = 16;
	uint16_t google_port_no = 53;
	const char* _dns_ip = DNS_SERVER;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = family;
	inet_aton(_dns_ip,&server_addr.sin_addr);
	server_addr.sin_port = htons(google_port_no);

	child = vfork();
	if(child >= 0)
	{
		if(child == 0) {
			//create the socket
			sock_fd = socket(family,type,protocol);
			if(sock_fd != -1 )
			{
				conn_fd = connect(sock_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
				if(conn_fd != -1)
				{
					local_addr_len = sizeof(local_addr);
					if (getsockname(sock_fd,(struct sockaddr*)&local_addr,&local_addr_len) != -1)
					{
						char service[20];
						getnameinfo(&local_addr, sizeof local_addr,local_host, sizeof local_host, service, sizeof service, 0);
						//printf("connection is %s \n",local_host);
						const char *ip_name = inet_ntop(family,&local_addr.sin_addr,buf,buflen);
						strcpy(local_ip,ip_name);
						//printf("The ip address of the machine is %s\n",local_ip);
					}
					close(conn_fd);
					close(sock_fd);
					exit(SUCCESS); //Exit the child process
				}
			}
		}
		else {
			wait(&status);
			//printf ("In the parent process socket fd %d\n",sock_fd);
		}
	}
	else {
		printf("Error in creating the child \n");
	}
	//return (0);
}
/*****************************************************************************************************
 * Start the timer
 ****************************************************************************************************/
static void start_timer()
{
	gettimeofday(&start_time,NULL);
}
/*****************************************************************************************************
 * end the timer and return in time difference in seconds
 ****************************************************************************************************/
static double get_time_diff()
{
	gettimeofday(&end_time,NULL);
	struct timeval local_time;
	double diff;
	if(start_time.tv_usec > end_time.tv_usec)
	{
		end_time.tv_usec += CONV_FACTOR_TIME;
		end_time.tv_sec --;
	}

	local_time.tv_sec = (end_time.tv_sec - start_time.tv_sec);
	local_time.tv_usec =  (end_time.tv_usec- start_time.tv_usec);
	diff = (double)	local_time.tv_sec;
	diff += ((double)local_time.tv_usec/CONV_FACTOR_TIME);
	printf("Elapsed time in seconds %f \n",diff);
	return (diff);

}
static double calulate_rate(int _file_size,double _time)
{
	_file_size = _file_size * 8; // _file_size in bytes convert it into bits
	return(_file_size/_time);	 //bps
}
static void show_dup_msg()
{
	printf("Duplicate connection not allowed \n");
}
static void show_creator()
{
	printf("This program is created by vikram sawant vikramsi[at]buffalo[dot]edu \n");
}

static void show_invalid_msg()
{
	printf ("You entered invalid option \n"
			"try HELP option\n");
}
static void show_help()
{
	printf("1.MYIP - show the local address of the machine \n"
			"2.CONNECT <HOST NAME> <PORT> - connect the specified host \n"
			"on specified port \n"
			"3.LIST - shows all connected host\n"
			"4.TERMINATE <CONNECTION NUMBER> \n"
			"6.UPLOAD <CONNECTION NUMBER> <FILE NAME> - upload specified file \n"
			"to specified connection \n"
			"7.DOWNLOAD <CONNECTION NO1> <FILE NAME1> .....<CONNECTION NO3> <FILE NAME3> -\n"
			"download specified file from specified connection \n"
			"8.MYPORT -shows the local port number\n"
			"on which process is listening \n"
			"9.CREATOR -shows the coder name\n"
			"10.EXIT - exit the process and terminate the connection \n"
			"11.HELP - shows the help menu \n");
}

static char** tokenize(const char* input,int length)
{
	char* str = strdup(input);
	int count = 0;
	char** result = malloc(length*sizeof(*result));
	char* tok=strtok(str,delim);
	while(1)
	{
		if (count > length)
		{
			printf("count %d : len %d\n",count,length);
			printf("please provide the correct number of argument\n");
			return ((char**)(1));
		}
		if (!tok)
		{
	//		if(count == length)	{
//				printf("%d : %d",count,length);
				break;
	//		}
	//		else{
	//			return ((char**)(1));
	//		}
		}
		result[count++] = strdup(tok);
		tok=strtok(NULL,delim);
	}
	free(str);
	return result;
}

int main(int argc, char **argv) {

	strcpy(port,argv[1]);
	local_port = atoi(port);
	initialize_my_connection();
	create_server_select();
	return (EXIT_SUCCESS);

}
