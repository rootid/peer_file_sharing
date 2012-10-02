/*
 * filetask.h
 *
 *  Created on: Feb 9, 2012
 *      Author: vikram
 */

#ifndef FILETASK_H_
#define FILETASK_H_

#include "boot.h"

//open read write file
//

typedef struct header_node
{
	char file_name[20];
	int file_size;
	int is_available; // 1 present 0 absent
}request_header;

void _send(int,int,int);
void _recv(int,int,int,int *);
extern void send_content(void *,int );
extern void accept_content(int,void *,int*);
extern int send_upload_response(int,const char*,size_t);
extern void* accept_header(int);
extern int send_header_ack(int,int *);
extern int send_content_ack(int);
extern void* send_header(char *,int);
extern void request_header_download(char *,int );

#endif /* FILETASK_H_ */
