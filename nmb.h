#include <netinet/in.h>
#define SERVER_SOCK_PATH "/tmp/1111"
#define MAX_BUFFER_LENGTH 1024
#define MSG_QUEUE_PATH "."

int __msqid;

int msgget_nmb();
int msgsnd_nmb(int clientsockfd, char *ip, int port, void *msg, size_t msgsz);
int msgrcv_nmb(int clientsockfd, void *msg, int port_no);