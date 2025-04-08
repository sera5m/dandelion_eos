#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

/* Memory offsets (in bytes) */
.set ALARM_COUNT_OFFSET, 0
.set ALARM_DATA_OFFSET, 4
.set TIME_OFFSET, 128

/* Entry point */
.global entry
entry:
    /* Read current minute from RTC (stored by main CPU) */
    move r0, TIME_OFFSET
    ld r1, r0, 0          /* Load current time (32-bit unix timestamp) */
    
    /* Convert to local minutes (simplified) */
    move r0, 60
    div r2, r1, r0        /* r2 = total minutes */
    move r0, 1440
    rem r3, r2, r0        /* r3 = minutes today */
    move r0, 60
    div r1, r3, r0        /* r1 = current hour */
    rem r2, r3, r0        /* r2 = current minute */
    
    /* Round to 5-minute intervals */
    move r0, 5
    div r2, r2, r0
    mul r2, r2, r0        /* r2 = floored to nearest 5 min */

    /* Load alarm count */
    move r0, ALARM_COUNT_OFFSET
    ld r3, r0, 0          /* r3 = alarm count */

    /* Alarm check loop */
    move r0, ALARM_DATA_OFFSET
.alarm_loop:
    jump .exit, eq r3, 0  /* Exit if no more alarms */

    /* Load alarm time */
    ld r1, r0, 0          /* Hour (byte 0) */
    ld r2, r0, 1          /* Minute (byte 1) */
    
    /* Compare with current time */
    move r4, TIME_OFFSET
    ld r4, r4, 0          /* Reload current time */
    /* ... time comparison logic ... */
    
    /* Match found? */
    jump .trigger, eq     /* Custom condition check */

.next_alarm:
    sub r3, r3, 1         /* Decrement counter */
    add r0, r0, 4         /* Next alarm (4-byte aligned) */
    jump .alarm_loop

.trigger:
    /* Load actions */
    ld r1, r0, 3          /* Actions byte */
    
    /* Trigger GPIO if needed */
    and r2, r1, 0x1       /* LED bit */
    jump .no_led, eq r2, 0
    WRITE_RTC_REG(RTC_GPIO_OUT_REG, 1<<LED_GPIO, 1, 1)
.no_led:
    
    /* Wake main CPU */
    wake
    halt

.exit:
    halt