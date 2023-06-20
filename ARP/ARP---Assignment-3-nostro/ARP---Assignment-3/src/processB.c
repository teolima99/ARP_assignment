// include libraries
#include "./../include/processB_utilities.h"
#include <time.h>
#include <fcntl.h>
#include <bmpfile.h>
#include <semaphore.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

int w = 1600;
int h = 600;
int d = 4;

int r = 30;

// Shared memory matrix
struct SharedMemory
{
    int matrix[1600][600];
};

// Semaphores
sem_t *first_semaph;
sem_t *second_semaph;

char Log_Buffer[100];
int FD_log;

struct tm *info;
time_t rawtime;

// Function to draw (flag_c==True) or to delele (flag_c==False) the blue circle
void Icon_Blue_Circle(int r, int x, int y, bmpfile_t *bmp_file, bool flag_c)
{
    rgb_pixel_t pxl;

    if (flag_c == true)
    {
        pxl.blue = 255; // Blue
        pxl.green = 0;
        pxl.red = 0;
        pxl.alpha = 0;
    }
    else
    {
        pxl.blue = 255; // Blue
        pxl.green = 255;
        pxl.red = 255;
        pxl.alpha = 0;
    }

    for (int i = -r; i <= r; i++)
    {
        for (int j = -r; j <= r; j++)
        {
            // If distance is smaller, point is within the circle
            if (sqrt(i * i + j * j) < r)
            {
                /*
                 * Color the pixel at the specified (x,y) position
                 * with the given pixel values
                 */
                bmp_set_pixel(bmp_file, x * 20 + i, y * 20 + j, pxl);
            }
        }
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
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    if ((FD_log = open("log/processB.log", O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
    {
        perror("Error opening process B log file");
        exit(1);
    }

    WriteLog("Process B started.");

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Variables for shared memory
    struct SharedMemory *Shared_mem_pointer;
    key_t Shared_mem_key;
    int Shared_mem_id;

    Shared_mem_key = ftok(".", 'x');

    Shared_mem_id = shmget(Shared_mem_key, sizeof(struct SharedMemory), IPC_CREAT | 0666);
    if (Shared_mem_id < 0)
    {
        perror("Error getting Shared Memory ID.");
        WriteLog("Error getting Shared Memory ID.");
        exit(1);
    }

    Shared_mem_pointer = (struct SharedMemory *)shmat(Shared_mem_id, NULL, 0);
    if ((int)Shared_mem_pointer == -1)
    {
        perror("Error attaching Shared Memory.");
        WriteLog("Error attaching Shared Memory.");
        exit(1);
    }

    // Open semaphores
    first_semaph = sem_open("/sem", 0);
    second_semaph = sem_open("/sem1", 0);
    if (first_semaph == (void *)-1 || second_semaph == (void *)-1)
    {
        perror("Error opening semaphores.");
        WriteLog("Error opening semaphores.");
        exit(1);
    }

    // Create the bmp file
    bmpfile_t *bmp_file;
    bmp_file = bmp_create(w, h, d);

    int Prev_X;
    int Prev_Y;
    int Center = 0;
    int Crdn_Y[600];
    int Crdn_X[600];

    bool flag;
    int index;

    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
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
        else
        {
            mvaddch(LINES / 2, COLS / 2, '0');
            refresh();

            sem_wait(second_semaph);

            // Get old center coordinates
            if (Crdn_Y[index - 1] != 0 && Crdn_X[index - 1] != 0)
            {
                Prev_X = Crdn_X[index - 1];
                Prev_Y = Crdn_Y[index - 1];
            }

            for (int i = 0; i < h; i++)
            {
                // Reset
                if (Crdn_Y[i] != 0)
                {
                    Crdn_Y[i] = 0;
                }
                else if (Crdn_X[i] != 0)
                {
                    Crdn_X[i] = 0;
                }
            }

            Center = 0;
            index = 0;
            flag = false;

            // Find the circle center
            for (int i = 0; i < w; i++)
            {
                if (flag == true)
                {
                    break;
                }
                for (int j = 0; j < 600; j++)
                {
                    if (Shared_mem_pointer->matrix[i][j] == 1)
                    {
                        Crdn_Y[index] = j;
                        Crdn_X[index] = i;

                        // If the y coordinate is greater than the previous one, break the loop
                        if (Crdn_Y[index] > Crdn_Y[index - 1])
                        {
                            flag = true;
                            break;
                        }
                        index++;
                        break;
                    }
                }
            }
            mvaddch(floor((int)((Crdn_Y[index - 1] + r) / 20)), floor((int)(Crdn_X[index - 1] / 20)), '0');
            refresh();

            sem_post(first_semaph);

            // Delete old blue circle
            Icon_Blue_Circle(r, Prev_X, Prev_Y, bmp_file, false);
            // Draw new blue circle
            Icon_Blue_Circle(r, Crdn_X[index - 1], Crdn_Y[index - 1], bmp_file, true);
        }
    }

    // Close and unlink everything
    sem_close(first_semaph);
    sem_close(second_semaph);
    shmdt((void *)Shared_mem_pointer);
    bmp_destroy(bmp_file);

    endwin();

    WriteLog("Process B terminated.");
    close(FD_log);

    return 0;
}
