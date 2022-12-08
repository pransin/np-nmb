#define SERVER_SOCK_PATH "/usr/tmp/1111"
#define MAX_BUFFER_LENGTH 1024
#define MSG_QUEUE_PATH "."

int __msqid;
struct messagetype
{
    long mtype;
    char message[MAX_BUFFER_LENGTH];
};

int msgget_nmb();
void msgsnd_nmb(void *msg, int clientsockfd, char *ip, int port);
void *msgrcv_nmb(int clientsockfd, void *msg, int port_no);