#ifndef mdl_accelerometer_H
#define mdl_accelerometer_H

extern  MPU6500 IMU;  //import extern imu object stfu

// Constants for sampling
#define IMU_SAMPLE_RATE_HZ 10  // 10 Hz (100ms per sample)
#define IMU_CircularBufferSize 20     // 2 seconds of data (10 Hz * 2s)

struct AccelSample {
    float ax, ay, az;       // Raw accelerometer data (in g's)
    float magnitude;     // Filtered magnitude of acceleration
};

AccelSample accelBuffer[IMU_CircularBufferSize];  // Circular buffer for accelerometer data
int bufferIndex = 0;                   // Current position in the buffer

extern AccelData imuAccel;   //grab it

float EarthG=9.81; 

// Apply low-pass filter (adjust alpha as needed for smoothness)
float lowPassFilter(float current, float previous, float alpha = 0.2f) {
    return previous + alpha * (current - previous);
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
// Call this every 100ms (IMU_SAMPLE_RATE_HZ)
void pollAccelAndUpdateBuffer() {
    IMU.update();                  // grab raw sensor data
    IMU.getAccel(&imuAccel);       // fill imuAccel.ax, .ay, .az

    float ax = imuAccel.accelX;
    float ay = imuAccel.accelY;
    float az = imuAccel.accelZ;

    float rawMagnitude   = sqrtf(ax*ax + ay*ay + az*az);
    float previousValue  = accelBuffer[bufferIndex].magnitude;
    float filteredMagnitude = lowPassFilter(rawMagnitude, previousValue);

    accelBuffer[bufferIndex].ax        = ax;
    accelBuffer[bufferIndex].ay        = ay;
    accelBuffer[bufferIndex].az        = az;
    accelBuffer[bufferIndex].magnitude = filteredMagnitude;

    bufferIndex = (bufferIndex + 1) % IMU_CircularBufferSize;
}



// isFacingUp: Returns true if the latest y-axis reading is "up or downish"
bool isFacingUp_f() {
    IMU.update();
    IMU.getAccel(&imuAccel);
    return fabsf(imuAccel.accelY) > 6.4f; //6.4 is roughly 0.7*earth gravity(m/s^2)
}

bool isFacingUp() {
    const AccelSample &s = accelBuffer[(bufferIndex - 1 + IMU_CircularBufferSize) % IMU_CircularBufferSize];
    return fabsf(s.ay) > 6.4f;
}





#endif // mdl_accelerometer_H
