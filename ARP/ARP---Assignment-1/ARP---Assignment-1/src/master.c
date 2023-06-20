// Include libraries
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <time.h>

// Initialization
pid_t motor_Z_pid;
pid_t motor_X_pid;
pid_t world_pid;
pid_t pid_cmd;
pid_t pid_insp;

char log_buffer[100];
int log_fd;

struct tm *info;
time_t rawtime;

int watchdog()
{
  // Array of the log file paths
  char *vet_log_files[5] = {"log/command.log", "log/motorX.log", "log/motorZ.log", "log/world.log", "log/inspection.log"};

  // Array of the PIDs
  pid_t vet_pid[5] = {pid_cmd, motor_Z_pid, motor_Z_pid, world_pid, pid_insp};

  // Flag to check if a file was modified
  int mod;

  // Variable to keep the number of seconds since the last modification
  int n_sec = 0;

  while (1)
  {

    // Get current time
    time_t current_time = time(NULL);

    for (int i = 0; i < 5; i++)
    {

      // Get the last modified time of the log file
      time_t last_time_modified = last_time_file_modified(vet_log_files[i]);

      // Check if the file was modified in the last 3 seconds
      if (current_time - last_time_modified >= 2)
      {
        mod = 0;
      }
      else
      {
        mod = 1;
        n_sec = 0;
      }
    }

    if (mod == 0)
    {
      n_sec += 2;
    }

    // If the counter is greater than 60, kill the child processes
    if (n_sec >= 60)
    {
      WriteLog("Master process terminated after 60 seconds of inactivity.");
      close(log_fd);
      // Kill all the processes
      kill(pid_cmd, SIGKILL);
      kill(motor_X_pid, SIGKILL);
      kill(motor_Z_pid, SIGKILL);
      kill(world_pid, SIGKILL);
      kill(pid_insp, SIGKILL);

      return 0;
    }

    sleep(2);
  }
}

int spawn(const char *program, char *arg_list[])
{

  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
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
    return 1;
  }
}

// Function to know last time the file was modified
time_t last_time_file_modified(char *filename)
{
  struct stat attr;
  stat(filename, &attr);
  return attr.st_mtime;
}

// WriteLog function to write the log file
void WriteLog(char *message)
{
  sprintf(log_buffer, message);
  sprintf(log_buffer + strlen(log_buffer), asctime(info));

  if (write(log_fd, log_buffer, strlen(log_buffer)) == -1)
  {
    close(log_fd);
    perror("Error in writing function");
  }
}

int main()
{
  // Opening and writing to log file
  if ((log_fd = open("log/master.log", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
  {
    perror("Error opening log file");
    return 1;
  }

  time(&rawtime);
  info = localtime(&rawtime);

  WriteLog("Master process started.");

  char *arg_list_command[] = {"/usr/bin/konsole", "-e", "./bin/command", NULL};
  char *arg_list_motorX[] = {"./bin/motorX", NULL};
  char *arg_list_motorY[] = {"./bin/motorZ", NULL};
  char *arg_list_world[] = {"./bin/world", NULL};

  pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  motor_Z_pid = spawn("./bin/motorZ", arg_list_motorY);
  motor_X_pid = spawn("./bin/motorX", arg_list_motorX);
  world_pid = spawn("./bin/world", arg_list_world);

  // Convert motor X pid to string
  char motor_X_pid_str[10];
  sprintf(motor_X_pid_str, "%d", motor_X_pid);

  // Convert motor Z pid to string
  char motor_Z_pid_str[10];
  sprintf(motor_Z_pid_str, "%d", motor_Z_pid);

  char *arg_list_inspection[] = {"/usr/bin/konsole", "-e", "./bin/inspection", motor_X_pid_str, motor_Z_pid_str, NULL};
  pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  // Create the log files
  int fd_cmd = open("log/command.log", O_CREAT | O_RDWR, 0666);
  int fd_insp = open("log/inspection.log", O_CREAT | O_RDWR, 0666);
  int fd_motor_Z = open("log/motorZ.log", O_CREAT | O_RDWR, 0666);
  int fd_motor_X = open("log/motorX.log", O_CREAT | O_RDWR, 0666);
  int fd_world = open("log/world.log", O_CREAT | O_RDWR, 0666);

  if (fd_cmd < 0 || fd_insp < 0 || fd_motor_X < 0 || fd_motor_X < 0 || fd_world < 0)
  {
    printf("Error opening FILE");
  }

  // Close the log files
  close(fd_cmd);
  close(fd_insp);
  close(fd_motor_Z);
  close(fd_motor_X);
  close(fd_world);

  // Whatchdog funcion called
  watchdog();

  // Get the time when the Master finishes its execution
  time(&rawtime);
  info = localtime(&rawtime);

  WriteLog("Master process terminated.");

  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_insp, &status, 0);
  waitpid(motor_Z_pid, &status, 0);
  waitpid(motor_X_pid, &status, 0);
  waitpid(world_pid, &status, 0);

  printf("Main program exiting with status %d\n", status);
  close(log_fd);
  return 0;
}