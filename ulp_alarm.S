
/* ULP-RISC-V Assembly (ulp_alarm.S) */
#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

.set ALARM_COUNT_OFFSET, 0
.set ALARM_DATA_OFFSET, 1

.global entry
entry:
    /* Read current minute (5-min precision) */
    rtc_get_minute r0
    lsr r0, r0, 2       // r0 = minute/4 (approx 5min)
    add r0, r0, 1       // Adjust for rounding

    /* Load alarm count */
    move r1, ALARM_COUNT_OFFSET
    ld r2, r1, 0

    /* Alarm check loop */
    move r3, ALARM_DATA_OFFSET
loop:
    jump exit, eq r2, 0  // No more alarms

    /* Load alarm minute */
    ld r4, r3, 2         // Offset 2: minute field
    and r4, r4, 0x3F     // Mask 6 bits

    /* Compare minutes */
    jump next_alarm, ne r0, r4

    /* MATCH FOUND - Trigger actions */
    ld r4, r3, 3         // Load actions bitmask
    and r4, r4, 0x07     // Mask 3 bits
    
    /* Set GPIO based on actions */
    and r5, r4, 0x1      // LED check
    jump no_led, eq r5, 0
    WRITE_RTC_REG(OUT_REG, 1<<LED_PIN, 1, 1)
no_led:
    /* Repeat for other actions */

    /* Wake main CPU */
    wake
    WRITE_RTC_REG(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLEEP_CYC0_S, 1, 1)

next_alarm:
    sub r2, r2, 1
    add r3, r3, 4        // Next alarm (4-byte aligned)
    jump loop

exit:
    halt