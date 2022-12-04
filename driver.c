#define MAX_BUFFER_LENGTH 1024
#include "nmb.c"
#include <stdio.h>

struct msg
{
    long mtype;
    char mtext[MAX_BUFFER_LENGTH];
};

int main(int argc, char *argv[])
{
    // TODO: Remove port part
    if (argc != 2)
    {
        printf("Usage: ./driver <your-port>\n");
        exit(EXIT_FAILURE);
    }

    char ip[16];
    int port;
    struct msg msg;
    int fd = msgget_nmb();
    for (;;)
    {
        printf("Enter destination IP:\n");
        scanf("%s", &ip);
        printf("Enter destination port:\n");
        scanf("%d", port);
        printf("Enter message:\n");
        scanf("%s", &msg.mtext);
    }
}