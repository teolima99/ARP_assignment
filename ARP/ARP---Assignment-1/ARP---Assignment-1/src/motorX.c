// Include libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>
#include <time.h>

// initialization
float pos_X = 0;
float vel_X = 0;
float vel_X_inc = 0;

const float Vel_max = 2;
const float X_max = 40;
const float X_min = 0;

char log_buffer[100];
int log_fd;

struct tm *info;
time_t rawtime;

// SendPos function to send the position values of motor X to the world
void SendPos(const char *fifo_path, float pos_X)
{
    int fd;
    if ((fd = open(fifo_path, O_WRONLY)) == -1)
    {
        close(log_fd);
        perror("Error opening fifo");
        exit(1);
    }
    if (write(fd, &pos_X, sizeof(pos_X)) == -1)
    {
        close(fd);
        close(log_fd);
        perror("Error in writing function");
        exit(1);
    }
    close(fd);
}

// Emergency Stop signal handler
void Stop_signal_handler(int sgn)
{
    if (sgn == SIGUSR1)
    {
        // Stop the motor X and listening the stop signal
        vel_X = 0;

        if (signal(SIGUSR1, Stop_signal_handler) == SIG_ERR)
        {
            exit(1);
        }
    }
}

// Reset signal handler
void Reset_signal_handler(int sgn)
{
    if (sgn == SIGUSR2)
    {
        // Setting velocity of motor X as -5 and listening the stop signal
        vel_X = -5;

        if (signal(SIGUSR2, Reset_signal_handler) == SIG_ERR)
        {
            exit(1);
        }
    }
}

// ComputeVel function to compute the velocity
float ComputeVel(const char *fifo_path, int cmd, float vel_X)
{
    int fd;
    if (fd = open(fifo_path, O_RDONLY | O_NDELAY))
    {
        if (read(fd, &cmd, sizeof(cmd)) == -1)
        {
            close(log_fd);
            close(fd);
            perror("Error reading from fifo");
            exit(1);
        }
        if (cmd == 0)
        {
            WriteLog("Vx-- button pressed.");
            return -1.0;
        }
        else if (cmd == 1)
        {
            WriteLog("Vx++ button pressed.");
            return 1.0;
        }
        else if (cmd == 2)
        {
            WriteLog("Vx stop button pressed.");
            return 2.0;
        }
        else
        {
            return 0.0;
        }
    }
    close(fd);
    perror("Error opening fifo");
    exit(1);
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
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    // Pipes initialization
    int fd;
    char *Vel_x_Fifo = "/tmp/Vel_x_Fifo";
    mkfifo(Vel_x_Fifo, 0666);
    char *Pos_x_Fifo = "/tmp/Pos_x_Fifo";
    mkfifo(Pos_x_Fifo, 0666);

    int cmd = -1;

    // Opening the log file and writing on it
    if ((log_fd = open("log/motorX.log", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
    {
        perror("Error opening motorX log file.");
        return 1;
    }

    // Listening the signals
    if (signal(SIGUSR1, Stop_signal_handler) == SIG_ERR || signal(SIGUSR2, Reset_signal_handler) == SIG_ERR)
    {
        // Close file descriptors
        close(Vel_x_Fifo);
        close(Pos_x_Fifo);
        close(log_fd);
        exit(1);
    }

    while (1)
    {
        time(&rawtime);
        info = localtime(&rawtime);

        // compute velocity X and positon X
        if (vel_X != -5)
        {
            vel_X_inc = ComputeVel(Vel_x_Fifo, cmd, vel_X);
            if (vel_X_inc == 2)
            {
                vel_X = 0;
                vel_X_inc = 0;
            }
            else
            {
                vel_X = vel_X + vel_X_inc;
                if (vel_X < -Vel_max)
                {
                    vel_X = -Vel_max;
                }
                else if (vel_X > Vel_max)
                {
                    vel_X = Vel_max;
                }
            }
        }

        pos_X = pos_X + vel_X;
        if (pos_X < X_min)
        {
            pos_X = X_min;
            vel_X = 0;
        }
        else if (pos_X > X_max)
        {
            pos_X = X_max;
            vel_X = 0;
        }

        SendPos(Pos_x_Fifo, pos_X);

        sleep(1);
    }

    close(log_fd);
    return 0;
}