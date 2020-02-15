#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void loop_pipe(char ***cmd);

int main()
{
  char *ls[] = {"ls", NULL};
  char *grep[] = {"grep", "pipe", NULL};
  char *wc[] = {"wc", NULL};
  char **cmd[] = {ls, grep, wc, NULL};

  loop_pipe(cmd);
  return 0;
}

void loop_pipe(char ***cmd)
{
  int p[2];
  pid_t pid;
  int fd_in = 0;

  while(*cmd != NULL)
  {
    pipe(p);
    if((pid = fork()) == -1)
    {
      exit(1);
    }
    else if(pid == 0)
    {
      dup2(fd_in, 0);
      if(*(cmd + 1) != NULL)
      {
	dup2(p[1], 1);
      }
      close(p[0]);
      execvp((*cmd)[0], *cmd);
      exit(1);
    }
    else
      {
	wait(NULL);
	close(p[1]);
	fd_in = p[0];
	cmd++;
      }
  }
}
