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

char port[10];
char adrs[20];
char prs[30];

int status;
int option;

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

  do
  {
    do
    {
      printf("Choose one of these 3 options:\n 1.Normal\n 2.Server\n 3.Client\n\nWrite the number here: ");
      scanf("%d", &option);

      if (option < 1 || option > 3)
      {
        printf("Error in the selection. Please try again.\n");
      }
    } while (option < 1 || option > 3);

    switch (option)
    {
    case 1:
      printf("Option ' Normal ' selected.\n");
      sprintf(prs, "./bin/processA");
      WriteLog("Option ' Normal ' selected.");
      break;
    case 2:
      printf("Option ' Server ' selected.\nInsert port: ");
      scanf("%s", port);
      sprintf(prs, "./bin/processAserver");
      WriteLog("Option ' Server ' selected.");
      break;
    case 3:
      printf("Option ' Client ' selected.\nInsert an address: ");
      scanf("%s", adrs);
      printf("Insert a port: ");
      scanf("%s", port);
      sprintf(prs, "./bin/processAclient");
      WriteLog("Option ' Client ' selected.");
      break;
    default:
      perror("Error selecting the option.");
      WriteLog("Error selecting the option.");
      return EXIT_FAILURE;
      break;
    }

    char *arg_list_A[] = {"/usr/bin/konsole", "-e", prs, port, adrs, NULL};
    char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

    pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

    WriteLog("A & B processes spawned.");

    waitpid(pid_procA, &status, 0);
    waitpid(pid_procB, &status, 0);

    if (status == -1)
    {
      printf("Encountered an error in process: %s\n", prs);
    }
    printf("Processes exiting with status %d\n\n", status);

  } while (status == 0 || status == -1);

  WriteLog("Master process terminated.");

  close(FD_log);

  printf("Main program exiting with status %d\n", status);
  return 0;
}