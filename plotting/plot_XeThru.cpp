
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

#include <vector>

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

static std::vector<float> range_vec;

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
    addr_bc.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, "192.168.1.49", &(addr_bc.sin_addr));
    // inet_pton(AF_INET, "127.0.0.1", &(addr_bc.sin_addr));
    addr_bc.sin_port = htons(UDP_BC_PORT);

    if(setsockopt(udp_sock,SOL_SOCKET,SO_BROADCAST,&udp_sock,sizeof(udp_sock)) < 0) error("Opt BC");
    int bind_ret = bind(udp_sock, (struct sockaddr*)&addr_bc, addr_len);
    if(bind_ret <0) error("bind");

    return 1;
}

void init_range_vec(int len, float offset=0.4)
{    
    range_vec.clear();
    range_vec.reserve(len);
    int bin_reduction = 8;
    double fs = 23.328e9;   //sampler at 23.328 GS/s
    double c = 299792458;
    double bin_to_delay = bin_reduction * c/(fs*2);
    for(int i=0; i<len; i++)
    {
        // range_vec[i] = bin_to_delay*i;
        range_vec.push_back(bin_to_delay*i+offset);
    
    }
}


int main()
{
    udp_init();

    int n_bytes_rcvd;
    char* buf[MAX_UDP_LEN];
    while(1)
    {
        if( (n_bytes_rcvd = recvfrom(udp_sock,buf,MAX_UDP_LEN, 0, NULL,NULL) )<0) continue;
        std::cout << "rcvd " << n_bytes_rcvd << "bytes\n";
        std::vector<float> amp((float*)buf, (float*)buf+n_bytes_rcvd/sizeof(float) );

        if( range_vec.size()!=amp.size() ) init_range_vec(amp.size(), 0.4);
        clf();
        plot(range_vec, amp);
        // plot(amp);
        ylim(-0.01, 0.04);
        grid(true);
        pause(0.00001);

    }
}