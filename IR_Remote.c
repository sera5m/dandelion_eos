
#ifndef IR_Remote_C
#define IR_Remote_C

#include "Wiring.h"

//rememver, use this pin: IR_IO_Pin

const float pollFrequencyKhz = 80.0;
const int pollIntervalUs = 1000 / pollFrequencyKhz;
/*
#define MAX_IR_SAMPLES_PERFILE 10000
SensorAnalogDataPoint dataBuffer[MAX_SAMPLES];



SensorAnalogDataPoint irBuffer[IR_BUFFER_LEN];

QueueHandle_t irCommandQueue;


enum class IRCommandType { RECORD, PLAY };
struct IRCommand {
    IRCommandType type;
    int duration_ms;
};

void IRWorker(void* arg) {
    IRCommand cmd;

    while (true) {
        if (xQueueReceive(irCommandQueue, &cmd, portMAX_DELAY)) {
            if (cmd.type == IRCommandType::RECORD) {
                pinMode(IR_PIN, INPUT);
                // fill irBuffer with samples
                StartIrRecordWindow(cmd.duration_ms);
                // break into chunks and send to helper
                for (int i = 0; i < lastSampleCount; i += MAX_SEGMENT_SIZE) {
                    int len = min(MAX_SEGMENT_SIZE, lastSampleCount - i);
                    ProcessIRSegment(&irBuffer[i], len);
                }
            } else if (cmd.type == IRCommandType::PLAY) {
                pinMode(IR_PIN, OUTPUT);
                EmitRecordedIR();
            }
        }
    }
}

void StartIRSystem() {
    irCommandQueue = xQueueCreate(4, sizeof(IRCommand));
    xTaskCreate(IRWorker, "IRWorker", 2048, nullptr, 2, nullptr);
}

// Example caller
void TriggerRecordIR() {
    IRCommand cmd = {IRCommandType::RECORD, 150};
    xQueueSend(irCommandQueue, &cmd, 0);
}

void StartIrRecordWindow(int durationMS) {
    int totalSamples = (durationMS * 1000) / pollIntervalUs;
    totalSamples = totalSamples > MAX_SAMPLES ? MAX_SAMPLES : totalSamples;

    uint32_t startTime = micros();
    for (int i = 0; i < totalSamples; i++) {
        dataBuffer[i].amplitude = analogRead(adcPin);
        dataBuffer[i].time_us = micros() - startTime;
        delayMicroseconds(pollIntervalUs);
    }
}



void DTA_EMIT(uint8_t pin, SensorAnalogDataPoint* data, int sampleCount) {
    pinMode(pin, OUTPUT);
    for (int i = 0; i < sampleCount; i++) {
        digitalWrite(pin, HIGH);
        delayMicroseconds(data[i].amplitude / 16);  // Basic PWM approximation
        digitalWrite(pin, LOW);
        delayMicroseconds(pollIntervalUs - (data[i].amplitude / 16));
    }
}

*/
//frequency is in bands. 33-40 and 50-60khz, most common is the nec protocol at 38khz. 
//given this we will poll at 80khz or so to get all of the datapoints in, even if these don't perfectly match up
//most of the protocols involved in infared are PWM or amplitude modulation. to emulate this we will have a set amplitude of each value (pwm) INPUT will be recorded by 


#endif

