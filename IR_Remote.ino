
#ifndef IR_Remote_C
#define IR_Remote_C

#include "Wiring.h"

//rememver, use this pin: IR_IO_Pin

const float pollFrequencyKhz = 80.0;
const int pollIntervalUs = 1000 / pollFrequencyKhz;

#include "esp_heap_caps.h"      // for PSRAM malloc on ESP32
#include "driver/rmt_tx.h" //timing circut on the inside of the esp32 that generates timed gipo pulses.
//https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/rmt.html


rmt_channel_handle_t tx_chan = NULL;
rmt_tx_channel_config_t tx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
    .gpio_num = IR_IO_Pin,                    // GPIO number
    .mem_block_symbols = 64,          // memory block size, 64 * 4 = 256 Bytes
    .resolution_hz = 1 * 1000 * 1000, // 1 MHz tick resolution, i.e., 1 tick = 1 µs
    .trans_queue_depth = 4,           // set the number of transactions that can pend in the background
    .flags.invert_out = false,        // do not invert output signal
    .flags.with_dma = false,          // do not need DMA backend
};
ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_chan));


//IRSample* irBuffer = nullptr;

size_t collectedSamples = 0;
const float IRpollFreqKhz = 38.0f; //by default, the ir polls at 38khz with remotes and shit
const uint32_t ir_pollIntervalUs = uint32_t(1000.0f / IRpollFreqKhz);

// Adjust based on available RAM

struct IRSample {
  uint32_t time_us;
  uint16_t amplitude;
}__attribute__((packed));//shrinky and simplify my calculations

class IRWorker {
public:
//this better config at create time
const float IR_RECORD_S=3.0f; //todo expose to settings
const size_t IR_SAMPLE_MAX = (IR_RECORD_S/(48*IRpollFreqKhz));  //bitsize 500kb/(16+32 bits)=result
//

//total alloc size/bytes per second
    enum IRWORKER_MODE {
        IDLE,
        RECORD_PRIMED,
        RECORDING,
        EMIT_PRIMED,
        EMITTING,
        SAVING,
        PROCESSING,
        BEACON
    };

    enum PROCESSING_STATE {
        DENOISING,
        CLEANING,
        ALIGNING,
        DONE
    };

    IRWorker(uint8_t irPin) : 
        irPin_(irPin), 
        currentMode_(IDLE),
        buffer_(nullptr),
        bufferSize_(0),
        sampleIndex_(0),
        emitIndex_(0),
        beaconInterval_(1000),
        processingState_(DONE),
        lastBeaconToggle_(0),
        taskHandle_(nullptr)
    {
        pinMode(irPin_, INPUT);
    }

    ~IRWorker() {
        cleanup();
    }

    // ---- Mode Management ----
    bool setMode(IRWORKER_MODE newMode) {
        if (currentMode_ == newMode) return true;

        // Validate transition
        if (!validateTransition(currentMode_, newMode)) {
            Serial.println("Invalid mode transition");
            return false;
        }

        // Cleanup current mode
        if (!cleanupCurrentMode()) {
            Serial.println("Failed to cleanup current mode");
            return false;
        }

        // Initialize new mode
        if (!initNewMode(newMode)) {
            Serial.println("Failed to initialize new mode");
            return false;
        }

        currentMode_ = newMode;
        return true;
    }

    // ---- Recording Functions ----
    bool primeRecording() {
        if (currentMode_ != IDLE) return false;
        
        if (buffer_) free(buffer_);
        bufferSize_ = IR_SAMPLE_MAX;
        buffer_ = (IRSample*)ps_malloc(bufferSize_ * sizeof(IRSample));
        
        if (!buffer_) {
            Serial.println("PSRAM allocation failed");
            return false;
        }
        
        pinMode(irPin_, INPUT);
        currentMode_ = RECORD_PRIMED;
        return true;
    }

    bool startRecording(uint32_t durationMs) {
        if (currentMode_ != RECORD_PRIMED) return false;

        recordingDuration_ = durationMs;
        xTaskCreatePinnedToCore(
            recordingTask, "IRRec", 4096, this, 2, &taskHandle_, 1
        );

        if (!taskHandle_) {
            Serial.println("Task creation failed");
            return false;
        }

        currentMode_ = RECORDING;
        return true;
    }

    // ---- Emission Functions ----
    bool primeEmission() {
        if (currentMode_ != IDLE) return false;
        
        if (!buffer_ || bufferSize_ == 0) {
            Serial.println("No recording to emit");
            return false;
        }
        
        pinMode(irPin_, OUTPUT);
        currentMode_ = EMIT_PRIMED;
        return true;
    }

    bool startEmission() {
        if (currentMode_ != EMIT_PRIMED) return false;

        xTaskCreatePinnedToCore(
            emissionTask, "IREmit", 4096, this, 2, &taskHandle_, 1
        );

        if (!taskHandle_) {
            Serial.println("Task creation failed");
            return false;
        }

        currentMode_ = EMITTING;
        return true;
    }

    // ---- Beacon Functions ----
    bool startBeacon(uint16_t frequencyHz) {
        if (currentMode_ != IDLE) return false;

        beaconInterval_ = 1000 / frequencyHz;
        pinMode(irPin_, OUTPUT);
        currentMode_ = BEACON;
        return true;
    }

    // ---- Processing Functions ----
    bool startProcessing() {
        if (currentMode_ != IDLE || !buffer_) return false;

        processingState_ = DENOISING;
        currentMode_ = PROCESSING;
        return true;
    }

    // ---- Utility Functions ----
    size_t getSampleCount() const { return sampleIndex_; }
    bool isActive() const { return currentMode_ != IDLE; }
    IRWORKER_MODE getCurrentMode() const { return currentMode_; }

private:
    // ---- Core Variables ----
    uint8_t irPin_;
    IRWORKER_MODE currentMode_;
    IRSample* buffer_;
    size_t bufferSize_;
    size_t sampleIndex_;
    size_t emitIndex_;
    uint32_t recordingDuration_;
    uint16_t beaconInterval_;
    uint32_t lastBeaconToggle_;
    PROCESSING_STATE processingState_;
    TaskHandle_t taskHandle_;

    // ---- Mode Validation ----
    bool validateTransition(IRWORKER_MODE current, IRWORKER_MODE next) {
        static const bool validTransitions[8][8] = {
            /* IDLE */       {0,1,0,1,0,0,1,1},
            /* RECORD_PRIMED*/ {1,0,1,0,0,0,0,0},
            /* RECORDING */   {1,0,0,0,0,0,0,0},
            /* EMIT_PRIMED */ {1,0,0,0,1,0,0,0},
            /* EMITTING */    {1,0,0,0,0,0,0,0},
            /* SAVING */      {1,0,0,0,0,0,0,0},
            /* PROCESSING */  {1,0,0,0,0,0,0,0},
            /* BEACON */      {1,0,0,0,0,0,0,0}
        };
        return validTransitions[current][next];
    }

    // ---- Mode Cleanup ----
    bool cleanupCurrentMode() {
        switch (currentMode_) {
            case RECORDING:
            case EMITTING:
            case BEACON:
                if (taskHandle_) {
                    vTaskDelete(taskHandle_);
                    taskHandle_ = nullptr;
                }
                break;
            default:
                break;
        }
        return true;
    }

    // ---- Mode Initialization ----
    bool initNewMode(IRWORKER_MODE newMode) {
        switch (newMode) {
            case BEACON:
                xTaskCreatePinnedToCore(
                    beaconTask, "IRBeacon", 2048, this, 1, &taskHandle_, 1
                );
                break;
            default:
                break;
        }
        return true;
    }

    // ---- Task Functions ----
    static void recordingTask(void* arg) {
        IRWorker* self = static_cast<IRWorker*>(arg);
        const uint32_t intervalUs = 26; // ~38kHz (1000/38 ≈ 26μs)
        uint32_t startTime = micros();
        uint32_t endTime = startTime + (self->recordingDuration_ * 1000);

        self->sampleIndex_ = 0;
        while (micros() < endTime && self->sampleIndex_ < self->bufferSize_) {
            uint32_t currentTime = micros() - startTime;
            
            self->buffer_[self->sampleIndex_].time_us = currentTime;
            self->buffer_[self->sampleIndex_].amplitude = analogRead(self->irPin_);
            self->sampleIndex_++;

            // Busy-wait for timing accuracy
            while (micros() - startTime < currentTime + intervalUs);
        }

        self->setMode(IDLE);
        vTaskDelete(NULL);
    }

    static void emissionTask(void* arg) {
        IRWorker* self = static_cast<IRWorker*>(arg);
        
        for (self->emitIndex_ = 0; self->emitIndex_ < self->sampleIndex_; self->emitIndex_++) {
            uint32_t targetTime = self->buffer_[self->emitIndex_].time_us;
            uint16_t amplitude = self->buffer_[self->emitIndex_].amplitude;
            
            // Convert amplitude to PWM (simplified)
            analogWrite(self->irPin_, amplitude >> 4);
            
            // Busy-wait for timing
            while (micros() < targetTime);
        }

        self->setMode(IDLE);
        vTaskDelete(NULL);
    }

    static void beaconTask(void* arg) {
        IRWorker* self = static_cast<IRWorker*>(arg);
        bool state = false;

        while (true) {
            uint32_t currentTime = millis();
            if (currentTime - self->lastBeaconToggle_ >= self->beaconInterval_) {
                state = !state;
                digitalWrite(self->irPin_, state);
                self->lastBeaconToggle_ = currentTime;
            }
            vTaskDelay(1); // Yield to other tasks
        }
    }

    // ---- Cleanup ----
    void cleanup() {
        if (taskHandle_) {
            vTaskDelete(taskHandle_);
            taskHandle_ = nullptr;
        }
        if (buffer_) {
            free(buffer_);
            buffer_ = nullptr;
        }
        currentMode_ = IDLE;
    }
};



//frequency is in bands. 33-40 and 50-60khz, most common is the nec protocol at 38khz. 
//given this we will poll at 80khz or so to get all of the datapoints in, even if these don't perfectly match up
//most of the protocols involved in infared are PWM or amplitude modulation. to emulate this we will have a set amplitude of each value (pwm) INPUT will be recorded by 


#endif

