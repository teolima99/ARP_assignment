// Include libraries
#include "./../include/processA_utilities.h"
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Initialization
int w = 1600;
int h = 600;
int d = 4;

int r = 30;

// Semaphores
sem_t *first_semaph;
sem_t *second_semaph;

// Counter for multiple photos
int count_bmp = 0;

char Log_Buffer[100];
int FD_log;

// Shared memory matrix
struct SharedMemory
{
    int matrix[1600][600];
};

struct tm *info;
time_t rawtime;

void Save_BMP_File(bmpfile_t *bmp_file)
{
    // Store name and concatenate it with a counter variable for saving multiple photos
    // There is the possibility (sometimes) that it could save 2 photos at once
    char name_file[50];
    snprintf(name_file, 50, "output/image%d.bmp", count_bmp);

    bmp_save(bmp_file, name_file);
    WriteLog("bpm file saved.");
    count_bmp++;
}

// Function to draw (flag_c==True) or to delele (flag_c==False) the blue circle
void Icon_Blue_Circle(int r, int x, int y, bmpfile_t *bmp_file, bool flag_c)
{
    rgb_pixel_t pxl;

    if (flag_c == true)
    {
        pxl.blue = 255;
        pxl.green = 0;
        pxl.red = 0;
        pxl.alpha = 0;
    }
    else
    {
        pxl.blue = 255;
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

// Writelof function to write the log file
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

int main(int argc, char *argv[])
{
    if ((FD_log = open("log/processA.log", O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
    {
        perror("Error opening process A Client log file.");
        exit(1);
    }

    WriteLog("Process A Client started.");

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Here we take port and address as arguments
    int port = atoi(argv[1]);
    char *ServerAdrs = argv[2];

    // Create a socket struct for address and port
    struct sockaddr_in adrs;
    adrs.sin_family = AF_INET;
    adrs.sin_port = htons(port);
    adrs.sin_addr.s_addr = inet_addr(ServerAdrs);

    // Create a socket
    int new_socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (new_socket_client == -1)
    {
        perror("Error creating socket");
        WriteLog("Error creating socket");
    }

    // Here we connect to the server
    int FDcli = connect(new_socket_client, (struct sockaddr *)&adrs, sizeof(adrs)); //
    if (FDcli == -1)
    {
        perror("Error during connection to a server");
        WriteLog("Error during connection to a server");
        close(FDcli);
    }

    // Variables for shared memory
    struct SharedMemory *Shared_mem_pointer;
    key_t Shared_mem_key;
    int Shared_mem_id;

    Shared_mem_key = ftok(".", 'x');

    // Get ID
    Shared_mem_id = shmget(Shared_mem_key, sizeof(struct SharedMemory), IPC_CREAT | 0666);
    if (Shared_mem_id < 0)
    {
        perror("Error getting Shared Memory ID.");
        WriteLog("Error getting Shared Memory ID.");
        exit(1);
    }

    // Attach the shared memory
    Shared_mem_pointer = (struct SharedMemory *)shmat(Shared_mem_id, NULL, 0);
    if ((int)Shared_mem_pointer == -1)
    {
        perror("Error attaching Shared Memory.");
        WriteLog("Error attaching Shared Memory.");
        exit(1);
    }

    // Open semaphores
    first_semaph = sem_open("/sem", O_CREAT, 0666, 1);
    second_semaph = sem_open("/sem1", O_CREAT, 0666, 0);
    if (first_semaph == (void *)-1 || second_semaph == (void *)-1)
    {
        perror("Error opening semaphores.");
        WriteLog("Error opening semaphores.");
        exit(1);
    }

    // Create the bpm file with sizes
    bmpfile_t *bmp_file;
    bmp_file = bmp_create(w, h, d);

    // Infinite loop
    while (TRUE)
    {
        // Coordinates of the circle
        int y = circle.y;
        int x = circle.x;

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
        else if (cmd == KEY_MOUSE)
        { // Else, if user presses print button...
            if (getmouse(&event) == OK)
            {
                if (check_button_pressed(print_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    WriteLog("Print button pressed");

                    // Save bpm image and log
                    Save_BMP_File(bmp_file);

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
        else if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        { // If input is an arrow key, move circle accordingly...

            sem_wait(first_semaph);

            WriteLog("Command received.");

            // Send commands to the server
            if(send(new_socket_client, cmd, 4, 0)==-1){
                perror("Error sending command to the server");
                WriteLog("Error sending command to the server");
                close(FDcli);
                return -1;
            }

            move_circle(cmd);
            draw_circle();

            // Delete the blue circle
            Icon_Blue_Circle(r, x, y, bmp_file, false);

            // Set shared memory to 0
            for (int i = 0; i < w; i++)
            {
                for (int j = 0; j < h; j++)
                {
                    Shared_mem_pointer->matrix[i][j] = 0;
                }
            }

            // Draw the blue circle
            Icon_Blue_Circle(r, circle.x, circle.y, bmp_file, true);

            // Write to the shared memory
            for (int i = 0; i < w; i++)
            {
                for (int j = 0; j < h; j++)
                {
                    // Get the pixel
                    rgb_pixel_t *pxl = bmp_get_pixel(bmp_file, i, j);

                    // Set the shared memory to 1, if the pixel is blue
                    if ((pxl->blue == 255) && (pxl->red == 0) && (pxl->green == 0) && (pxl->alpha == 0))
                    {
                        Shared_mem_pointer->matrix[i][j] = 1;
                    }
                }
            }
            sem_post(second_semaph);
        }
    }

    // Close and unlink everything
    sem_close(first_semaph);
    sem_close(second_semaph);
    sem_unlink("/sem");
    sem_unlink("/sem1");
    bmp_destroy(bmp_file);
    shmdt((void *)Shared_mem_pointer);
    shmctl(Shared_mem_id, IPC_RMID, NULL);

    endwin();

    WriteLog("Process A Client terminated.");
    close(FD_log);

    return 0;
}
