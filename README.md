# np-nmb
A network message bus implemented for the course IS F462 Network Programming at BITS Pilani

## Run
```
    gcc -o server server.c
    
    ./server

    gcc -o driver driver.c nmb.c
    
    ./driver <port number>
```

## Design Features

The Network Message Bus (NMB) has following characteristics

- The message from process A on OS 1 to process B on OS 2 is routed as follows: Process A &rarr; Unix Domain Datagram Socket of OS 1 &rarr; UDP Socket of OS 2 (via multicast) &rarr; Unix Domain Datagram Socket / Message queue of OS 2 &rarr; Process B
- 


### NMB API

- msget_nmb(): It takes port number as a parameter and returns a Unix Domain Datagram socket for the client process
- msgsnd_nmb(): It is used by both client and error process to send a message to its local server (on Unix Domain Socket). It accepts the client socket fd, destination ip and port and message to be sent and its size as parameters and sends the message to the corresponding local server (on Unix Domain Socket). If error process uses this API, it passes NULL and 0 as destination ip and port respectively. It also sets the mtype of the message using destination ip and port number
- msgrcv_nmb(): It tries to fetch messages from message queue (if message queue is non empty). If the message queue is empty, it fetches the message from its socket (which is passed as parameter)

### Local server

### Error Process

- 

## Assumptions
