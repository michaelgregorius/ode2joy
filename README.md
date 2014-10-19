# ode2joy


----------


## Introduction
*ode2joy* is intended to become an application that translates joystick input to MIDI events.

You might for example map button presses on your joystick to the following MIDI events:

* *Note on* and *note off* events, e.g. to trigger drum samples.
* *Program change* events to switch instruments.

The input of the axis' might be mapped to:

* Pitch bends
* Controllers

## Current state
The current implementation only maps joystick button events to hardcoded *note on* and *note off* MIDI events.

An actual mapping and a configuration of the mapping via the graphical user interface is not implemented yet.

## Dependencies
Currently *ode2joy* is dependent on the following libraries:

* [Qt 4.8](http://qt-project.org/) (required)
* ALSA (optional)

## Getting involved in development
Here are some tips if you want to get involved in the development of *ode2joy*.

### Developing under Ubuntu 14.04 LTS
Actually *ode2joy* is developed on an [Arch Linux](http://archlinux.org/) system. However, as [Ubuntu](http://www.ubuntu.com) is the more widely used distribution here are some pointers on how to get a develop environment running on a fresh install of Ubuntu 14.04 LTS.

#### Installation of Ubuntu 14.04 LTS
If you do not have Ubuntu 14.04 LTS installed you will obviously need to install it first. In case you install it in VirtualBox you may want to install the guest additions as well. Issue the following command from within the virtual machine:
> sudo apt-get install virtualbox-guest-x11

#### Installation of the needed development tools
The following development components are needed:

* git
* g++
* CMake
* Qt4
* ALSA

You can install all these components at once with the following command:
> sudo apt-get install git g++ cmake qt4-default libasound2-dev

#### Cloning of the repository
Use the following command to clone the git repository:
> git clone https://github.com/michaelgregorius/ode2joy.git

#### Building the application from source
Now we should be ready to compile everything with make. For the sake of simplicity we will assume that the repository was cloned into the home directory, i.e. there is a directory *~/ode2joy*. To compile the application follow these steps:

1. Change the working directory to your home directory:
> cd ~

2. Create a build directory:
> mkdir ode2joy-build

3. Change into the build directory:
> cd ode2joy-build

4. Run CMake (this step will create the *make* file):
> cmake ../ode2joy/src

5. Build the application:
> make

6. Start the application:
> ./ode2joy 

#### Developing with Qt Creator
* Install Qt Creator:
> sudo apt-get install qtcreator

* Start Qt Creator, e.g. from a terminal.
> qtcreator &

* From the "Welcome" screen select "Open Project".
* In the file dialog navigate to ~/ode2joy/src and select the file CMakeLists.txt.
* CMake is now executed automatically.
* Try to build the application, e.g. by pressing CTRL+B.

### Known caveats
If you install Ubuntu 14.04 LTS in VirtualBox the mouse will be reported as device */dev/input/js0*. Currently this device is hard coded as the joystick in the kernel joystick driver found at *Drivers/Joystick/JoystickDriverLinux.cpp*. If you want to develop in VirtualBox you may need to change the device in this file to */dev/input/js1* or something similar.

## TODOs

* Implement data structures for mappings
* Implement GUI to define the mappings
* Highlight joystick events in GUI
* Implement joystick and MIDI drivers for Windows

