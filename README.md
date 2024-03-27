# THIS IS OUTDATED

# Directory Structure:

## boot
This directory contains all file pertinent to initial setup of the commissioner.

## memory
This directory contains the memory allocator and any relevant configuration files.

## process
This directory contains structures and algorithms for process creation, management, and scheduling.

## stdlib
This directory contains functions and programs which come with the OS to make programming and management easier. This includes abstractions for GPIO, DIO, serial, terminal interface, and any other programming interface functions.

# Features:

## Commissioner:
The commissioner is responsible for keeping track of all kernel agents. This includes the process scheduler/manager, filesystem manager, memory manager and any modules/programs relevant to lower level input and output like an FPGA programmer or interface. The commissioner is able to dynamically reset the operating system or some subprocesses. This means the commissioner is a persistent process which has a special process tag which gives it priority higher than kernal-space programs. This is so it can recieve an interrupt and replace a kernel process or reset the whole kernel without rebooting.

The purpose and advantage of the commissioner is to allow prototyping of new task schedulers and memory managers. This means that any process which should be slotted in place of a kernel program must have the same interface. This is outlined in each kernel programs header file. The commissioner has a panic mode which can be manually entered or will automatically enter when an interface function is not available or triggers a panic failure. This means that panic failures should intentionally be programmed into new kernel processes so that the commissioner can return to its previous stable state. The commissioner keeps a pointer to valid kernel processes with flags that indicate the most recently used stable one.

## Governor:
The governor allows implicit multithreading and allows all threads to be governed by a host process.

## Virtual Machine:
The virtual machine is used to test kernel processes before sending them to the commissioner.

## Process Scheduler:

## Memory Management
A slab allocator is implemented as well as functions like malloc and free for user programs.

## File System:
FAT32 is the filesystem.

## Terminal:

# Programming Interface:

# Standard Library:
Several key libraries are provided for user programs. A full system call interface is included here. The standard libraries file are found in the stdlib directory. Included here are things like containers and string functions.