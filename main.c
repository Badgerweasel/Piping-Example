#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void syserror(const char *s);

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    pid_t pid;
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);
	char* cmd[num_words + 1];
	int cmdIndex = 0;
	int fd_in = 0;
	int p[2];
	int pipeFlag = 1;
        
        for (int i=0; i < num_words; i++)
	  {
	    if(strcmp(line_words[i], "|") == 0)
	      {
		if(pipe(p) < 0)
		  {
		    syserror("pipe failed");
		  }
		switch(pid = fork())
		  {
	          case -1:
		    syserror("fork failed");
	          case 0:
		    dup2(fd_in, 0);
		    dup2(p[1],1);
		    if(close(p[0]) < 0)
		      {
			syserror("could not close");
		      }
		    cmd[cmdIndex] = NULL;
		    execvp(cmd[0], cmd);
		    syserror("could not exec");
		  default:
		    while(wait(NULL) != -1);
		    if(close(p[1]) < 0)
		      {
			syserror("could not close");
		      }
		    fd_in = p[0];
		    cmdIndex = 0;
		  }
		
	      }
	    else if(strcmp(line_words[i], ">") == 0)
	      {
		switch(pid = fork())
		  {
	          case -1:
		    syserror("fork failed");
	          case 0:   
		    dup2(fd_in, 0);
		    int fd = open(line_words[i+1], O_WRONLY|O_CREAT|O_TRUNC, 0777);
		    if(fd < 0)
		      {
			syserror("could not open");
		      }
		    dup2(fd,1);
		    cmd[cmdIndex] = NULL;
		    execvp(cmd[0], cmd);
		    syserror("could not exec");
		  default:
		    while(wait(NULL) != -1);
		    pipeFlag = -1;
		  }
	      }
	    else if(strcmp(line_words[i], "<") == 0)
	      {
		if(pipe(p) < 0)
		  {
		    syserror("pipe failed");
		  }
		switch(pid = fork())
		  {
	          case -1:
		    syserror("fork failed");
	          case 0: ;
		    int fd = open(line_words[i+1], O_RDONLY, 0777);
		    if(fd < 0)
		      {
			syserror("could not open");
		      }
		    dup2(fd, 0);
		    dup2(p[1],1);
		    execlp("cat", "cat", NULL);
		    syserror("could not exec");
		  default:
		    while(wait(NULL) != -1);
		    if(close(p[1]) < 0)
		      {
			syserror("could not close");
		      }
		    fd_in = p[0];
		    i++;
		  }
	      }
	    else
	      {
		char* temp = line_words[i];
		if(strstr(temp, "\"") != NULL || strstr(temp, "\'") != NULL)
		  {
		    int end = strlen(temp)-1;
		    memmove(&temp[end], &temp[end + 1], strlen(temp) - end);
		    memmove(&temp[0], &temp[1], strlen(temp));
		    
		    
		  }
		cmd[cmdIndex] = temp;
		cmdIndex++;
	      }
	  }
	if(pipeFlag > 0)
	  {
	    if(pipe(p) < 0)
	      {
		syserror("pipe failed");
	      }
	    switch(pid = fork())
	      {
	      case -1:
		syserror("fork failed");
	      case 0:
		dup2(fd_in, 0);
		if(close(p[0]) < 0)
		  {
		    syserror("could not close");
		  }
		cmd[cmdIndex] = NULL;
		execvp(cmd[0], cmd);
		syserror("could not exec");
	      default:
		while(wait(NULL) != -1);
		if(close(p[1]) < 0)
		  {
		    syserror("could not close");
		  }
		fd_in = p[0];
	      }
	  }

    }

    return 0;
}


void syserror(const char *s)
{
  extern int errno;

  fprintf (stderr, "%s\n", s);
  fprintf( stderr, " (%s)\n", strerror(errno));
  exit(1);
}
