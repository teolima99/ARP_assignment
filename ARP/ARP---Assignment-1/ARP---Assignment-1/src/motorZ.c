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
float pos_Z = 0;
float vel_Z_inc = 0;
float vel_Z = 0;

const float Vel_max = 2;
const float Z_max = 10;
const float Z_min = 0;

char log_buffer[100];
int log_fd;

struct tm *info;
time_t rawtime;

// SendPos function to send the position values of motor Z to the world
void SendPos(const char *fifo_path, float pos_Z)
{
    int fd;
    if ((fd = open(fifo_path, O_WRONLY)) == -1)
    {
        close(log_fd);
        perror("Error opening fifo");
        exit(1);
    }
    if (write(fd, &pos_Z, sizeof(pos_Z)) == -1)
    {
        close(fd);
        close(log_fd);
        perror("Error in writing function");
        exit(1);
    }
    close(fd);
}

// Creating the Emergency Stop signal handler
void Stop_signal_handler(int sgn)
{
    if (sgn == SIGUSR1)
    {
        // Stop the motor Z and listening the stop signal
        vel_Z = 0;

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
        // Setting the motor Z velocity as -2 and listening the stop signal
        vel_Z = -2;

        if (signal(SIGUSR2, Reset_signal_handler) == SIG_ERR)
        {
            exit(1);
        }
    }
}

// creating a function to make the velocity calculation
float ComputeVel(const char *fifo_path, int cmd, float vel_Z)
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
            WriteLog("Vz-- button pressed.");
            return -1.0;
        }
        else if (cmd == 1)
        {
            WriteLog("Vz++ button pressed.");
            return 1.0;
        }
        else if (cmd == 2)
        {
            WriteLog("Vz Stop_signal_handler button pressed.");
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
    char *Vel_z_Fifo = "/tmp/Vel_z_Fifo";
    mkfifo(Vel_z_Fifo, 0666);
    char *Pos_z_Fifo = "/tmp/Pos_z_Fifo";
    mkfifo(Pos_z_Fifo, 0666);

    int cmd = -1;

    // Opening the log file and writing to the log file
    if ((log_fd = open("log/motorZ.log", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
    {
        perror("Error opening motorZ log file.");
        return 1;
    }

    // Listening the signals
    if (signal(SIGUSR1, Stop_signal_handler) == SIG_ERR || signal(SIGUSR2, Reset_signal_handler) == SIG_ERR)
    {
        // Close file descriptors
        close(Vel_z_Fifo);
        close(Pos_z_Fifo);
        close(log_fd);
        exit(1);
    }

    while (1)
    {
        time(&rawtime);
        info = localtime(&rawtime);

        // Compute velocity Z and positon Z
        vel_Z_inc = ComputeVel(Vel_z_Fifo, cmd, vel_Z);
        if (vel_Z_inc == 2)
        {
            vel_Z = 0;
            vel_Z_inc = 0;
        }
        else
        {
            vel_Z = vel_Z + vel_Z_inc;
            if (vel_Z < -Vel_max)
            {
                vel_Z = -Vel_max;
            }
            else if (vel_Z > Vel_max)
            {
                vel_Z = Vel_max;
            }
        }

        pos_Z = pos_Z + vel_Z;
        if (pos_Z < Z_min)
        {
            pos_Z = Z_min;
            vel_Z = 0;
        }
        else if (pos_Z > Z_max)
        {
            pos_Z = Z_max;
            vel_Z = 0;
        }

        SendPos(Pos_z_Fifo, pos_Z);

        sleep(1);
    }

    close(log_fd);
    return 0;
}
