/* J. David's webserver */
/* This is a simple webserver.
 * Created November 1999 by J. David Blackstone.
 * CSE 4344 (Network concepts), Prof. Zeigler
 * University of Texas at Arlington
 */
/* This program compiles for Sparc Solaris 2.6.
 * To compile for Linux:
 *  1) Comment out the #include <pthread.h> line.
 *  2) Comment out the line that defines the variable newthread.
 *  3) Comment out the two lines that run pthread_create().
 *  4) Uncomment the line that runs accept_request().
 *  5) Remove -lsocket from the Makefile.
 */
#include <string>
#include "httpd.h"
#include "httpd.hpp"

int getFileType(const char* filename);
void process_request(int sockfd, char* req, size_t len);
void* accept_request(void*);
void bad_request(int);
void cat(int, FILE*);
void cannot_execute(int);
void error_die(const char*);
void execute_cgi(int, const char*, const char*, const char*);
int get_line(int, char*, int);
void headers(int, const char*);
void not_found(int);
void serve_file(int, const char*);
int startup(u_short*);
void unimplemented(int);

typedef enum { HTML=0, CGI=1, OTHERS } FileType;

/**********************************************************************/
/* return the file type given its file name. Currently, only html and 
 * cgi is supported and others will be ignored */
/**********************************************************************/
int getFileType(char* filename) 
{
    char* type;
    if ((type = strrchr(filename, '.')) == NULL)
        return OTHERS;
    if (strcasecmp(type + 1, "HTML") == 0)
        return HTML;
    else if (strcasecmp(type + 1, "CGI") == 0)
        return CGI;
    else
        return OTHERS;

}

void process_request(int sockfd, char* buf, size_t len)
{
    char method[255];
    char url[255];
    char path[512] = SOURCE_DIR;
    size_t i, j;
    int cgi = 0;
    char* query_string = NULL;
    char* filename;

    i = 0; j = 0;
    // skip blank space
    while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
    {
        method[i] = buf[j];
        i++; j++;
    }
    method[i] = '\0';

    // only supprt "GET" and "POST" method in this httpd
    if ((strcasecmp(method, "GET") != 0) && (strcasecmp(method, "POST") != 0))
    {
        unimplemented(sockfd);
        return; // newthread is terminated here
    }

    //if (strcasecmp(method, "POST") == 0)
    //    cgi = 1;

    i = 0;
    // skip blank space 
    while (ISspace(buf[j]) && (j < sizeof(buf)))
        j++;
    while (!isspace(buf[j])) {
        url[i] = buf[j];
        i++;
        j++;
    }
    url[i] = '\0';

    // "GET" method
    if (strcasecmp(method, "GET") == 0)
    {
        query_string = url;
        while ((*query_string != '?') && (*query_string != '\0'))
            query_string++;
        if (*query_string == '?')
        {
            cgi = 1;
            *query_string = '\0';
            query_string++;
        }
    }

    strcat(path, url);
    switch(getFileType(url)) {
        case HTML:
            serve_file(sockfd, path);
            //printf("serve_file\n");
            break;
        case CGI:
            execute_cgi(sockfd, path, method, query_string);
            break;
        case OTHERS:
            not_found(sockfd);
            break;
        }
    //close(sockfd);
    //pthread_exit(NULL);
    return;
}

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
void* accept_request(void* arg)
{
    int sockfd;
    char buf[1024];
    ssize_t n;

    pthread_detach(pthread_self());
    sockfd = (int)arg;
    if ((n = get_line(sockfd, buf, sizeof(buf))) > 0)
        process_request(sockfd, buf, n);
    //close((int)arg);
    return (NULL);
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void bad_request(int sockfd)
{
    StartLine sl;
    Headers h;
    MesgBody m("<P>Your browser send a bad request. such as a POST without a Content-Length.");
    std::string s;

    s.append(sl.getStartLine(400));
    h.setContentType("text/html");
    h.setContentLength(m.size());
    h.setServer("tinyhttpd");

    s.append(sl.getStartLine(400));
    s.append(h.getContentType());
    s.append(h.getContentLength());
    s.append(h.getServer());
    s.append(m.getMesg());
    
    size_t size = s.size();
    send(sockfd, s.c_str(), size, 0);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/
void cat(int client, FILE *resource)
{
    char buf[1024];

    fgets(buf, sizeof(buf), resource);
    while (!feof(resource))
    {
        send(client, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), resource);
    }
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int sockfd)
{
    std::string s;
    s.append("HTTP/1.0 500 Internal Server Error\r\n");
    s.append("Content-type: text/html\r\n");
    s.append("\r\n");
    s.append("<P>Error prohibited CGI execution.\r\n");
    size_t len = s.size();
    send(sockfd, s.c_str(), len, 0); 
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi(int sockfd, const char* path, const char* method, const char* query_string)
{
    char buf[1024];
    //int cgi_output[2];
    //int cgi_input[2];
    int pipefd[2];
    pid_t pid;
    int status;
    int i;
    char c;
    int nread;
    int content_length = -1;

    //if (strcasecmp(method, "GET") == 0)
    //    while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
    //        numchars = get_line(sockfd, buf, sizeof(buf));
    //else    /* POST */
    //{
    //    numchars = get_line(sockfd, buf, sizeof(buf));
    //    while ((numchars > 0) && strcmp("\n", buf))
    //    {
    //        buf[15] = '\0';
    //        if (strcasecmp(buf, "Content-Length:") == 0)
    //        content_length = atoi(&(buf[16]));
    //        numchars = get_line(sockfd, buf, sizeof(buf));
    //    }
    //    if (content_length == -1) {
    //        bad_request(sockfd);
    //        return;
    //    }
    //}

    //sprintf(buf, "HTTP/1.0 200 OK\r\n");
    //send(sockfd, buf, strlen(buf), 0);

    if (pipe(pipefd) == -1) {
        cannot_execute(sockfd);
        return;
    }

    if ((pid = fork()) == -1) {
        perror("fork");
        cannot_execute(sockfd);
        return;
    }
    if (pid == 0)  /* child: CGI script */
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        // environment variables
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);
        if (strcasecmp(method, "GET") == 0) { /* GET */
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }
        else { /* POST */
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }
        execl("/usr/bin/python", "python", path, NULL);
        //exit(0);
    } else { /* parent */
        setvbuf(fdopen(STDOUT_FILENO, "w"), NULL, _IONBF, 0);
        if (strcasecmp(method, "POST") == 0) {
            for (i = 0; i < content_length; i++) {
                recv(sockfd, &c, 1, 0);
                write(pipefd[1], &c, 1);
            }
        }
        sprintf(buf, "HTTP/1.0 200 OK\r\n");
        send(sockfd, buf, strlen(buf), 0);
        while ((nread = read(pipefd[0], buf, sizeof(buf))) > 0)
            send(sockfd, buf, nread, 0);
        waitpid(pid, &status, 0);
    }
}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int get_line(int sockfd, char* buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n')) {
        n = recv(sockfd, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                n = recv(sockfd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(sockfd, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        } else {
            c = '\n';
        }
    }
    buf[i] = '\0';
    return i;
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
void headers(int sockfd, const char* filename)
{
    std::string s;
    s.append("HTTP/1.0 200 OK\r\n");
    s.append(SERVER_STRING);
    s.append("Content-Type: text/html\r\n");
    s.append("Content-Length: 10\r\n");
    s.append("\r\n");
    size_t len = s.size();
    send(sockfd, s.c_str(), len, 0);
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int sockfd)
{
    std::string s;
    s.append("HTTP/1.0 404 NOT FOUND\r\n");
    s.append(SERVER_STRING);
    s.append("Content-Type: text/html\r\n");
    s.append("\r\n");
    s.append("<HTML><TITLE>Not Found</TITLE>\r\n");
    s.append("<BODY><P>The server could not fulfill your request because the resource specified\r\n");
    s.append("is unavailable or nonexistent.\r\n</BODY></HTML>\r\n");
    size_t len = s.size();
    send(sockfd, s.c_str(), len, 0);
}

/**********************************************************************/
/* Send a regular file to the client. Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/
void serve_file(int client, const char *filename)
{
    FILE *resource = NULL;

    resource = fopen(filename, "r");
    if (resource == NULL)
        not_found(client);
    else
    {
        headers(client, filename);
        cat(client, resource);
    }
    fclose(resource);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
void unimplemented(int sockfd)
{
    //start line of response message
    std::string s("HTTP/1.0 501 Method Not Implemented\r\n");
    s.append(SERVER_STRING);
    s.append("Content-Type: text/html\r\n");
    s.append("\r\n"); // blank line
    s.append("<HTML><HEAD><TITLE>Method Not Implemented\r\n</TITLE></HEAD>\r\n");
    s.append("<BODY><P>HTTP request method not supported.\r\n</BODY></HTML>\r\n");
    size_t len = s.size();
    send(sockfd, s.c_str(), len, 0);
}

/**********************************************************************/

int main(void)
{
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
    pthread_t newthread;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        err_sys("socket failed");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT); /* httpd server */

    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) 
        err_sys("bind failed");
    listen(listenfd, LISTENQ);

    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue;
            else
                err_sys("accept failed");
        }
        if (pthread_create(&newthread , NULL, &accept_request, (void *)connfd) != 0)
            err_sys("pthread_create");
    }
}
