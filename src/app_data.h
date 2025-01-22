#ifndef APP_DATA_H_
#define APP_DATA_H_

#include "Arduino.h"

#define AMPLIFICATION_TIME_MIN          15
#define ACUTATION_TIME_MIN              5
#define DETECTION_TIME_MIN              1

#define FW_VERSION_STR                  "FW:v0.4"

/*MK Generation Calibration CONST*/
#ifdef BOARDCONFIG_MK1_1
#define SAMPLE_ZONE_AMP_SOAK_TARGET_C   68
#define VALVE_ZONE_AMP_SOAK_TARGET_C    69
#define SAMPLE_ZONE_VALVE_SOAK_TARGET_C 68
#define VALVE_ZONE_VALVE_SOAK_TARGET_C  97
#define VALVE_ZONE_MIN_VALID_TEMP_C     85
#define HEATER_SHUTDOWN_C               0
#define SLEW_RATE_LIMIT                 255
#define BUILD_HW_STR                    "HW:MK1_1 #x"
#elif defined(BOARDCONFIG_MK2)
#define SAMPLE_ZONE_AMP_SOAK_TARGET_C   68
#define VALVE_ZONE_AMP_SOAK_TARGET_C    70
#define SAMPLE_ZONE_VALVE_SOAK_TARGET_C 68
#define VALVE_ZONE_VALVE_SOAK_TARGET_C  97
#define VALVE_ZONE_MIN_VALID_TEMP_C     85
#define HEATER_SHUTDOWN_C               0
#define SLEW_RATE_LIMIT                 255
#define BUILD_HW_STR                    "HW:MK2 #x"
#elif defined(BOARDCONFIG_MK3)

#define SAMPLE_ZONE_AMP_SOAK_TARGET_C   68
#define VALVE_ZONE_AMP_SOAK_TARGET_C    70
#define SAMPLE_ZONE_VALVE_SOAK_TARGET_C 68
#define VALVE_ZONE_VALVE_SOAK_TARGET_C  97
#define VALVE_ZONE_MIN_VALID_TEMP_C     85
#define HEATER_SHUTDOWN_C               0
#define SLEW_RATE_LIMIT                 255
#define BUILD_HW_STR                    "HW:MK3 #2"

#elif defined(BOARDCONFIG_MK4)
#define SAMPLE_ZONE_AMP_SOAK_TARGET_C   68
#define VALVE_ZONE_AMP_SOAK_TARGET_C    70
#define SAMPLE_ZONE_VALVE_SOAK_TARGET_C 68
#define VALVE_ZONE_VALVE_SOAK_TARGET_C  97
#define VALVE_ZONE_MIN_VALID_TEMP_C     85
#define HEATER_SHUTDOWN_C               0
#define SLEW_RATE_LIMIT                 255
#define BUILD_HW_STR                    "HW:MK4 #x"

#else

// DEFAULT-should work with all other env builds
#define SAMPLE_ZONE_AMP_SOAK_TARGET_C   65
#define VALVE_ZONE_AMP_SOAK_TARGET_C    65
#define SAMPLE_ZONE_VALVE_SOAK_TARGET_C 65
#define VALVE_ZONE_VALVE_SOAK_TARGET_C  90
#define HEATER_SHUTDOWN_C               0
#define SLEW_RATE_LIMIT                 255
#define BUILD_HW_STR                    "HW:OTHER"
#endif
/*MK Generation Calibration CONST*/


// Timer related
#define TICK_TIMER_INTERVAL             60000L
#define PID_TIMER_INTERVAL              500L
#define TEMPERATURE_TIMER_INTERVAL      100L
#define LOGGING_TIMER_INTERVAL          1000L
#define LED_TIMER_INTERVAL              200L

#define STARTUP_DELAY_MS                5000L

#define ATTINY_8BIT_PWM_MAX             255

#define numProcess                      4  
struct CONTROL 
{
    float setpoint;
    float input;
    float output;
    float kp;
    float ki;
    float kd;

};

struct APP_DATA
{
    // structure containing application data, for passing through LOG and DEBUG interfaces
    uint8_t sample_heater_pwm_value = 0;
    uint8_t valve_heater_pwm_value = 0;
    float sample_temperature_c;
    float valve_temperature_c;
    uint8_t state = 0;
    uint8_t alarm = 0;
    volatile uint8_t time_ticker = 0;

    float battery_voltage;
    float valve_max_temperature_c;
    uint32_t valve_ramp_time;

} data;

struct flags_t {
    volatile bool flagDataCollection = false;
    volatile bool flagUpdateTemperature = false;
    volatile bool flagUpdatePID = false;
    volatile bool flagSendLog = false;
    volatile bool flagUpdateLed = false;
    volatile bool flagDelayedStart = false;
} flags;

typedef void (*irqCallback) ();
typedef struct 
  {
    irqCallback   irqCallbackFunc;
    uint32_t      TimerInterval;
    unsigned long deltaMillis;
    unsigned long previousMillis;
  } ISRTimerData;

extern APP_DATA data;

#endif /*APP_DATA_H_*/