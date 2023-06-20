// Include libraries
#include "./../include/inspection_utilities.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <math.h>

// Initialization

char log_buffer[100];
int log_fd;

float Pos[2];

struct tm *info;
time_t rawtime;

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

    // Initialization of the End-effector X and Y coordinates: these values with the parameters below need to make sense together
    float end_eff_X, end_eff_Y;
    end_eff_X = 0.0;
    end_eff_Y = 0.0;

    // Initialize the UI
    init_console_ui();

    // Pipe initialization
    int fd;
    char *Actual_Pos_Fifo = "/tmp/Actual_Pos_Fifo";
    mkfifo(Actual_Pos_Fifo, 0666);

    pid_t motor_X_pid = atoi(argv[1]);
    pid_t motor_Z_pid = atoi(argv[2]);

    // Opening the log file and if the file will not open, print an error message

    if ((log_fd = open("log/inspection.log", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
    {
        perror("Error opening command file");
    }

    while (TRUE)
    {

        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Get mouse/resize commands in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI
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

            // Check which button has been pressed...
            if (getmouse(&event) == OK)
            {
                // RESET button pressed
                if (check_button_pressed(rst_button, &event))
                {
                    mvprintw(LINES - 1, 1, "RST button pressed");

                    WriteLog("RESET button pressed.");

                    kill(motor_X_pid, SIGUSR2);
                    kill(motor_Z_pid, SIGUSR2);

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // STOP button pressed
                else if (check_button_pressed(stp_button, &event))
                {
                    mvprintw(LINES - 1, 1, "STP button pressed");

                    WriteLog("STOP button pressed.");

                    kill(motor_X_pid, SIGUSR1);
                    kill(motor_Z_pid, SIGUSR1);

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // Opening the pipes for read
        if ((fd = open(Actual_Pos_Fifo, O_RDONLY)) == -1)
        {
            close(fd);
            perror("Error opening fifo");
        }

        // Read the pipes from World
        if (read(fd, Pos, sizeof(Pos)) == -1)
        {
            close(fd);
            perror("Error reading fifo");
        }

        // closing pipe
        close(fd);

        end_eff_X = Pos[0];
        end_eff_Y = Pos[1];

        // Update the UI
        update_console_ui(&end_eff_X, &end_eff_Y);
        sleep(1);
    }

    // closing log file
    close(log_fd);

    // Terminate
    endwin();
    return 0;
}