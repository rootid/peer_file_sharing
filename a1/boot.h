/*
 * boot.h
 *
 *  Created on: Jan 25, 2012
 *      Author: vikram
 */

#ifndef BOOT_H_
#define BOOT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include <sys/select.h>
#include <sys/file.h>
#include <time.h>  //for start time and end time
#include <unistd.h>
#include <sys/stat.h> //file size
#include <errno.h> //for error no
#include <sys/ioctl.h>
#include <sys/wait.h>   // Wait for Process Termination

#define MAX_BUF_SIZE 150
#define MAX_LINE 1024
#define PORT_NO 4000
#define IP_ADDR_LEN 16
#define SUCCESS 0
#define FAILURE 1
#define SOCK_ADD_SIZE 128
#define LOCAL_HOST "127.0.0.1"
#define LOCAL_PORT 0000
#define DEFAULT_SOCK_FD 999
#define CHUNK_SIZE 512
#define CHUNK_HEADER_SIZE 128
#define HEADER_ACK_SIZE 11
#define HEADER_ACK_MSG "Got header"
#define CONTENT_ACK_MSG "Got content"
#define DNS_SERVER "8.8.8.8"
#endif /* BOOT_H_ */
