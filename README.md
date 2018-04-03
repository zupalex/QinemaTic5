# QinemaTic5: a kinematics calculator based on Qt5

It uses QCustomPlot from Emanuel Eichhammer to plot the graphs (http://www.qcustomplot.com/)

Runs on unix system only.
requires cmake and Qt5 development libraries (installed through your linux distrib package manager)

**Example for Ubuntu:**
sudo apt-get install qtbase5-dev

# Installation steps:

**create a build directory wherever you want:**  
mkdir build  
cd build

**build using cmake:**  
cmake build path/to/source

**compile:**  
make install

**execute:**  
cd path/to/source/exec  
./Qinematic5
