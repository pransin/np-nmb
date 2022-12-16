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

### NMB API

- msget_nmb(): It takes port number as a parameter and returns a Unix Domain Datagram socket for the client process
- msgsnd_nmb(): It is used by both client and error process to send a message to its local server (on Unix Domain Socket). It accepts the client socket fd, destination ip and port and message to be sent and its size as parameters and sends the message to the corresponding local server (on Unix Domain Socket). If error process uses this API, it passes NULL and 0 as destination ip and port respectively. It also sets the mtype of the message using destination ip and port number 

### Local server

### Error Process

- 

## Assumptions
