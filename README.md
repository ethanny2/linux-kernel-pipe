# Linux kernel level pipe
[![Twitter Badge](https://img.shields.io/badge/chat-twitter-blue.svg)](https://twitter.com/ArrayLikeObj)
[![GitHub license](https://img.shields.io/github/license/ethanny2/linux-kernel-pipe)](https://github.com/ethanny2/linux-kernel-pipe)

## Requirements
A Linux operating system and GNU Make to compile and build the C executables.

## Usage
- Run make to build module files and create both producer and consumer executables.
- insmod ./numpipe.ko <bufferSize=N>  Where N=number of slots in the buffer.
- Run as many concurrent producers and cosumers as you want in multiple terminal instances to test



## Concepts and Languages Used
+ **Pipes**: Form of interprocess communication on Linux Operating Systems.
  + Ex: Using a named pipe (writing data in Terminal 1):  ls -l > pipe
+ **Consumer**: A program that is responsible for reading in bytes from a named pipe.
  + This is a program made to read in data from a named pipe.
  + Checks argc to check if # of arguments is equal to 2. You need to pass in the name of the named pipe you created.
  + int fd is the file descriptor used to for your named pipe. If fd= open(argv[1],O_RDONLY) < 0 the pathname could not be successfully turned into a file descriptor and the program exits
  + There is an infinite while loop to continually read in data from the named pipe.
  + Bytes are read from the pipe using the read( int File descriptor, void *yourBuffer , size_t numberOfBytesToReadIn) command.
  + On a successful read() call the number of bytes is the return value (Should be equal to the last parameter). If 0 is returned then the file is done (EOF). If a negative number is returned there was an error. 
+ **Producer**: A program that is responsible for writing data into a named pipe.
  + Everything is identical to the consumer (pass in name of Named pipe ) however the file is opened with (fd = open(argv[1], O_WRONLY) the write only flag.
  + signal(SIGPIPE, SIG_IGN) ignores when signal normally sent when the write
  + bzero(numstr, MAXLEN) fills the buffer you defined with '/0'. It erases any data previously in the buffer (to write new information on other iteration of the while loop)
  + printf(numstr, "%d%d\n", getpid(), count++); Puts the process id (pid) and the a count variable (holds the number of times you wrote using the producer) into a string called numstr
  + num_to_write holds a numerical value of the numstr counter.
  + The named pipe is written to using write(int fileDescriptor, void *yourBuffer,size_t numberOfBytesToWrite); On success, the number of bytes written is returned (zero indicates nothing was written). On error, -1 is returned, and errno is set appropriately.
+ **Semaphore**: A single non-negative integer value which is used to manage concurrent processes across threads.
  
  
This project required 4 different semaphores.
1. One to check if its okay to write (for the write FOPS)
2. One to check if its okay to read (for the read FOPS)
3. One to check if the buffer from the char device is full 
4. One to check if the buffer from the char device is empty.


