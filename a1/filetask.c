/*
 * filetask.c
 *Author: vikram
 */


#include "filetask.h"

//TOLIST ECONNRESET 54 = connection reset by the peer
static void calculate_chunks(int size);
struct {
	char *file_type;
	int index;
} extension[] = {
		{"jpeg",1},
		{"gif",2},
		{"png",3},
		{"jpg",4},
		{"bmp",5},
		{0,0}};

extern int errno;
int total_chunks;
int partial_chunks;
const char *folder = "cse_50026708";

//TOLIST EPIPE 32 : BRoken pipe if client terminate the connection
/*****************************************************************************************************
 *
 *****************************************************************************************************/
int send_upload_response(int sock_fd,const char*msg,size_t len)
{

	size_t bytes_sent = 0;
	int this_write;

	while (bytes_sent < len) {
		do
		{
			printf("write response started \n");
			this_write = write(sock_fd,msg, len - bytes_sent);
			if(errno == EPIPE) {
				break;
			}
		}while ( (this_write < 0) && (errno == EINTR) );
		if (this_write <= 0)
			return this_write;
		bytes_sent += this_write;
		msg += this_write;
	}
	return len;

}

/*****************************************************************************************************
 *Create the directory to download/accept upload files
 *****************************************************************************************************/
static int create_dir()
{
	int success = mkdir(folder, 0700);
	if(success == 0)
	{
		printf("ok \n");
	}
	else
	{
		//printf("err no %d %s\n",errno,strerror(errno));
	}
	return (success);
}

/*****************************************************************************************************
 *calculate the required number of chunks
 *****************************************************************************************************/
static void calculate_chunks(int size)
{
	total_chunks = size / CHUNK_SIZE;
	partial_chunks = size % CHUNK_SIZE;
}

/*****************************************************************************************************
 *Send request header download
 *****************************************************************************************************/

void request_header_download(char *_file_name,int _sock_fd)
{
	char header [CHUNK_HEADER_SIZE] = {0x0};
	sprintf(header,"%s#%d#%d",_file_name,-1,-1);
	write(_sock_fd,header,CHUNK_HEADER_SIZE);
	//printf("header for download send \n");

}

//header for the sender
/*****************************************************************************************************
 *Send the header
 *****************************************************************************************************/
void* send_header(char *_file_name,int _sock_fd)
{
	struct stat st;
	off_t size;
	//send the header first
	stat(_file_name, &st);
	size =  st.st_size;
	char header [CHUNK_HEADER_SIZE] = {0x0};
	int _is_available = 1;
	sprintf(header,"%s#%d#%d",_file_name,(int)size,_is_available);
	write(_sock_fd,header,CHUNK_HEADER_SIZE);
//	printf("header send \n");

	//Store the information
	request_header *header_ptr = (request_header *) malloc(sizeof(request_header));
	bzero((request_header *) header_ptr,sizeof(request_header));
	strcpy(header_ptr->file_name,_file_name);
	header_ptr->file_size = size;
	header_ptr->is_available = 1;		//TOLIST: try to verify the availability of the file
	return (header_ptr);
}

/*****************************************************************************************************
 *send the header acknowledgment
 *****************************************************************************************************/
int send_header_ack(int sock_fd,int *response_tag)
{
	const char *msg = HEADER_ACK_MSG;
	int ok_ = write(sock_fd,msg,strlen(msg));
	printf("___________ok %d____________",ok_);
	*response_tag = 1;
	return (ok_);
}
/*****************************************************************************************************
 *send the content acknowledgment
 *****************************************************************************************************/
int send_content_ack(int _sock_fd)
{
	const char *msg = CONTENT_ACK_MSG;
	int ok_ = write(_sock_fd,msg,strlen(msg));
	printf("___________ok %d____________",ok_);
	return (ok_);
}

/*****************************************************************************************************
 *accept the header and content
 *****************************************************************************************************/
void* accept_header(int sock_fd)
{
	int read_cnt = 0;
	int cnt_ = 0;
	char* token;
	char delim[] = "#";
	char data_chunk[CHUNK_HEADER_SIZE] = {0x0};
	request_header *header_ptr;
	header_ptr = (request_header *) malloc(sizeof(request_header));
	bzero((request_header *) header_ptr,sizeof(request_header));
	char** args_ = malloc( 3 * sizeof(*args_));
	read_cnt = read(sock_fd,data_chunk,CHUNK_HEADER_SIZE);
	//printf("received data %s with read count %d\n",data_chunk,read_cnt);
	if(read_cnt <= 0)
	{
		if(errno == EWOULDBLOCK || errno == EPIPE)
		{
			perror("header accept error\n");
			//return ((void*)(1));
		}
		header_ptr->file_size = -999;
		return (header_ptr);
	}
	else {
		token = strtok(data_chunk,delim);
		while(1) {
			if (!token)
			{
				break;
			}
			else {
				args_[cnt_++] = strdup(token);
				token = strtok(NULL,delim);
			}
		}
		//printf("read cnt %d\n",read_cnt);
		strcpy(header_ptr->file_name,args_[0]);
		header_ptr->file_size = atoi(args_[1]);
		header_ptr->is_available = atoi(args_[2]);
		//printf("received data %s : %d\n",header_ptr->file_name,header_ptr->file_size);
	}

	return (header_ptr);
}
/*****************************************************************************************************
 *accept the content
 *****************************************************************************************************/
void accept_content(int sock_fd,void *_header,int *_bytes_accepted) //also accept_download
{

	printf("Content accept started \n");
	int write_fd;
	request_header *header = (request_header *)_header;
	calculate_chunks(header->file_size);
	char file_name[64];

	int temp_dir = create_dir();
	if(temp_dir != 0)
	{
		//printf("Directory already created \n");
	}
	sprintf(file_name,"%s/%s",folder,header->file_name);
	printf("file name :%s\n",file_name);
	write_fd = open(file_name,O_CREAT|O_WRONLY,0777);
	_recv(sock_fd,write_fd,header->file_size,_bytes_accepted);

}
/*****************************************************************************************************
 *returns number of bytes present at descriptor
 *****************************************************************************************************/
int get_n_readable_bytes(int fd) {
	int n = -1;
	if (ioctl(fd, FIONREAD, &n) < 0) {
		return -1;
	}
	return n;
}
/*****************************************************************************************************
 *send the content to receiver
 *****************************************************************************************************/
void _send(int _sock_fd,int fd,int size)
{
	uint8_t buff[CHUNK_SIZE];
	int actually_read;
	int cnt_;
	while(size !=0)
	{
		actually_read = read(fd, buff, sizeof(buff));
		cnt_ = send(_sock_fd,buff,actually_read,0);
		size = size - cnt_;
	}
	close(fd);
	//printf("Sended all data \n");
}
/*****************************************************************************************************
 *Receive content from sender
 *****************************************************************************************************/
void _recv(int _sock_fd,int _fd,int size,int *_bytes)
{
	uint8_t buff[CHUNK_SIZE];
	int actually_read;
	while(size != 0)
	{
		actually_read = recv(_sock_fd, buff, sizeof(buff),0);
		write(_fd,buff,actually_read);
		size = size - actually_read;
		*_bytes = size;
	}
	//printf("recved complete\n");
	close(_fd);
}

/*****************************************************************************************************
 *send the content
 *****************************************************************************************************/
void send_content(void *_header_ptr,int sock_fd)
{

	//printf("content send started \n");
	int fd;
	request_header *header_ptr = (request_header *) _header_ptr;
	calculate_chunks(header_ptr->file_size);
	fd = open(header_ptr->file_name,O_RDONLY,0755);
	_send(sock_fd,fd,header_ptr->file_size);
	//printf("content send ended \n");
}
