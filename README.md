# CSCI 411
### Spring 2018
#### Professor Marguerite Doman

------------------------------

Objectives
------------------------------
Survey the functions of operating systems and the concepts necessary for the 
design of an operating system

Topics Covered
------------------------------
- Process Control
- Mutual Exclusion
- Deadlock
- Memory Management
- Processor Scheduling
- I/O Management
- File Management

Texts
------------------------------
Modern Operating Systems (4th ed), Tanenbaum, Boss, 2015, ISBN 978-0-13-359762

Compiling Projects
------------------------------
- Change directory into project
- Make a build directory
- Change directory into build directory
- Run `cmake ..`
- Run `make`
- Run built target

**Example:**
```
$ cd simple_shell
$ mkdir cmake-build && cd cmake-build
$ cmake .. && make
.....
[100%] Built target simple_shell
$ ./simple_shell
```
