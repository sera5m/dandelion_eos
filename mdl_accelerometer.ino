#ifndef mdl_accelerometer_H
#define mdl_accelerometer_H




//configs FOR THIS MODULE
#define PMeter_sampleLEN 8  // Window size (this can be adjusted based on your data)- i think best if power of 2

#define PMeter_sampleInterval_s 0.1f //how often we sample, and default set this to 0.1

#define STEP_THRESHOLD 0.08f       // Threshold to detect a step (tune this)
#define DEAD_ZONE 2               // Minimum indices between steps

Vector3 ACCLcurrent_sample; // Declare as extern
 //public current sample set to whatever the fuck this is rn and ext so we can get that otherwhere

//Ensure the circular buffer (ACCLsampleCache) is statically allocated in global memory to avoid stack overflows.
 static Vector3 ACCLsampleCache[PMeter_sampleLEN]; //set len of this vector on start to to pmeter sample len on boot. array OF VECTORS

void initializeSampleCache() {
    for (int i = 0; i < PMeter_sampleLEN; ++i) {
        ACCLsampleCache[i] = Vector3(0, 0, 0);
    }
}

void Update_ACCLsample_IMU();
float totalG(const AccelData& accelData);
float processMagnitude(int i, int N);
int countSteps();



//define the regular stuff
extern MPU6500 IMU; // Declare IMU as an external object
extern AccelData accelData;     //Sensor data
 extern GyroData gyroData;



//honestly you need to look into the "calibrated quat" code from the fastimu examples
//fastimu does most of the things we need, so in this module we'll work on developing the following functions
/*
-usersteps
-watch lifted 
-watch shook
-high g stop
-compass heading-not possible with this sensor.sadge 
-absolute orentation current //not possible without magnetometer-
-a primitive form of gesture sensing including left/right/up/down hand waves, and forward/back arm pull
-more gestures, arm swing and more. i'd like to try to do that
*/

void updateIMU()
{
IMU.update(); // call imu update to the lib we're using--should return xyz where z is the up and down axis
IMU.getAccel(&accelData); //acceldata, lowercase so we know it's from the imu itself
IMU.getTemp();//how to get the stupid temperature

}


//function to return a float output for the total acceleration of devuce based on [sqrt(xx+yy+zz)], the equation to 3d motion.
float totalG(const AccelData& accelData) {
    return std::sqrt(
        accelData.accelX * accelData.accelX +
        accelData.accelY * accelData.accelY +
        accelData.accelZ * accelData.accelZ
    );
} //tip use this to get total motion. if it's more than 1g, the user is on another planet or accelerating




  //get if accel coord if accel DOWNWARD is greater than .84 for like tollerance of slight tilts. will make it turn on if you move more than that many gs, so uhhhhh too bad if you're running?
  //CHANGED TO TAKE 0.85% OF TOTAL GRAVITIES! SO it should work on mars or when ur in cars.  but the downward accel requires a very specific position, so it should just work and not do stupid shit if you're running

//i'm gonna have to add proper cardinal direction input instead of up down but for now this is simple "isfacing up" so we can detect when to turn on. except for like, 5 seconds after this is triggered because this should set a timer
//also, expanding this will mean you should maap axises to the cardinal directions so we can get facing direction. another todo that may be useful later!



bool is_facing_up(const AccelData& accelData) {
    // Calculate total acceleration
    float totalAccel = totalG(accelData);

    // Check if the downward acceleration (accelZ) is at least 85% of total acceleration
    if (totalAccel > 0.0f) { // Prevent division by zero
        return (accelData.accelZ / totalAccel) >= 0.85f;
    }
    return false; // Default to false if no valid total acceleration
}







//this component of dandelion os uses parts of the below linked repo's, liscensed under MIT liscense. [-free to use in commercial apps if credited]
//step rate algorythm adaptead from http://mau.diva-portal.org/smash/get/diva2:1474455/FULLTEXT01 who provided https://github.com/Oxford-step-counter/C-Step-Counter
//thank you to Anna Brondin & Marcus Nordstrom at Malmö University.

// i'm not gonna use whatever they did with pointers, because at this point i'm not experienced enough to reliably make correct functional software out of that.
//i'll use arrays. NOW IF I BARELY FUCKING UNDERSTAND THE CODE I'M WRITING AT MIDNIGHT




void update_IMU() { //this is in main to update the sensor
    // set our vector to imu data
     //take the acceldata from the imu. accelData.accel is the imu, current_sample is our data
     // Scale and store accelerometer data
    ACCLcurrent_sample = Vector3(accelData.accelX, accelData.accelY, accelData.accelZ);

    // Update the sample cache with the new reading
    Update_ACCLsample_IMU(); 
}





//our vector sample cache of acceleration data
 //add the current sample to the current position in the array and set that forward a little bit to make a circular buffer taking samples directly from the imu, UJST LIEK the paper says

void Update_ACCLsample_IMU() {
    // Shift old samples DOWN
    for (int i = PMeter_sampleLEN - 1; i > 0; --i) {
        ACCLsampleCache[i] = ACCLsampleCache[i - 1];
    }
    // Insert new sample at the start
    ACCLsampleCache[0] = ACCLcurrent_sample;
}
//sorting this so often is kina bad. so. just don't. in fact, i'm going to do something else



// step counter formula
//SLIGHTLY BETTER?
float processMagnitude(int i, int N) {// Calculate the summation for the scoring formula
    float score = 0.0f;

    for (int k = -N; k <= N; ++k) { //start for loop to emulate the functionality of summation operations
        if (k != 0) { // Skip current index when we're working with this
            int idx = i + k;// Neighbor index calc
            if (idx >= 0 && idx < PMeter_sampleLEN) { // Check bounds
// Sum the differences of (m_i - m_(i+k) ) 
                float mag_i = ACCLsampleCache[i].magnitude(); // Magnitude of current sample 
                float mag_k = ACCLsampleCache[idx].magnitude();// Magnitude of neighbor
                score += fabs(mag_i - mag_k); // Add absolute difference
            }
        }
    }

    return score / (2 * N); // Normalize by window size
}

//call by updating imu, then calling float score=proscessmagnitude(i,2);

//ooohhhh when will we use this in the real world huh...... RIGHT FUCKING NOW. DO SIGMA SUMMATION NOW!!!!

//now start count steps. chatgpt helped with this one because i only had 45 mins for this
int countSteps() {
    int stepCount = 0;
    int lastStepIndex = -DEAD_ZONE;  // Ensure the first detected step is valid

    // Iterate through the sample cache to detect steps
    for (int i = 1; i < PMeter_sampleLEN - 1; ++i) {
        float score = processMagnitude(i, 2);  // N=2 window size

        // Detect a peak above the threshold
        if (score > STEP_THRESHOLD && (i - lastStepIndex) > DEAD_ZONE) {
            stepCount++;
            lastStepIndex = i;  // Update last step index
        }
    }

    return stepCount;
}


#endif // mdl_accelerometer_H
