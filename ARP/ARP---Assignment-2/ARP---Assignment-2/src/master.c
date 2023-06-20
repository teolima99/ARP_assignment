// Include libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

// Initialization
char Log_Buffer[100];
int FD_log;

struct tm *info;
time_t rawtime;

int spawn(const char *program, char *arg_list[])
{

  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
    WriteLog("Error while forking...");
    return 1;
  }
  else if (child_pid != 0)
  {
    return child_pid;
  }
  else
  {
    if (execvp(program, arg_list) == 0)
      ;
    perror("Exec failed");
    WriteLog("Exec failed");
    return 1;
  }
}

// WriteLog function to write the log file
void WriteLog(char *message)
{
  time(&rawtime);
  info = localtime(&rawtime);

  sprintf(Log_Buffer, message);
  sprintf(Log_Buffer + strlen(Log_Buffer), asctime(info));

  if (write(FD_log, Log_Buffer, strlen(Log_Buffer)) == -1)
  {
    close(FD_log);
    perror("Error in writing function");
  }
}

int main()
{
  if ((FD_log = open("log/master.log", O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
  {
    perror("Error opening master log file");
    return 1;
  }

  WriteLog("Master process started.");

  char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

  pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  WriteLog("A & B processes spawned.");

  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);

  WriteLog("Master process terminated.");

  close(FD_log);

  printf("Main program exiting with status %d\n", status);
  return 0;
}