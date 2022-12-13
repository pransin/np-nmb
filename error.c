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
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Receive a packet
    char buf[MAX_BUFFER_LENGTH];
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        ssize_t bytes_received = recvfrom(sock, buf, sizeof(buf), 0,
                                          (struct sockaddr *)&addr, &addr_len);
        if (bytes_received < 0)
        {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        if (!bytes_received)
        {
            continue;
        }
        // Parse the packet
        struct ip *ip;
        ip = (struct ip *)buf;
        if (ip->ip_p != IPPROTO_ICMP) // Not an ICMP Message
            continue;
        int header_len = ip->ip_hl >> 2;
        struct icmp *icmp = (struct icmp *)(buf + header_len);
        int icmp_len = bytes_received - header_len;
        if (icmp_len < 8) // malformed packet
        {
            continue;
        }
        if (icmp->icmp_type == ICMP_HOST_UNREACH)
        {
            // Send message to local server with {dest ip address and error}
            struct error_msg err;
            err.mtype = 12345;
            inet_ntop(AF_INET, &((ip->ip_dst).s_addr), err.dest_ip, sizeof(err.dest_ip));
            strcpy(err.error_message, "ICMP Host Unreachable");
            int sockfd = msgget_nmb();
            msgsnd_nmb(sockfd, "127.0.0.1", 35467, &err, sizeof(err));
            printf("%s\n%s\n", err.dest_ip, err.error_message);
        }
        else if (icmp->icmp_type == ICMP_NET_UNREACH)
        {
            // Send message to local server with {dest ip address and error}
            struct error_msg err;
            err.mtype = 12345;
            inet_ntop(AF_INET, &((ip->ip_dst).s_addr), err.dest_ip, sizeof(err.dest_ip));
            strcpy(err.error_message, "ICMP Network Unreachable");
            int sockfd = msgget_nmb();
            msgsnd_nmb(sockfd, "127.0.0.1", 35467, &err, sizeof(err));
            printf("%s\n%s\n", err.dest_ip, err.error_message);
        }
        // char destination_ip[INET_ADDRSTRLEN + 1];
        // inet_ntop(AF_INET, &((ip->ip_dst).s_addr), destination_ip, sizeof(destination_ip));
        // printf("ICMP Message type: %d\nDestination IP: %s\n", icmp->icmp_type, destination_ip);
    }
    return 0;
}
