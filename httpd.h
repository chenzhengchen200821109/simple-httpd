#ifndef _HTTPD_H
#define _HTTPD_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

/* 
 * Following could be derived from SOMAXCONN in <sys/socket.h>, but many
 * kernels still define it as 5, while actually supporting many more
 */
#define LISTENQ 1024
#define SERV_PORT 5122

/*
 * Miscellaneous constants
 */
#define MAXLINE 4096
#define BUFFSIZE 8192

/*
 * Following shortens all the typecasts of pointer arguments
 */
#define SA struct sockaddr

#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "Server: tinyhttpd\r\n"
#define SOURCE_DIR "/home/chenzheng/Documents/tinyhttpd/htdocs"

/*
 * Error handlers
 */
void err_ret(const char* fmt, ...);
void err_sys(const char* fmt, ...);
void err_dump(const char* fmt, ...);
void err_msg(const char* fmt, ...);
void err_quit(const char* fmt, ...);

/*
 * I/O handlers
 */
ssize_t readline(int fd, void* buf, size_t maxlen);
#endif
