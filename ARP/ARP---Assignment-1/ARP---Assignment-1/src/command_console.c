// Include libraries
#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>

// Initialization
char log_buffer[100];
int log_fd;

struct tm *info;
time_t rawtime;

// SendVel function to send the velocity to the motor X
void SendVel(char *fifo_path, int Velocity)
{
    int fd;
    if ((fd = open(fifo_path, O_WRONLY)) == -1)
    {
        close(log_fd);
        perror("Error.");
        exit(1);
    }
    if (write(fd, &Velocity, sizeof(Velocity)) == -1)
    {
        close(log_fd);
        close(fd);
        perror("Error in writing function");
        exit(1);
    }
    close(fd);
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

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Pipes initialization
    int fd;
    char *Vel_z_Fifo = "/tmp/Vel_z_Fifo";
    mkfifo(Vel_z_Fifo, 0666);
    char *Vel_x_Fifo = "/tmp/Vel_x_Fifo";
    mkfifo(Vel_x_Fifo, 0666);

    // Open the log file and if the file will not be opened, print an error message
    if ((log_fd = open("log/command.log", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
    {
        perror("Error opening command file");
        exit(1);
    }

    while (TRUE)
    {

        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);

        fflush(stdout);

        // Get mouse/resize commands in non-blocking mode
        int cmd = getch();

        // If user resizes the screen, re-draw UI
        if (cmd == KEY_RESIZE)
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if (cmd == KEY_MOUSE)
        {

            // Checking which button pressed
            if (getmouse(&event) == OK)
            {
                // Vx++ button pressed -> Velocity data sending and writing to the log file
                if (check_button_pressed(vx_incr_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");

                    SendVel(Vel_x_Fifo, 1);
                    WriteLog("Vx++ button pressed.");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
                // Vx-- button pressed -> Velocity data sending and writing to the log file
                else if (check_button_pressed(vx_decr_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");

                    SendVel(Vel_x_Fifo, 0);
                    WriteLog("Vx-- button pressed.");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz++ button pressed - Velocity data sending and writing to the log file
                else if (check_button_pressed(vz_decr_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");

                    SendVel(Vel_z_Fifo, 0);
                    WriteLog("Vz++ button pressed.");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz-- button pressed - Velocity data sending and writing to the log file
                else if (check_button_pressed(vz_incr_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");

                    SendVel(Vel_z_Fifo, 1);
                    WriteLog("Vz-- button pressed.");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx stop button pressed - Velocity data sending and writing to the log file
                else if (check_button_pressed(vx_stp_button, &event))
                {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");

                    SendVel(Vel_x_Fifo, 2);
                    WriteLog("Vx stop button pressed.");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz stop button pressed - Velocity data sending and writing to the log file
                else if (check_button_pressed(vz_stp_button, &event))
                {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");

                    SendVel(Vel_z_Fifo, 2);
                    WriteLog("Vz stop button pressed.");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
        refresh();
    }

    // closing the log file
    close(log_fd);

    endwin();
    return 0;
}
