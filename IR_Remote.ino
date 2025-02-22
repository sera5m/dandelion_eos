#ifndef IR_Remote_C
#define IR_Remote_C



const int adcPin = 32;  // ADC1 pin for IR input
const float pollFrequencyKhz = 80.0;
const int pollIntervalUs = 1000 / pollFrequencyKhz;

#define MAX_SAMPLES 10000
SensorAnalogDataPoint dataBuffer[MAX_SAMPLES];

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


//move this to the math module later and the one above here
void DTA_EMIT(uint8_t pin, SensorAnalogDataPoint* data, int sampleCount) {
    pinMode(pin, OUTPUT);
    for (int i = 0; i < sampleCount; i++) {
        digitalWrite(pin, HIGH);
        delayMicroseconds(data[i].amplitude / 16);  // Basic PWM approximation
        digitalWrite(pin, LOW);
        delayMicroseconds(pollIntervalUs - (data[i].amplitude / 16));
    }
}


//frequency is in bands. 33-40 and 50-60khz, most common is the nec protocol at 38khz. 
//given this we will poll at 80khz or so to get all of the datapoints in, even if these don't perfectly match up
//most of the protocols involved in infared are PWM or amplitude modulation. to emulate this we will have a set amplitude of each value (pwm) INPUT will be recorded by 


#endif
