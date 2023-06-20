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
#include <time.h>
#include <math.h>

// Initialization
float Prev_pos_X = 0;
float pos_X = 0;
float Actual_pos_X = 0;
float Prev_pos_Z = 0;
float pos_Z = 0;
float Actual_pos_Z = 0;

float Pos[2];

const float X_max = 40;
const float X_min = 0;
const float Z_max = 10;
const float Z_min = 0;

char log_buffer[100];
int log_fd;

struct tm *info;
time_t rawtime;

// SendPos function to send the position values
void SendPos(char *fifo_path, float Actual_Pos[2])
{
    int fd;
    if ((fd = open(fifo_path, O_WRONLY)) == -1)
    {
        close(log_fd);
        perror("Error opening fifo");
        exit(1);
    }
    if (write(fd, Actual_Pos, sizeof(Actual_Pos)) == -1)
    {
        close(log_fd);
        perror("Error in writing function");
        exit(1);
    }
    // closing the pipe
    close(fd);
}

// creation of a random error
float GenerateError(float Pos_tmp)
{
    if (Pos_tmp == 0.0)
    {
        return 0.0;
    }
    int rand_err = (int)(Pos_tmp);
    int n = (rand() % (rand_err - (-rand_err) + 1)) + (-rand_err);
    WriteLog("Error generated.");
    return (float)n / 100;
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
    char *Pos_z_Fifo = "/tmp/Pos_z_Fifo";
    char *Pos_x_Fifo = "/tmp/Pos_x_Fifo";
    char *Actual_Pos_Fifo = "/tmp/Actual_Pos_Fifo";
    mkfifo(Pos_z_Fifo, 0666);
    mkfifo(Pos_x_Fifo, 0666);
    mkfifo(Actual_Pos_Fifo, 0666);

    // Opening the log file and writing on it
    if ((log_fd = open("log/world.log", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
    {
        perror("Error opening world log file");
        return 1;
    }

    while (1)
    {
        // Taking the current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Accessing the pipe coming from motor Z and retrieving the position Z
        if (fd = open(Pos_z_Fifo, O_RDONLY))
        {
            if (read(fd, &pos_Z, sizeof(pos_Z)) == -1)
            {
                close(log_fd);
                close(fd);
                perror("Error reading fifo");
                exit(1);
            }
            close(fd);
        }
        else
        {
            close(log_fd);
            close(fd);
            perror("Error opening fifo");
            exit(1);
        }

        // Add the error to the position Z
        if (pos_Z != Prev_pos_Z)
        {
            Actual_pos_Z = pos_Z + GenerateError(pos_Z);
            if (Actual_pos_Z > Z_max)
            {
                Actual_pos_Z = Z_max;
            }
            else if (Actual_pos_Z < Z_min)
            {
                Actual_pos_Z = Z_min;
            }
            Prev_pos_Z = pos_Z;
        }

        // Accessing the pipe coming from motor X and retrieving the position X
        if (fd = open(Pos_x_Fifo, O_RDONLY))
        {
            if (read(fd, &pos_X, sizeof(pos_X)) == -1)
            {
                close(log_fd);
                close(fd);
                perror("Error reading fifo");
                exit(1);
            }

            close(fd);
        }
        else
        {
            close(log_fd);
            close(fd);
            perror("Error opening fifo");
            exit(1);
        }

        // Add the error to the position X
        if (pos_X != Prev_pos_X)
        {
            Actual_pos_X = pos_X + GenerateError(pos_X);
            if (Actual_pos_X > X_max)
            {
                Actual_pos_X = X_max;
            }
            else if (Actual_pos_X < X_min)
            {
                Actual_pos_X = X_min;
            }
            Prev_pos_X = pos_X;
        }

        Pos[0] = Actual_pos_X;
        Pos[1] = Actual_pos_Z;

        SendPos(Actual_Pos_Fifo, Pos);
    }
    close(log_fd);

    return 0;
}
