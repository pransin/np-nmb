#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define BUFSIZE 1024

int main()
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    struct msghdr msg;
    struct iovec iov;
    char recvbuf[BUFSIZE];
    char controlbuf[BUFSIZE];
    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = controlbuf;
    msg.msg_name = NULL;
    while (1)
    {
        msg.msg_controllen = sizeof(controlbuf);
        int n = recvmsg(sockfd, &msg, 0);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("recvmsg");
            }
        }
        int hlen1, icmplen;
        struct ip *ip;
        struct icmp *icmp;
        ip = (struct ip *)recvbuf;
        hlen1 = ip->ip_hl << 2;
        if (ip->ip_p != IPPROTO_ICMP) // Not an ICMP message
        {
            continue;
        }
        icmp = (struct icmp *)(recvbuf + hlen1);
        if ((icmplen = n - hlen1) < 8) // malformed packet
        {
            continue;
        }
        if (icmp->icmp_type == ICMP_HOST_UNREACH || icmp->icmp_type == ICMP_NET_UNREACH)
        {
            // Send message to local server with {dest ip address and error}
            struct msghdr error_msg;
        }
    }
}