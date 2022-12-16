#define MAX_BUFFER_LENGTH 1024
#include "nmb.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

struct msg
{
    long mtype;
    char mtext[MAX_BUFFER_LENGTH];
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./driver <your-port>\n");
        exit(EXIT_FAILURE);
    }
    char ip[16];
    int port;
    struct msg msg;
    int fd = msgget_nmb(atoi(argv[1]));
    char yesno;
    for (;;)
    {
        printf("Do you want to send?\n");
        scanf(" %c", &yesno);
        if (yesno == 'y' || yesno == 'Y')
        {
            printf("Enter destination IP:\n");
            scanf(" %s", ip);
            printf("Enter destination port:\n");
            scanf("%d", &port);
            printf("Enter message:\n");
            scanf(" %[^\n]s", msg.mtext);
            msgsnd_nmb(fd, ip, port, &msg, sizeof(msg));
        }
        else
        {
            printf("Waiting for message.....\n");
            msgrcv_nmb(fd, &msg, sizeof(msg) - sizeof(msg.mtype));
            printf("%s\n", msg.mtext);
        }
    }
}