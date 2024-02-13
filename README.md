# About

This program reports on different metrics and information of the system it is run on. Similar to a Task Manager for Windows or <code>btop</code> for Linux.

This program is meant to be run on a **Linux operating system** (WSL on Windows works as well), as it makes use of Linux system files such as <code>proc/cpuinfo</code> and more.

## Example

![Screenshot of system monitoring tool](/program_example.png "Program screenshot")

# Installation

1. Clone the repo
  ```sh
  git clone https://github.com/andre-fong/System-Monitoring-Tool.git
  ```
2. Compile
  ```sh
  gcc system_monitor.c
  ```
3. Run
  ```sh
  ./a.out
  ```

# How to Run

When running, the following command line arguments are accepted:

* <code>--system</code>
  * Only generate system-related information
* <code>--user</code>
  * Only generate user-related information
* <code>--graphics</code>
  * Include graphical output for CPU and memory usage
* <code>--sequential</code>
  * Generate output sequentially (useful for redirecting output to a file)
  * E.g., <code>./a.out --sequential > output.txt</code>
* <code>--samples=N</code>
  * Program takes N samples (default: 10)
  * Available as positional argument
* <code>--tdelay=T</code>
  * Delay T seconds in between each data sample (default: 1)
  * Available as positional argument

## Example Usage

```sh
./a.out --graphics --sequential 3 5 > output.txt
```

Run the system monitoring tool with **graphics** and output data **sequentially**, with **3 samples** each **5 seconds apart**. 
**Redirect the output** of the program to <code>output.txt</code>.

# Implementation

To get system data from Linux in C, I included a collection of Linux libraries and read from specific system files. Notable mentions include:

* `sys/resource.h` (getrusage())
  * This was important in seeing how much memory the calling process (our system monitoring program) was using.
* `sys/utsname.h` (uname())
  * This was important in getting system information like the system name, machine name, version, release, and architecture.
* `sys/sysinfo.h` (sysinfo())
  * This was important in seeing total uptime since last reboot. 
  * It was also important in reading memory usage throughout the system.
* `utmp.h`
  * This file defined the utmp struct, which was important in parsing through the system file “/var/run/utmp”, where data on users that use the system is located.
* `/proc/cpuinfo`
  * This system file allowed me to get the number of cpu cores in the system.
* `/proc/stat`
  * This system file allowed me to calculate the cpu usage by finding the delta of the cpu idle and busy times over a period of 1 second.
