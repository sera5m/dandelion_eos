#ifndef S_HELL_H
#define S_HELL_H

#include "types.h"
#include "helperfunctions.h"
#include "Micro2D_A.h"
#include "globals.h"
#include <cstdint>
#include <Arduino.h>
#pragma once
extern uint8_t CurrentOpenApplicationIndex; //application handler bs

//structs and stuff

 extern WindowCfg d_ls_c_cfg;

 extern WindowCfg d_ls_b_cfg;

 extern WindowCfg d_ls_th_cfg;

//switching apps
void transitionApp(uint8_t index);


//terminal
#endif