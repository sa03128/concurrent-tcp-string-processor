# Concurrent TCP String Processor

This project implements a TCP client-server application in C.

The client sends a string and a requested operation to the server. The server processes the request and returns the result to the client.

## Project Idea

The client provides a menu that allows the user to choose one of several string operations.

The server receives the request, identifies the operation code, processes the string, and sends a response back to the client.

The server can handle multiple clients by creating a child process for each connected client using `fork()`.

## Technologies Used

- C
- TCP sockets
- Linux socket programming
- Process-based concurrency
- fork system call
- Signals
- Client-server communication

## Files

```text
src/client.c
src/server.c
```

## Features

- TCP client-server communication
- Menu-based client interface
- Converts a string to uppercase
- Counts the number of characters in a string
- Counts the frequency of a selected character
- Allows the user to enter a new string
- Uses operation codes to define client requests
- Handles multiple clients using child processes
- Prevents zombie processes using signal handling

## How to Compile

```bash
gcc src/server.c -o server
gcc src/client.c -o client
```

## How to Run

Terminal 1:

```bash
./server 5000
```

Terminal 2:

```bash
./client 127.0.0.1 5000
```

The port number must be greater than 1024 and less than 65535.

## Example Operations

```text
1. Change the string to capital letters
2. Count number of characters in the string
3. Count the frequency of some character
4. Enter another string
```

## What I Learned

- How to build a TCP client-server application in C
- How to send operation codes between a client and a server
- How to process different requests on the server side
- How to handle multiple clients using fork
- How to manage child processes and avoid zombie processes
