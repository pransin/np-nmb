#include <stdio.h>
#include <sys/msg.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "errno.h"
#include "nmb.h"

struct ip_msg
{
    long mtype;
    char mtext[MAX_BUFFER_LENGTH];
    int ip;
};

void error_exit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int get_mq()
{
    key_t key = ftok(MSG_QUEUE_PATH, 'n');
    if (key == -1)
        error_exit("ftok");

    int msqid;
    if ((msqid = msgget(key, 0644)) == -1)
        error_exit("msgget");

    return msqid;
}

int msgget_nmb(short port)
{
    __port = port;
    struct sockaddr_un client_address;
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    client_address.sun_family = AF_UNIX;
    snprintf(client_address.sun_path, sizeof(client_address.sun_path), "/tmp/ud_ucase_cl.%d", __port);
    unlink(client_address.sun_path);
    if (bind(sockfd, (struct sockaddr *)&client_address, sizeof(struct sockaddr_un)) == -1)
        error_exit("bind");

    __msqid = get_mq();
    return sockfd;
}

int msgsnd_nmb(int clientsockfd, char *ip, short port, void *msg, size_t msgsz)
{
    in_addr_t ipaddress;
    if (inet_pton(AF_INET, ip, &ipaddress) != 1)
        return -1;
    long mtype = ((long)ipaddress << 16) | port;
    *(long *)msg = mtype;
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_un));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, SERVER_SOCK_PATH, sizeof(server_address.sun_path) - 1);
    int n = sendto(clientsockfd, msg, msgsz + sizeof(long), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un));
    if (n == -1)
    {
        perror("sendto");
    }
}

int msgrcv_nmb(int clientsockfd, void *msg, size_t msgsz)
{
    long msgtype = __port;
    // Try reading from message queue
    struct ip_msg ipmsg;
    int n = msgrcv(__msqid, &ipmsg, sizeof(ipmsg), msgtype, IPC_NOWAIT);
    if (n != -1)
    {
        *(long *)msg = ((long)ipmsg.ip << 16) | ipmsg.mtype;
        memcpy((char *)msg + sizeof(long), &ipmsg.mtext, msgsz);
        return n;
    }

    // Read from socket if msg queue is empty
    n = recvfrom(clientsockfd, msg, msgsz, 0, NULL, NULL);
    return n;
}