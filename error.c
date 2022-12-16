#define PORT 0 // 0 for error messages

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "nmb.h"

struct error_msg
{
    long mtype;
    char dest_ip[INET_ADDRSTRLEN + 1];
    char error_message[30];
};

int main(int argc, char **argv)
{
    // Create a raw socket
    int rawfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (rawfd < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    // Create a unix domain socket
    int udsfd = msgget_nmb(PORT);
    int maxfd = (rawfd > udsfd) ? rawfd : udsfd;
    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(rawfd, &allset);
    FD_SET(udsfd, &allset);

    printf("Listening to ICMP Messages\n");
    // Receive a packet
    char buf[MAX_BUFFER_LENGTH];
    while (1)
    {
        rset = allset;
        if (select(maxfd + 1, &rset, NULL, NULL, NULL) == -1)
            continue;
        struct error_msg err;
        printf("ICMP Received\n");
        if (FD_ISSET(udsfd, &rset))
        {
            msgrcv_nmb(udsfd, &err, sizeof(err));
            printf("%s: %s\n", err.error_message, err.dest_ip);
        }

        if (FD_ISSET(rawfd, &rset))
        {
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            ssize_t bytes_received = recvfrom(rawfd, buf, sizeof(buf), 0,
                                              (struct sockaddr *)&addr, &addr_len);

            if (bytes_received <= 0)
            {
                continue;
            }
            // Parse the packet
            struct ip *ip = (struct ip *)buf;
            int header_len = ip->ip_hl << 2;

            // Extract ICMP part from the buffer
            struct icmp *icmp = (struct icmp *)(buf + header_len);
            int icmp_len = bytes_received - header_len;
            if (icmp_len < 8 + 20) // malformed packet
                continue;

            // Send message to local server with {dest ip address and error}
            if ((icmp->icmp_type == ICMP_DEST_UNREACH) &&
                (icmp->icmp_code == ICMP_HOST_UNREACH || icmp->icmp_code == ICMP_NET_UNREACH))
            {
                // Create error message
                err.mtype = PORT;
                if (icmp->icmp_code == ICMP_HOST_UNREACH)
                    strcpy(err.error_message, "ICMP Host Unreachable");
                else
                    strcpy(err.error_message, "ICMP Network Unreachable");

                struct ip *innerip = &(icmp->icmp_dun).id_ip.idi_ip;
                inet_ntop(AF_INET, &((innerip->ip_dst).s_addr), err.dest_ip, sizeof(err.dest_ip));
                err.dest_ip[INET_ADDRSTRLEN] = '\0';
                msgsnd_nmb(udsfd, NULL, 0, &err, sizeof(err));
                printf("%s: %s\n", err.error_message, err.dest_ip);
            }
        }
    }
    return 0;
}
