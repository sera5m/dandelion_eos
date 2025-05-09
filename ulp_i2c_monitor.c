//now normally this would be .s assembly files
//this file here allows you to read the i2c periphrials FROM the ulp, and then hold the data in memory and send it to the main chip over i2c
//pretty sure most don't use it this was, so -\_(>_<)_/-

//DO NOT COMPILE THIS IN THE MAIN CODE! THIS MUST BE COMPILED SEPERATELY AS A BIANARY .BIN BLOB AND INJECTED
/*
#include "ulp_riscv.h"
#include "ulp_riscv_i2c.h"
#include "ulp_riscv_utils.h"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define I2C_ADDR 0x68       // Example: MPU6050
#define WAKE_INTERVAL 10    // 10 x 30 sec = 5 min

uint32_t sensor_data;       // Stores I2C readings

void ulp_entry() {
    static int counter = 0;

    // Init I2C (ULP-safe)
    ulp_riscv_i2c_master_init(I2C_SDA_PIN, I2C_SCL_PIN);

    // Read from I2C (example: read 1 reg)
    uint8_t reg_data;
    ulp_riscv_i2c_master_read_reg(I2C_ADDR, 0x00, &reg_data, 1);
    sensor_data = reg_data;  // Store for main CPU

    // Wake main CPU every 5 min (10 x 30 sec)
    if (counter++ >= WAKE_INTERVAL) {
        counter = 0;
        ulp_riscv_wakeup_main_processor();
    }

    // Sleep until next 30-sec timer
    ulp_riscv_timer_stop();
    ulp_riscv_halt();
}
*/