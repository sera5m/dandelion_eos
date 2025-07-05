
#ifndef IR_Remote_C
#define IR_Remote_C

#include "Wiring.h"

//rememver, use this pin: IR_IO_Pin

#ifndef IR_Remote_C
#define IR_Remote_C

#include "Wiring.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "esp_heap_caps.h"

// RMT configuration
const uint32_t RMT_RESOLUTION_HZ = 1000000; // 1 µs resolution
const uint16_t RMT_MEM_BLOCK_SYMBOLS = 64;
const uint8_t RMT_TX_QUEUE_DEPTH = 4;
const uint16_t RMT_RX_FILTER_US = 100;     // 100µs filter
const uint16_t RMT_RX_IDLE_THRESHOLD_US = 5000; // 5ms end of frame

// IR protocols
const uint16_t IR_NEC_FREQUENCY = 38000;
const uint16_t IR_SAMSUNG_FREQUENCY = 40000;
const uint16_t IR_SONY_FREQUENCY = 40000;

class IRWorker {
public:
    enum IRWORKER_MODE { //i need to make the ir have the prior state transition model
        IDLE,
        RECORDING,
        EMITTING,
        BEACON
    };

    IRWorker(uint8_t irPin) : 
        irPin_(irPin),
        currentMode_(IDLE),
        tx_chan_(NULL),
        rx_chan_(NULL),
        buffer_(NULL),
        buffer_size_(0),
        recorded_symbols_(0),
        beacon_freq_(38000),
        task_handle_(NULL) {}

    ~IRWorker() {
        cleanup();
    }

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

bool validateTransition(IRWORKER_MODE current, IRWORKER_MODE next) {//not sure if this should still even be here idk
        static const bool validTransitions[8][8] = {
            /* IDLE */       {0,1,0,1,0,0,1,1},
            /* RECORD_PRIMED*/{1,0,1,0,0,0,0,0},
            /* RECORDING */   {1,0,0,0,0,0,0,0},
            /* EMIT_PRIMED */ {1,0,0,0,1,0,0,0},
            /* EMITTING */    {1,0,0,0,0,0,0,0},
            /* SAVING */      {1,0,0,0,0,0,0,0},
            /* PROCESSING */  {1,0,0,0,0,0,0,0},
            /* BEACON */      {1,0,0,0,0,0,0,0} 
        
        };
        return validTransitions[current][next];
    }



    bool startRecording(size_t duration_ms) {
        if (currentMode_ != IDLE) return false;
        
        // Allocate PSRAM buffer
        buffer_size_ = duration_ms * 1000 / 26;  // 38kHz ≈ 26µs per sample
        buffer_ = (rmt_symbol_word_t*)ps_malloc(buffer_size_ * sizeof(rmt_symbol_word_t));
        
        if (!buffer_) {
            Serial.println("PSRAM allocation failed");
            return false;
        }

        // Initialize RMT RX
        if (!setup_rx()) {
            Serial.println("RMT RX setup failed");
            free(buffer_);
            buffer_ = NULL;
            return false;
        }

        // Start recording task
        xTaskCreatePinnedToCore(
            recording_task, "IRRec", 4096, this, 2, &task_handle_, 1
        );

        currentMode_ = RECORDING;
        return true;
    }

    bool startEmission() {
        if (currentMode_ != IDLE || !buffer_ || recorded_symbols_ == 0) 
            return false;

        // Initialize RMT TX
        if (!setup_tx()) {
            Serial.println("RMT TX setup failed");
            return false;
        }

        // Start emission task
        xTaskCreatePinnedToCore(
            emission_task, "IREmit", 4096, this, 2, &task_handle_, 1
        );

        currentMode_ = EMITTING;
        return true;
    }

    bool startBeacon(uint16_t frequency = 38000) {
        if (currentMode_ != IDLE) return false;
        
        beacon_freq_ = frequency;
        xTaskCreatePinnedToCore(
            beacon_task, "IRBeacon", 2048, this, 1, &task_handle_, 1
        );
        
        currentMode_ = BEACON;
        return true;
    }

    void stop() {
        if (currentMode_ != IDLE) {
            currentMode_ = IDLE;
            if (task_handle_) {
                vTaskDelay(10); // Allow task to exit
                task_handle_ = NULL;
            }
            cleanup();
        }
    }

    size_t getRecordedSamples() const { return recorded_symbols_; }
    bool isRecording() const { return currentMode_ == RECORDING; }
    bool isEmitting() const { return currentMode_ == EMITTING; }

private:
    uint8_t irPin_;
    IRWORKER_MODE currentMode_;
    rmt_channel_handle_t tx_chan_;
    rmt_channel_handle_t rx_chan_;
    rmt_symbol_word_t* buffer_;
    size_t buffer_size_;
    size_t recorded_symbols_;
    uint16_t beacon_freq_;
    TaskHandle_t task_handle_;

    bool setup_tx() {
        if (!tx_chan_) {
            rmt_tx_channel_config_t tx_cfg = {
                .gpio_num = irPin_,
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = RMT_RESOLUTION_HZ,
                .mem_block_symbols = RMT_MEM_BLOCK_SYMBOLS,
                .trans_queue_depth = RMT_TX_QUEUE_DEPTH,
                .flags = {
                    .invert_out = false,
                    .with_dma = false,
                }
            };
            if (rmt_new_tx_channel(&tx_cfg, &tx_chan_) != ESP_OK) {
                return false;
            }
        }
        return rmt_enable(tx_chan_) == ESP_OK;
    }

    bool setup_rx() {
        if (!rx_chan_) {
            rmt_rx_channel_config_t rx_cfg = {
                .gpio_num = irPin_,
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = RMT_RESOLUTION_HZ,
                .mem_block_symbols = RMT_MEM_BLOCK_SYMBOLS,
                .flags = {
                    .invert_in = false,
                    .with_dma = false,
                }
            };
            if (rmt_new_rx_channel(&rx_cfg, &rx_chan_) != ESP_OK) {
                return false;
            }
        }

        rmt_rx_event_callbacks_t cbs = {
            .on_recv_done = recv_done_callback
        };
        return rmt_rx_register_event_callbacks(rx_chan_, &cbs, this) == ESP_OK &&
               rmt_enable(rx_chan_) == ESP_OK;
    }

    static bool recv_done_callback(rmt_channel_handle_t channel, 
                                  const rmt_rx_done_event_data_t *edata, 
                                  void *user_data) {
        IRWorker* self = static_cast<IRWorker*>(user_data);
        
        // Calculate how much space remains in buffer
        size_t remaining = self->buffer_size_ - self->recorded_symbols_;
        size_t to_copy = (edata->num_symbols < remaining) ? 
                         edata->num_symbols : remaining;
        
        // Copy symbols to buffer
        memcpy(self->buffer_ + self->recorded_symbols_, 
               edata->received_symbols, 
               to_copy * sizeof(rmt_symbol_word_t));
        
        self->recorded_symbols_ += to_copy;
        
        // If buffer full, stop recording
        if (self->recorded_symbols_ >= self->buffer_size_) {
            self->stop();
        }
        return false;
    }

    static void recording_task(void* arg) {
        IRWorker* self = static_cast<IRWorker*>(arg);
        
        // Configure RX settings
        rmt_receive_config_t rx_cfg = {
            .signal_range_min_ns = RMT_RX_FILTER_US * 1000,
            .signal_range_max_ns = RMT_RX_IDLE_THRESHOLD_US * 1000
        };
        
        // Start receiving
        rmt_receive(self->rx_chan_, self->buffer_, 
                   self->buffer_size_ * sizeof(rmt_symbol_word_t), &rx_cfg);

        // Wait for stop signal
        while (self->currentMode_ == RECORDING) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        // Cleanup
        rmt_disable(self->rx_chan_);
        self->currentMode_ = IDLE;
        vTaskDelete(NULL);
    }

    static void emission_task(void* arg) {
        IRWorker* self = static_cast<IRWorker*>(arg);
        
        // Create copy encoder
        rmt_copy_encoder_config_t enc_cfg = {};
        rmt_encoder_handle_t encoder;
        rmt_new_copy_encoder(&enc_cfg, &encoder);
        
        // Transmit configuration
        rmt_transmit_config_t tx_cfg = {
            .loop_count = 0,
            .flags = {
                .eot_level = 0
            }
        };
        
        // Transmit symbols
        rmt_transmit(self->tx_chan_, encoder, self->buffer_, 
                    self->recorded_symbols_ * sizeof(rmt_symbol_word_t), &tx_cfg);
        
        // Wait for transmission to complete
        rmt_tx_wait_all_done(self->tx_chan_, portMAX_DELAY);
        
        // Cleanup
        rmt_del_encoder(encoder);
        rmt_disable(self->tx_chan_);
        self->currentMode_ = IDLE;
        vTaskDelete(NULL);
    }

    static void beacon_task(void* arg) {
        IRWorker* self = static_cast<IRWorker*>(arg);
        
        // Calculate timing for carrier frequency
        const uint32_t half_period = 1000000 / (self->beacon_freq_ * 2); // in µs
        
        // Create symbols for one carrier cycle
        rmt_symbol_word_t symbol = {
            .duration0 = half_period,
            .level0 = 1,
            .duration1 = half_period,
            .level1 = 0
        };
        
        // Create encoder and config
        rmt_copy_encoder_config_t enc_cfg = {};
        rmt_encoder_handle_t encoder;
        rmt_new_copy_encoder(&enc_cfg, &encoder);
        
        rmt_transmit_config_t tx_cfg = {
            .loop_count = -1, // Infinite loop
            .flags = {
                .eot_level = 0
            }
        };
        
        // Start beacon transmission
        rmt_transmit(self->tx_chan_, encoder, &symbol, sizeof(symbol), &tx_cfg);
        
        // Keep task running while beacon is active
        while (self->currentMode_ == BEACON) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // Cleanup
        rmt_disable(self->tx_chan_);
        rmt_del_encoder(encoder);
        self->currentMode_ = IDLE;
        vTaskDelete(NULL);
    }

    void cleanup() {
        if (buffer_) {
            free(buffer_);
            buffer_ = NULL;
            buffer_size_ = 0;
            recorded_symbols_ = 0;
        }
        
        if (tx_chan_) {
            rmt_del_channel(tx_chan_);
            tx_chan_ = NULL;
        }
        
        if (rx_chan_) {
            rmt_disable(rx_chan_);
            rmt_del_channel(rx_chan_);
            rx_chan_ = NULL;
        }
    }
};

#endif

/*
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
            /* IDLE */      // {0,1,0,1,0,0,1,1},
            /* RECORD_PRIMED*/ //{1,0,1,0,0,0,0,0},
            /* RECORDING */   //{1,0,0,0,0,0,0,0},
            /* EMIT_PRIMED */ //{1,0,0,0,1,0,0,0},
            /* EMITTING */   // {1,0,0,0,0,0,0,0},
            /* SAVING */     // {1,0,0,0,0,0,0,0},
            /* PROCESSING */ // {1,0,0,0,0,0,0,0},
            /* BEACON */     // {1,0,0,0,0,0,0,0} 
        /*
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

*/

//frequency is in bands. 33-40 and 50-60khz, most common is the nec protocol at 38khz. 
//given this we will poll at 80khz or so to get all of the datapoints in, even if these don't perfectly match up
//most of the protocols involved in infared are PWM or amplitude modulation. to emulate this we will have a set amplitude of each value (pwm) INPUT will be recorded by 


#endif

