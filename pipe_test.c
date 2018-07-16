#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PATH "/home/chenzheng/Documents/tinyhttpd/htdocs/hello.cgi"

int main(int argc, char* argv[])
{
    int pipefd[2];
    pid_t cpid;
    char buf[1024];
    size_t nread;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((cpid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (cpid == 0) { /* Child reads from pipe */
        //close(pipefd[1]);
        //while (read(pipefd[0], &buf, 1) > 0)
        //    write(STDOUT_FILENO, &buf, 1);
        //write(STDOUT_FILENO, "\n", 1);
        //close(pipefd[0]);
        //_exit(EXIT_SUCCESS);
        dup2(pipefd[0], STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execl("/usr/bin/python", "python", PATH, NULL);
    } else { /* Parent writes argv[1] to pipe */
        //close(pipefd[0]);
        //write(pipefd[1], argv[1], strlen(argv[1]));
        //close(pipefd[1]);
        //printf("STDOUT_FILENO = %d\n", STDOUT_FILENO);
        setvbuf(fdopen(STDOUT_FILENO, "w"), NULL, _IONBF, 0);
        if ((nread = read(pipefd[0], buf, 1024)) > 0)
            write(STDOUT_FILENO, buf, nread);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}
