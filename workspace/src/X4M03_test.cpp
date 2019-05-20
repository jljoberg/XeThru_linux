#include "XEP.hpp"
#include "ModuleConnector.hpp"
#include "matplotlibcpp.h"

#include <iostream>
#include <csignal>
#include <unistd.h>

namespace plt = matplotlibcpp;

namespace{
  volatile std::sig_atomic_t gSignalStatus = 0;
}

void exitHandle(int s)
{
    std::cout << "Exiting using signal\n";
    if (gSignalStatus == s){
        exit(1);
    }
    gSignalStatus = s;
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
int main(int argc, char ** argv)
{
    std::signal(SIGINT, exitHandle);

    if(argc != 2){
        std::cout << "Wrong argument count. Correct format is:\n ./XeThru /dev/ttyACM<x>\n";
        return -1;
    }
    char* device_name = argv[1];
    XeThru::ModuleConnector mc(device_name, 0);
    XeThru::XEP & xep = mc.get_xep();

    int x4init = xep.x4driver_init();
    x4init = 0; // Reset loop seems to fuck up 
    if (x4init != 0){
        usleep(500e3);
        xep.module_reset();
        std::cout << "gonna sleep\n";
        usleep(4000e3);
        std::cout << "slept\n";
        XeThru::XEP & xep = mc.get_xep();
        x4init = xep.x4driver_init();
        /*
        if (x4init != 0){
            std::cout << "Init failed\n";
            xep.module_reset();
            return 0;
        }*/
    }
    std::cout << "Init returns int value: " << x4init << std::endl;

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

    xep.x4driver_set_dac_max(dac_max);
    xep.x4driver_set_dac_min(dac_min);
    xep.x4driver_set_iterations(iteration);
    xep.x4driver_set_pulses_per_step(pps);
    xep.x4driver_set_fps(fps);
    xep.x4driver_set_frame_area_offset(offset);
    xep.x4driver_set_frame_area(fa1, fa2);
    xep.x4driver_set_downconversion(dc);

    xep.x4driver_set_prf_div(16); // Defualt 16. Can break RF regulations if too low

    uint8_t normailzation = 1;
    xep.get_normalization(&normailzation);
    std::cout << "Default normalization is: " << unsigned(normailzation) << std::endl;


    //xep.module_reset();

    XeThru::DataFloat radarFloats;
    std::vector<float> radarFloat_vec;
    std::vector<float> amp;
    while(gSignalStatus == 0)
    {
        std::cout << "Is in loop\n";
        xep.read_message_data_float(&radarFloats);
        radarFloat_vec = radarFloats.get_data();
        amp = get_bbAbs(radarFloat_vec);
        plt::clf();
        plt::plot(amp);
        plt::pause(0.00001);
    }

    std::cout << "Exiting main\n";
    xep.x4driver_set_fps(0);
    xep.module_reset();
    std::cout << "Module is reset\n";

    return 0;
}