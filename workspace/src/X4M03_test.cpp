#include "XEP.hpp"
#include "ModuleConnector.hpp"
#ifndef __ARM__
#include "matplotlibcpp.h"
#endif

#include <algorithm>
#include <functional> 

#include <iostream>
#include <csignal>
#include <unistd.h>

#ifdef __ARM__
    #define plot(...)
    #define pause(...)
    #define clf(...)
#endif

using namespace matplotlibcpp;


namespace{
  volatile std::sig_atomic_t gSignalStatus = 0;
}


template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    std::vector<T> result;
    result.reserve(a.size());
    
    for(int i=0; i<a.size(); i++)
    {
        result.push_back(a[i]+b[i]);
    }
    return result;
}
template <typename T>
std::vector<T> operator-(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    std::vector<T> result;
    result.reserve(a.size());
    
    for(int i=0; i<a.size(); i++)
    {
        result.push_back(a[i]-b[i]);
    }
    return result;
}
template <typename T>
std::vector<T> operator+=(std::vector<T>&a, const std::vector<T>&b)
{
    a = a+b;
    return a;
}
template<typename T>
std::vector<T> operator*(const std::vector<T> vec, const T scalar)
{
    std::vector<T> result;
    result.reserve(vec.size());
    for(int i=0; i<vec.size(); i++)
    {
        result.push_back(vec[i]*scalar);
    }
    return result;
}


// static XeThru::XEP xep;

static std::vector<float> range_vec;
static std::vector<float> bg_amp;
static std::vector<float> bg_amp_accumulated;
static unsigned int n_bg_amp;
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

int range_vector_init(std::vector<float>* p_range_vec, bool downc_converted)
{
    int bin_reduction = downc_converted? 8:1;
    int max_bins_downconverted = 1536;
    double fs = 23.328e9;   //sampler at 23.328 GS/s
    double c = 299792458;
    double bin_to_delay = bin_reduction * c/(fs*2);
    for(int i=0; i<p_range_vec->size(); i++)
    {
        (range_vec)[i] = bin_to_delay*i;
    }
}

int radar_init(char* device_name, XeThru::XEP* p_xep)
{    
    int x4init = p_xep->x4driver_init();
    x4init = 0; 
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

    p_xep->x4driver_set_dac_max(dac_max);
    p_xep->x4driver_set_dac_min(dac_min);
    p_xep->x4driver_set_iterations(iteration);
    p_xep->x4driver_set_pulses_per_step(pps);
    p_xep->x4driver_set_fps(fps);
    p_xep->x4driver_set_frame_area_offset(offset);
    p_xep->x4driver_set_frame_area(fa1, fa2);
    p_xep->x4driver_set_downconversion(dc);

    p_xep->x4driver_set_prf_div(16); // Defualt 16. Can break RF regulations if too low

    uint8_t normailzation = 1;
    p_xep->get_normalization(&normailzation);
    std::cout << "Default normalization is: " << unsigned(normailzation) << std::endl;

    return 0;
}

int radar_step(XeThru::XEP* p_xep, int calibrate)
{
    //std::cout << "Is in loop\n";
    std::vector<float> radarFloat_vec;
    XeThru::DataFloat radarFloats;
    p_xep->read_message_data_float(&radarFloats);
    radarFloat_vec = radarFloats.get_data();
    std::vector<float> amp = get_bbAbs(radarFloat_vec);
    if(calibrate)
    {
        if(bg_amp_accumulated.size()==0)
        {
            bg_amp_accumulated = amp;
            range_vec = amp;
            range_vector_init(&range_vec, true);
            return 0;
        }
        bg_amp_accumulated = bg_amp_accumulated + amp;
        n_bg_amp++;

        clf();
        plot(range_vec, bg_amp_accumulated);
        // plot(bg_amp_accumulated);
        pause(0.00001);

        if(calibrate==2)
        {
            bg_amp = bg_amp_accumulated * (float)(1/(float)n_bg_amp);                
            printf("bg_amp is made\n");       
        }
        return 0;
    }
    amp = amp - bg_amp;
    amp[0] = 0.04;
    amp[1] = -0.01;
    clf();
    plot(range_vec, amp);
    pause(0.00001);
}


void radar_terminate(XeThru::XEP* p_xep)
{
    std::cout << "Exiting main\n";
    p_xep->x4driver_set_fps(0);
    p_xep->module_reset();
    //delete p_xep;
    std::cout << "Module is reset\n";
}

int main(int argc, char ** argv)
{   std::signal(SIGINT, exitHandle);

    if(argc != 2){
        std::cout << "Wrong argument count. Correct format is:\n ./XeThru /dev/ttyACM<x>\n";
        return -1;
    }

    char* device_name = argv[1];
    
    XeThru::XEP* p_xep;
    
    XeThru::ModuleConnector mc(device_name, 0);
    static XeThru::XEP& (xep) = mc.get_xep();
    p_xep = &xep;
    radar_init(device_name, p_xep);

    for(int i=0; i<100 & !gSignalStatus; i++)
    {
        std::cout << "calib:\n";
        radar_step(p_xep, 1);
    }
    radar_step(p_xep, 2); // Do the averaging

    while(gSignalStatus == 0)
    {
        radar_step(p_xep, 0);
    }

    std::cout << "Exiting main\n";
    radar_terminate(p_xep);
    std::cout << "Module is reset\n";

    return 0;
}


int not_main(int argc, char ** argv)
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
    int i = 0;
    while(gSignalStatus == 0)
    {
        //std::cout << "Is in loop\n";
        xep.read_message_data_float(&radarFloats);
        radarFloat_vec = radarFloats.get_data();
        amp = get_bbAbs(radarFloat_vec);
        if(i<=40)
        {
            if(i++==0)
            {
                bg_amp_accumulated = amp;    
                continue;
            }
            bg_amp_accumulated = bg_amp_accumulated + amp;
            n_bg_amp++;

            clf();
            plot(bg_amp_accumulated);
            pause(0.00001);
            if(i==40)
            {
                // std::transform(bg_amp_accumulated.begin(), bg_amp_accumulated.end(), bg_amp.begin(),
                //                                 std::bind(std::multiplies<float>(), std::placeholders::_1, n_bg_amp));
                bg_amp = bg_amp_accumulated * (float)(1/(float)n_bg_amp);                
                printf("bg_amp is made\n");       
            }
            continue;
        }
        amp = amp - bg_amp;
        amp[0] = 0.04;
        clf();
        plot(amp);
        pause(0.00001);
    }

    std::cout << "Exiting main\n";
    xep.x4driver_set_fps(0);
    xep.module_reset();
    delete &xep;
    std::cout << "Module is reset\n";

    return 0;
}