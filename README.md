# np-nmb
A network message bus implemented for the course IS F462 Network Programming at BITS Pilani

## Run
```
    gcc -o server server.c
    
    ./server

    gcc -o driver driver.c nmb.c
    
    ./driver <port number>
    
    gcc -o error error.c nmb.c
    
    ./error
```

## Design Features

The Network Message Bus (NMB) has following characteristics (in brief)

- The message from process A on OS 1 to process B on OS 2 is routed as follows: Process A &rarr; Unix Domain Datagram Socket of OS 1 &rarr; UDP Socket of OS 1 &rarr; UDP Socket of OS 2 (via multicast) &rarr; Unix Domain Datagram Socket / Message queue of OS 2 &rarr; Process B
- Each OS (local server) has a UDP socket (for receiving multicast messages), a Unix Domain Datagram Socket (for communication with its own processes), a message queue (which stores messages for processes which are down) and an error process
- During multicast, if the destination IP Address provided is loop back address, then that message isn't multicasted
- When a receiving process is down, the local server buffers its messages in message queue and the process later reads those messages from the queue itself. Otherwise the message is directly sent to the Unix Domain Datagram Socket of the process

### NMB API

- msget_nmb(): It takes port number as a parameter and returns a Unix Domain Datagram socket for the client process
- msgsnd_nmb(): It is used by both client and error process to send a message to its local server (on Unix Domain Socket). It accepts the client socket fd, destination ip and port and message to be sent and its size as parameters and sends the message to the corresponding local server (on Unix Domain Socket). If error process uses this API, it passes NULL and 0 as destination ip and port respectively. It also sets the mtype of the message using destination ip and port number
- msgrcv_nmb(): It tries to fetch messages from message queue (if message queue is non empty). If the message queue is empty, it fetches the message from its socket (which is passed as a parameter)

### Local server

- Each local server has a UDP socket and a Unix Domain Datagram Socket. UDP Socket is used for receiving multicasted messages and Unix Domain Datagram Socket is used for relaying messages to its processes
- The local server multicasts the messages which are sent by its processes and thse messages are received by other servers through UDP socket
- When a local server receives a multicasted message at its UDP socket, it calculates the port of the destination client by calculating its port number from the mtype of the message. If that client process is running, this message is directly sent to its UDS. Else the message is buffered in the message queue

### Error Process

- 

## Assumptions

## Screenshots

### Example of communication between two processes on different systems

![Process 1](./screenshots/process1.png?raw=true)
![Process 2](./screenshots/process2.png?raw=true)
