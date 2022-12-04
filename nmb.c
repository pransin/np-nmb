#include <stdio.h>
#include <sys/msg.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_SOCK_PATH "/usr/tmp/1111"
#define MAX_BUFFER_LENGTH 1024

struct messagetype
{
    long mtype;
    char message[MAX_BUFFER_LENGTH];
};

int msgget_nmb()
{
    struct sockaddr_un client_address;
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    client_address.sun_family = AF_UNIX;
    snprintf(client_address.sun_path, sizeof(client_address.sun_path), "/tmp/ud_ucase_cl.%ld", (long)getpid());
    if (bind(sockfd, (struct sockaddr *)&client_address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// void msgsnd_nmb(struct messagetype *msg)
// {
//     struct sockaddr_un server_address, client_address;
//     int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
//     memset(&client_address, 0, sizeof(struct sockaddr_un));
//     client_address.sun_family = AF_UNIX;
//     snprintf(client_address.sun_path, sizeof(client_address.sun_path), "/tmp/ud_ucase_cl.%ld", (long)getpid());
//     if (bind(sockfd, (struct sockaddr *)&client_address, sizeof(struct sockaddr_un)) == -1)
//     {
//         perror("bind");
//         exit(EXIT_FAILURE);
//     }
//     memset(&server_address, 0, sizeof(struct sockaddr_un));
//     server_address.sun_family = AF_UNIX;
//     strncpy(server_address.sun_path, SERVER_SOCK_PATH, sizeof(server_address.sun_family) - 1);
//     int n = sendto(sockfd, msg, sizeof(*msg), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un));
//     if (n == -1)
//     {
//         perror("sendto");
//     }
//     remove(client_address.sun_path);
// }

void msgsnd_nmb(struct messagetype *msg, int clientsockfd, char *ip, int port)
{
    int ipaddress = inet_pton(AF_INET, ip, NULL);
    msg->mtype = (ipaddress << 16) | port;
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_un));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, SERVER_SOCK_PATH, sizeof(server_address.sun_family) - 1);
    int n = sendto(clientsockfd, msg, sizeof(*msg), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un));
    if (n == -1)
    {
        perror("sendto");
    }
}

struct messagetype msgrcv_nmb(int clientsockfd, int msqid, int port_no)
{
    struct messagetype buf;
    long msgtype = port_no;
    int n = msgrcv(msqid, &buf, sizeof(buf), msgtype, 0);
    if (n)
    {
        return buf;
    }
    n = recvfrom(clientsockfd, &buf, sizeof(buf), 0, NULL, NULL);
    if (n == -1)
    {
        perror("recvfrom");
    }
    return buf;
}