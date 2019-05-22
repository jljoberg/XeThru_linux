
#ifndef __arm__
    #include "matplotlibcpp.h"
#endif
// udp includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <string.h>
#include <csignal>
#include <unistd.h>

#if defined __arm__ || defined NO_PLOTTING
    #error "plot_XeThru.cpp: Why would you plot without GUI?"
    #define plot(...)
    #define pause(...)
    #define clf(...)
#else
    using namespace matplotlibcpp;
#endif

#define MAX_UDP_LEN 5000
#define UDP_BC_PORT 13001
static int udp_sock;
static struct sockaddr_in addr_bc;

void error(const char* msg)
{
    perror(msg);
    exit(0);
}

int udp_init()
{
    if( (udp_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0  ) error("Socket init error");
    int addr_len = sizeof(addr_bc);

    addr_bc.sin_family = AF_INET;
    // addr_bc.sin_addr.s_addr = INADDR_ANY;
    addr_bc.sin_addr.s_addr = INADDR_BROADCAST;
    // inet_pton(AF_INET, "129.241.154.39", &(addr_bc.sin_addr));
    // inet_pton(AF_INET, "127.0.0.1", &(addr_bc.sin_addr));
    addr_bc.sin_port = htons(UDP_BC_PORT);

    if(setsockopt(udp_sock,SOL_SOCKET,SO_BROADCAST,&udp_sock,sizeof(udp_sock)) < 0) error("Opt BC");

    if(  bind(udp_sock, (struct sockaddr*)&addr_bc, addr_len)  <0) error("bind");

    return 1;
}

int main()
{
    udp_init();

    int n_bytes_rcvd;
    char* buf[MAX_UDP_LEN];
    while(1)
    {
        std::cout << "gonna recv\n";
        if( (n_bytes_rcvd = recvfrom(udp_sock,buf,MAX_UDP_LEN, 0, NULL,NULL) )<0) continue;
        std::cout << "rcvd " << n_bytes_rcvd << "bytes\n";
        std::vector<float> amp((float*)buf, (float*)buf+n_bytes_rcvd/sizeof(float) );

        amp[0] = 0.04;
        amp[1] = -0.01;
        clf();
        // plot(range_vec, amp);
        plot(amp);
        pause(0.00001);

    }
}