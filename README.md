# ARP Assignments

*Advanced and Robot Programming* (ARP) assignments

## Project made by:

Nicholas Attolino S5656048

Teodoro Lima S5452613

## How to test:

### ncurses installation
To install the ncurses library, simply open a terminal and type the following command:
```console
sudo apt-get install libncurses-dev
```

### libbitmap library installation
Download this repository: https://github.com/draekko/libbitmap.

Navigate to the root directory of the folder in the console and run the following commands.

For making the configuration
```console
./configure 
```

For compiling
```console
make
```

For installing
```console
sudo make install
```

After the installation, check if the library has been installed navigating to `/usr/local/lib`, where you are supposed to find the `libbmp.so` file.

The last step is to open the `.bashrc` in the root folder of your own system:
```console
gedit .bashrc
```
and add `export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"`.
Now reload your `.bashrc`:
```console
source .bashrc
```


### Compiling and running the code
Make all the .c files inside `src/` executable, running:
```console
chmod +x *.c
```
Move with the terminal to the folder where there are `compile.sh` and `run.sh` files. Run:
```console
./compile.sh
```
then:
```console
./run.sh
```

## Explanation I assignment

The Project aims to control a hoist with two degrees of freedom by creating six different processes. The hoist is equipped with two motors: one for movement along the x-axis and the other for movement along the z-axis. When the program is running, the user will see two main consoles. The first console is the command console, where user inputs are taken, and the second console is the inspection console, which displays the hoist movement.

The command console, the first window, contains six buttons. The first three buttons are: 'Vx-' to decrease the velocity on the x-axis, 'S' to stop the hoist on the x-axis, and 'Vx+' to increase the velocity on the x-axis. The other three buttons are: 'Vz-' to decrease the velocity on the z-axis, 'S' to stop the hoist on the z-axis, and 'Vz+' to increase the velocity on the z-axis. The user interacts with these buttons using the mouse to provide input. The velocity steps have two levels: clicking 'Vx+' sets the velocity on the x-axis to +1, initiating hoist movement, and clicking it again increases the velocity to +2, resulting in faster movement. The maximum speed for all axes and directions is set to 2. The command console contains two pipes that send inputs to the motor x and motor z processes.

The motor x and motor z processes are represented by the second and third windows, respectively. These windows read the pipes coming from the command console to receive inputs. Inside these processes, velocity and position are calculated every second. Additionally, a pipe is created for each motor to send position values to the world console.

The world console represents the fourth process of the project. It receives pipes from the motor x and motor z processes and reads data about position. The purpose of the world console is to simulate real-life behavior by introducing friction, wind, and other possible environmental effects. To achieve this, random values are calculated for each received data and added. After this process, a pipe sends the new "real position" to the inspection console.

The inspection console serves the purpose of displaying the hoist movement to the user. It also includes two important buttons that work with signals:
- The 'R' button (Reset) moves the hoist back to the initial position on both the x-axis and the z-axis at a speed of -5 when pressed.
- The 'S' button (Stop) immediately halts all processes when pressed.

The watchdog process, the sixth process, is responsible for killing all processes, including itself, if no button is pressed for 60 seconds.

## Explanation II assignment

The objective of the project is to establish separate processes that communicate with each other using a shared memory. 

The `/src` folder contains three files: processA, processB, and master. 

When the program is executed, two console windows will appear: one for process A and another for process B. The user can manipulate the position of the pivot (represented by a green cross) within the process A console by using the arrow keys on the keyboard. Depending on the user's input, the object will move accordingly within the console. Additionally, there is a blue button (P button) for capturing and saving an image (multiple numbered images can be saved) as a `.bmp` file in the `/output` folder. The image depicts the position of the pivot in process A, represented by a blue circle. 

While these processes are running, the user can observe the object's movements as continuously drawn zeroes (0s) in the process B console. 

Process A and process B are interconnected through a shared memory and are synchronized using two semaphores. 

The master file is responsible for creating child processes and initializing the consoles for process A and process B. 

Each process maintains a log of its status, which is stored in a `.log` file within the `/log` folder.

## Explanation III assignment

Within the `/src` folder, there are five files: processA, processAclient, processAserver, processB, and master.

When the program is executed, the user is presented with three selectable options: Normal, Server and Client. The user can choose one of these options by entering 1, 2, or 3, respectively.

In Normal, we have the same configuration as in the second assignment;

If the user selects Server, they will be prompted to enter a port number to establish a connection with a client running on another machine within the network. Commands to move the pivot will be received from the client.

Alternatively, if the user chooses Client, they will need to provide a port number and an address to establish a connection with a server running on another machine within the network. By manipulating the pivot in the local console, commands will be sent simultaneously to the server to move the pivot.

The master file is responsible for creating child processes and initializing the consoles for processA, processAserver, processAclient, and processB. If the user closes the two created consoles, they will have the option to select a new option, switch between options, and connect to a different machine.

Each process maintains its status and logs it in a `.log` file within the `/log` folder.

## Troubleshooting

If you encounter any unusual behavior after launching the application, such as buttons not appearing within the GUI or graphical assets being misaligned, try resizing the terminal window as a potential solution for resolving the bug.
