#ifndef mdl_accelerometer_H
#define mdl_accelerometer_H


#define SAMPLE_RATE_HZ 10  // 10 Hz (100ms per sample)
#define BUFFER_SIZE 20     // 2 seconds of data (10 Hz * 2s)

struct AccelSample {
    float x, y, z;         // Raw accelerometer data
    float magnitude;       // Filtered magnitude
};

AccelSample accelBuffer[BUFFER_SIZE]; // Circular buffer for accelerometer data
int bufferIndex = 0;                  // Current position in the buffer

float alpha = 0.2; // Smoothing factor (adjust as needed)

float lowPassFilter(float newValue, float previousValue) {
    return alpha * newValue + (1 - alpha) * previousValue;
}

void updateAccelBuffer(float x, float y, float z) {
    // Calculate raw magnitude
    float rawMagnitude = sqrt(x * x + y * y + z * z);

    // Apply low-pass filter
    float filteredMagnitude = lowPassFilter(rawMagnitude, accelBuffer[bufferIndex].magnitude);

    // Store in buffer
    accelBuffer[bufferIndex] = {x, y, z, filteredMagnitude};

    // Update buffer index
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
}
bool isPeak(int index) {
    int prev = (index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    int next = (index + 1) % BUFFER_SIZE;

    return accelBuffer[index].magnitude > accelBuffer[prev].magnitude &&
           accelBuffer[index].magnitude > accelBuffer[next].magnitude;
}bool isPeak(int index) {
    int prev = (index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    int next = (index + 1) % BUFFER_SIZE;

    return accelBuffer[index].magnitude > accelBuffer[prev].magnitude &&
           accelBuffer[index].magnitude > accelBuffer[next].magnitude;
}
bool isPeak(int index) {
    int prev = (index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    int next = (index + 1) % BUFFER_SIZE;

    return accelBuffer[index].magnitude > accelBuffer[prev].magnitude &&
           accelBuffer[index].magnitude > accelBuffer[next].magnitude;
}
float stepThreshold = 0.1; // Initial threshold (adjust as needed)
int lastStepIndex = -1;    // Index of the last detected step

int countSteps() {
    int stepCount = 0;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (isPeak(i) && accelBuffer[i].magnitude > stepThreshold) {
            // Ensure steps are spaced appropriately (e.g., 0.5 seconds)
            if (lastStepIndex == -1 || (i - lastStepIndex) >= (SAMPLE_RATE_HZ / 2)) {
                stepCount++;
                lastStepIndex = i;

                // Update threshold dynamically
                stepThreshold = 0.9 * stepThreshold + 0.1 * accelBuffer[i].magnitude;
            }
        }
    }

    return stepCount;
}
void updateIMU() {
    IMU.update();
    IMU.getAccel(&accelData);

    // Update accelerometer buffer
    updateAccelBuffer(accelData.accelX, accelData.accelY, accelData.accelZ);

    // Count steps
    int steps = countSteps();
    if (steps > 0) {
        Serial.print("Steps detected: ");
        Serial.println(steps);
    }
}

#endif // mdl_accelerometer_H
