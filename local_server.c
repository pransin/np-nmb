#define SUN_PATH "/usr/tmp/1111"
#define MSG_QUEUE_PATH "."
#define MAX_BUFFER_LENGTH 1024
#define MULTICAST_PORT 1112
#define MULTICAST_GROUP "239.0.0.1"

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/un.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>

struct msg
{
    long mtype;
    char mtext[MAX_BUFFER_LENGTH];
};

void error_exit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int create_mq()
{
    key_t key = ftok(MSG_QUEUE_PATH, 'n');
    if (key == -1)
        error_exit("ftok");

    int msqid;
    if ((msqid = msgget(key, IPC_CREAT | 0644)) == -1)
        error_exit("msgget");

    return msqid;
}

int create_multi_receiver()
{
    // Send is also through this socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in in_servaddr;
    struct ip_mreq mreq;
    // Bind UDP server
    bzero(&in_servaddr, sizeof(in_servaddr));
    in_servaddr.sin_family = AF_INET;
    in_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    in_servaddr.sin_port = htons(MULTICAST_PORT);
    if (bind(fd, (struct sockaddr *)&in_servaddr, sizeof(in_servaddr)) == -1)
        error_exit("bind error");

    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        error_exit("setsockopt");

    int opt = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt)) < 0)
        error_exit("setsockfd");

    return fd;
}

struct msg *recv_multi_msg(int udpfd)
{
    struct msg nmb_msg;   /* space to msg into */
    struct iovec vector;  /* file name from the child */
    struct msghdr msg;    /* full message */
    struct cmsghdr *cmsg; /* control message with the fd */
    int fd;

    /* set up the iovec for the file name */
    vector.iov_base = &nmb_msg;
    vector.iov_len = sizeof(nmb_msg);

    /* the message we're expecting to receive */

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vector;
    msg.msg_iovlen = 1;

    /* overprovisioning buffer for now */
    char cmbuf[128];
    msg.msg_control = cmbuf;
    msg.msg_controllen = sizeof(cmbuf);

    if (recvmsg(fd, &msg, 0) == -1)
        return NULL;
    // TODO: Test this part, very high chance of error
    for ( // iterate through all the control headers
        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg != NULL;
        cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        // ignore the control headers that don't match what we want
        if (cmsg->cmsg_level != IPPROTO_IP ||
            cmsg->cmsg_type != IP_PKTINFO)
        {
            continue;
        }
        struct in_pktinfo pi;
        memcpy(&pi, CMSG_DATA(cmsg), sizeof(pi));
        printf("%s", inet_ntoa(pi.ipi_addr));
        int ip = (nmb_msg.mtype) >> 16;
        if (pi.ipi_addr.s_addr != ip)
            return NULL;

        break;
    }
    /* grab the file descriptor from the control structure */
    struct msg *mymsg = malloc(sizeof(struct msg));
    memcpy(mymsg, &nmb_msg, sizeof(struct msg));

    return mymsg;
}

void process_message(struct msg *msg, int udsfd, int msqid)
{
    if (!msg)
        return;

    struct sockaddr_un cliaddr;
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sun_family = AF_UNIX;
    unsigned int port = msg->mtype & 0xffff;
    snprintf(cliaddr.sun_path, sizeof(cliaddr.sun_path), "/tmp/ud_ucase_cl.%u", port);
    if (sendto(udsfd, msg, sizeof(*msg), 0, (struct sockaddr *)&cliaddr, sizeof(struct sockaddr_un)) == -1)
    {
        msgsnd(msqid, msg, sizeof(*msg), 0);
    }
}

void send_multi_msg(int udpfd, int udsfd)
{
    struct msg msg;
    if (recv(udsfd, &msg, sizeof(msg), 0) > 0)
    {
        struct sockaddr_in addr;
        int alen;
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
        addr.sin_port = htons(MULTICAST_PORT);
        alen = sizeof(addr);
        sendto(udpfd, &msg, sizeof(msg), 0, (struct sockaddr *)&addr, alen);
    }
}

int main()
{
    int udsfd, udpfd;
    int maxfd;
    int msqid;
    socklen_t clilen;
    fd_set rset, allset;
    struct sockaddr_un servaddr, cliaddr;
    struct sockaddr_in in_servaddr, in_cliaddr;
    struct msg msg;

    msqid = create_mq();

    // Create sockets
    udsfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    udpfd = create_multi_receiver();
    maxfd = (udpfd > udsfd) ? udpfd : udsfd;

    // Bind unix domain server
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, SUN_PATH);
    if (bind(udsfd, (struct sockaddr *)&servaddr, SUN_LEN(&servaddr)) == -1)
        error_exit("bind error");

    printf("Listening on: %s", servaddr.sun_path);

    FD_ZERO(&allset);
    FD_SET(udpfd, &allset);
    FD_SET(udsfd, &allset);
    for (;;)
    {
        clilen = sizeof(cliaddr);
        rset = allset;
        select(maxfd + 1, &rset, NULL, NULL, NULL);
        int fd;
        if (FD_ISSET(udsfd, &rset))
        {
            send_multi_msg(udpfd, udsfd);
        }
        if (FD_ISSET(udpfd, &rset))
        {
            struct msg *msg = recv_multi_msg(udpfd);
            process_message(msg, udsfd, msqid);
        }
    }
}