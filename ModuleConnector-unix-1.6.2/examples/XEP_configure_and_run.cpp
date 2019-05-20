#define _USE_MATH_DEFINES
#include <cmath>
//#include "/home/jens/Documents/plot/matplotlib-cpp/matplotlibcpp.h"

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include "XEP.hpp"
#include "Data.hpp"
#include "xtid.h"
#include "X4M300.hpp"
#include "ModuleConnector.hpp"

/** \example XEP_configure_and_run.cpp
 */

using namespace XeThru;
//namespace plt = matplotlibcpp;

void usage()
{
    std::cout << "Enter the port number of the device" << std::endl;
}

int handle_error(std::string message)
{
    std::cerr << "ERROR: " << message << std::endl;
    return 1;
}

void xep_app_init(XEP* xep)
{
    char configure;
    std::cout << "Would you like to customize XEP configurations(y/n)? ";
    //std::cin >> configure;
    configure = 'n';
    xep->x4driver_init();

    //Setting default values for XEP
    int dac_min = 949;
    int dac_max = 1100;
    //int dac_min = 500;
    //int dac_max = 1500;
    int iteration = 16;
    int pps = 300;
    int fps = 8;
    float offset = 0.18;
    float fa1 = 0.4;
    float fa2 = 5.0;
    int dc = 1;

    if (configure == 'y') {
        //Getting user input for configuration of xep
        std::cout << "Set dac min: "<< std::endl; std::cin >> dac_min;
        std::cout << "Set dac max: "<< std::endl; std::cin >> dac_max;
        std::cout << "Set iterations: "<< std::endl; std::cin >> iteration;
        std::cout << "Set pulse per step: "<< std::endl; std::cin >> pps;
        std::cout << "Set fps: "<< std::endl; std::cin >> fps;
        std::cout << "Set frame area offset: "<< std::endl; std::cin >> offset;
        std::cout << "Set frame area closest: "<< std::endl; std::cin >> fa1;
        std::cout << "Set frame area furthest: "<< std::endl; std::cin >> fa2;
        std::cout << "Would you like to enable downconversion (1/0): " << std::endl; std::cin >> dc;
        std::cout << std::endl;
        //Configure custom XEP
    }

    std::cout << "Configuring XEP" << std::endl;
    //Writing to module XEP
    xep->x4driver_set_dac_min(dac_min);
    xep->x4driver_set_dac_max(dac_max);
    xep->x4driver_set_iterations(iteration);
    xep->x4driver_set_pulses_per_step(pps);
    xep->x4driver_set_fps(fps);
    xep->x4driver_set_frame_area_offset(offset);
    xep->x4driver_set_frame_area(fa1, fa2);
    xep->x4driver_set_downconversion(dc);

    xep->x4driver_set_prf_div(16); // Defualt 16. Can break RF regulations if too low
    
    //plt::plot({1,3,2,4});
    //plt::pause(1);
    
    uint8_t normailzation = 1;
    //xep->set_normalization(normailzation);
    xep->get_normalization(&normailzation);
    std::cout << "Default normalization is: " << unsigned(normailzation) << std::endl;
    std::cout << "Puased until a string is entered:";
    //std::cin >> configure;

}

std::vector<float> get_bbAbs(std::vector<float> bb)
{
    float value;
    int out_sz = bb.size()/2;
    std::vector<float> amp(out_sz);
    for (int i = 0; i<out_sz; i++){
        value = sqrt(pow(bb[i],2)+pow(bb[i+out_sz],2) );
        amp[i] = value;
    }
    return amp;
}

int read_frame(const std::string & device_name)
{
    const unsigned int log_level = 0;
    ModuleConnector mc(device_name, log_level);
    XEP & xep = mc.get_xep();

    std::string FWID;
    //If the module is a X4M200 or X4M300 it needs to be put in manual mode
    xep.get_system_info(0x02, &FWID);
    if (FWID != "XEP") {
        //Module X4M300 or X4M200
        std::string Module;
        X4M300 & x4m300 = mc.get_x4m300();
        x4m300.set_sensor_mode(XTID_SM_STOP,0);
        x4m300.set_sensor_mode(XTID_SM_MANUAL,0);
        xep.get_system_info(0x01, &Module);
        std::cout << "Module " << Module << " set to XEP mode"<< std::endl;
    }

    // Configure XEP
    xep_app_init(&xep);

    //float fc = 7.29e9;     // Lower pulse generator setting
    float fs = 23.328e9;   // X4 sampling rate
    float light_speed = 3e8;
    FrameArea fA;
    xep.x4driver_get_frame_area(&fA);
    float d0 = fA.start;
    float dt = 8*(light_speed/2)/fs;

    // DataFloat
    XeThru::DataFloat test;
    // Check packets queue
    int packets = xep.peek_message_data_float();
    std::cout << "Packets in queue: " << packets<<'\n';

    // Printing output until KeyboardInterrupt
    // This data is not readable.
    std::cout << "Printing out data: " << '\n';
    for(int step=0; step<10;step++) {
        if (xep.read_message_data_float(&test)) {
            return handle_error("read_message_data_float failed");
        }
        std::vector<float> outData = test.data;
        int sz = outData.size();
        std::vector<float> amplitude = get_bbAbs(outData);
        std::vector<float> dist(amplitude.size());
        for(uint i=0; i<dist.size(); i++){
            dist[i] = d0+i*dt;
        }
        //plt::clf();
        //plt::plot(outData);
        //plt::plot(dist, amplitude);
        //plt::pause(0.0001);
        std::cout << sz << std::endl;
        for (int x = 0; x < sz; ++x) {
            //printing out unreadable RF-data
            //std::cout << test.data[x] << " ";
        }
        std::cout << '\n';
    }
    xep.x4driver_set_fps(0);
    xep.module_reset();
    return 0;
}


int main(int argc, char ** argv)
{
    if (argc < 2) {
        usage();
        return 2;
    }
    const std::string device_name = argv[1];
    return read_frame(device_name);
}
