#ifndef DISPLAY_HELPER_H
#define DISPLAY_HELPER_H

#include <string>

struct DisplayVars {
    float Battery = 0.0f;
    float Soc = 0.0f;
    double Min_Cell = 0.0;
    double Max_Cell = 0.0;
    std::string Time = "00:00";
    double Min_Pack = 0.0;
    double Max_Pack = 0.0;
    double Pack = 0.0;
    double Motor = 0.0;
    double Solar = 0.0; 
    uint8_t Speed = 0;
};

#endif