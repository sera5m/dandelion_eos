#ifndef mdl_accelerometer_H
#define mdl_accelerometer_H

extern  MPU6500 IMU;  //import extern imu object stfu
// Constants for sampling
#define IMU_SAMPLE_RATE_HZ 10  // 10 Hz (100ms per sample)
#define IMU_CircularBufferSize 20     // 2 seconds of data (10 Hz * 2s)

struct AccelSample {
    float x, y, z;       // Raw accelerometer data (in g's)
    float magnitude;     // Filtered magnitude of acceleration
};

AccelSample accelBuffer[IMU_CircularBufferSize];  // Circular buffer for accelerometer data
int bufferIndex = 0;                   // Current position in the buffer

// Low-pass filter smoothing factor
float alpha = 0.2;

// Applies a simple low-pass filter to smooth the data.
float lowPassFilter(float newValue, float previousValue) {
    return alpha * newValue + (1 - alpha) * previousValue;
}

// Update the accelerometer buffer with new readings.
void updateAccelBuffer(float x, float y, float z) {
    float rawMagnitude = sqrt(x * x + y * y + z * z);
    // Use the current buffer value as the previous value.
    float previousValue = accelBuffer[bufferIndex].magnitude;
    float filteredMagnitude = lowPassFilter(rawMagnitude, previousValue);

    accelBuffer[bufferIndex] = { x, y, z, filteredMagnitude };
    bufferIndex = (bufferIndex + 1) % IMU_CircularBufferSize;
}

// Check if the sample at index is a local peak.
bool isPeak(int index) {
    int prev = (index - 1 + IMU_CircularBufferSize) % IMU_CircularBufferSize;
    int next = (index + 1) % IMU_CircularBufferSize;
    return (accelBuffer[index].magnitude > accelBuffer[prev].magnitude) &&
           (accelBuffer[index].magnitude > accelBuffer[next].magnitude);
}

// Step counter variables:
float stepThreshold = 0.1; // Initial threshold (adjust as needed)
int lastStepIndex = -1;    // Index of the last detected step

// Count steps by checking for peaks above the threshold in the buffer.
int countSteps() {
    int stepCount = 0;

    for (int i = 0; i < IMU_CircularBufferSize; i++) {
        if (isPeak(i) && accelBuffer[i].magnitude > stepThreshold) {
            // Ensure steps are spaced appropriately (e.g., at least 0.5 seconds apart)
            // Using sample indices: (IMU_SAMPLE_RATE_HZ / 2) samples = 0.5s
            if (lastStepIndex == -1 ||
                ((i - lastStepIndex + IMU_CircularBufferSize) % IMU_CircularBufferSize) >= (IMU_SAMPLE_RATE_HZ / 2)) {
                stepCount++;
                lastStepIndex = i;
                // Update threshold dynamically using a smoothing approach.
                stepThreshold = 0.9 * stepThreshold + 0.1 * accelBuffer[i].magnitude;
            }
        }
    }

    return stepCount;
}

// isFacingUp: Returns true if the latest y-axis reading is 0.75g or greater.
bool isFacingUp() {
    // Use the most recent sample in the circular buffer.
    int lastIndex = (bufferIndex - 1 + IMU_CircularBufferSize) % IMU_CircularBufferSize;
    return (accelBuffer[lastIndex].y >= 0.75);
}

// Update IMU: Call this function periodically (e.g., at 10 Hz).
void updateIMU() {
    // Assumes that IMU.update() and IMU.getAccel() exist, and that accelData is filled.
    // Define a struct for accelerometer data if not already done, e.g.:
    // struct AccelData { float accelX, accelY, accelZ; } accelData;
    IMU.update();
    AccelData accelData;
    IMU.getAccel(&accelData);

    // Update our circular buffer with new accelerometer data.
    updateAccelBuffer(accelData.accelX, accelData.accelY, accelData.accelZ);

    /* Count steps and print if any detected.
    int steps = countSteps();
    if (steps > 0) {
        Serial.print("Steps detected: ");
        Serial.println(steps);
    }*/

    // Check if the device is facing up.
    if (isFacingUp()) {
        Serial.println("Device is facing up.");
    } else {
        Serial.println("Device is not facing up.");
    }
}

#endif // mdl_accelerometer_H
