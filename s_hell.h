#ifndef S_HELL_H
#define S_HELL_H

#include "types.h"
#include "helperfunctions.h"
#include "Micro2D_A.h"
#include "globals.h"
#include <cstdint>
#include <Arduino.h>
#pragma once
// Tracks current open app index

//structs and stuff

 extern WindowCfg d_ls_c_cfg;

 extern WindowCfg d_ls_b_cfg;

 extern WindowCfg d_ls_th_cfg;

 extern QueueHandle_t ProcInputQueTarget;
 extern QueueHandle_t processInputQueue;
 extern TaskHandle_t inputTaskHandle;
 
 void CreateInputHandler();

 extern void WatchScreenTask(void*); //defined in mainapp
 extern void nfcTask(void*);//defined at nfcapp


 extern AppName CurrentOpenApplicationIndex;
 // Fetch the TaskHandle pointer for a given app index
 TaskHandle_t* GetTaskHandleByIndex(AppName index);

extern TaskHandle_t inputTaskHandle;//never stopped or changed


//ptr's to tasks
extern TaskHandle_t currentAppTaskHandle;
extern TaskHandle_t watchScreenHandle;
extern TaskHandle_t healthTaskHandle;
extern TaskHandle_t nfcTaskHandle;
extern TaskHandle_t settingsTaskHandle;
extern TaskHandle_t gyroInfoTaskHandle;
extern TaskHandle_t filesTaskHandle;
extern TaskHandle_t radioTaskHandle;
extern TaskHandle_t irRemoteTaskHandle;
extern TaskHandle_t utilitiesTaskHandle;
extern TaskHandle_t etoolsTaskHandle;
extern TaskHandle_t rubberDuckyTaskHandle;
extern TaskHandle_t connectionsTaskHandle;
extern TaskHandle_t smartDevicesTaskHandle;
extern TaskHandle_t diagnosticsTaskHandle;
extern TaskHandle_t gamesTaskHandle;


void INPUT_tick(void *pvParameters);



//switching apps







// Switch from one app to another, suspending or deleting the old task
bool on_app_change(
    AppName newIndex,
    TaskHandle_t* newTaskHandlePtr,
    AppName oldIndex,
    TaskHandle_t* oldTaskHandlePtr,
    bool deleteOldTask
);

// High-level transition function
void transitionApp(AppName newApp, bool deleteOldTask = false);

void SleepApp(TaskHandle_t target);
void LaunchApp(TaskHandle_t target);





#endif