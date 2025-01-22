/*

TIMERS:
1. Temperature update --> Flag --> update struct
2. PID update --> FLAG --> use latest temperature --> update struct
3. LOG update --> Flag


*/

#include "main.h"
#if !( defined(MEGATINYCORE) )
  #error This is designed only for MEGATINYCORE megaAVR board! Please check your Tools->Board setting
#endif
#include "TMP23x.h"
#include "app_data.h"
#include "state_machine.h"
#include "alarm.h"
#include "pid.h"
#include "utils.h"

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ATtiny_TimerInterrupt.h"

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ATtiny_ISR_Timer.h"

#define SH_FIXED_PWM_TEST 128
#define VH_FIXED_PWM_TEST 255

/*CONTROL structure
  holds process steps for each STATE in application. Each [INDEX] maps to enum state_machine
*/
CONTROL sample_amp_control[numProcess] = 
{
  {HEATER_SHUTDOWN_C, 0, 0, 2, 1, .5},
  {SAMPLE_ZONE_AMP_SOAK_TARGET_C, 0,0, 29.75, 0.083,0.333},
  {SAMPLE_ZONE_VALVE_SOAK_TARGET_C, 0, 0, 29.75, 0.083,0.333},
  {HEATER_SHUTDOWN_C, 0, 0, 2, 5, 1}
};
CONTROL valve_amp_control[numProcess] = 
{
  {HEATER_SHUTDOWN_C, 0, 0, 0, 0, 0},
  {VALVE_ZONE_AMP_SOAK_TARGET_C, 0,0, 14.92, 0.083, 0.333},
  {VALVE_ZONE_VALVE_SOAK_TARGET_C,0, 0, 14.92, 0.083, 0.333},
  {HEATER_SHUTDOWN_C, 0, 0, 0, 0, 0}
};


// Temperature objects, handles sensor interface
TMP23X TMP1;
TMP23X TMP2;


volatile byte interrupt1 = 0;
uint8_t led_value = 0;



/*Callbacks, handle FLAG updates*/
void update_ticker();
void update_pid();
void update_temperature();
void send_log();
void update_led();
void update_start();
/*Timers*/
ISR_Timer ISR_Timer1;
int tickTimerNumber;
int ledTimerNumber;
int delayStartNumber;
int logISRNumber;

// tickISRTimer handles MILLIS counter used for time keeping
ISRTimerData tickISRTimer = {update_ticker,  TICK_TIMER_INTERVAL, 0,  0};

// pidISRTimerData handles timer for update rate for PID compute
ISRTimerData pidISRTimerData = {update_pid,   PID_TIMER_INTERVAL,   0,  0};

// temperatureISRTimer handles timer for update rate for temperature sensor query
ISRTimerData temperatureISRTimer = {update_temperature,   TEMPERATURE_TIMER_INTERVAL,   0,  0};

// logISRTimer handles timer for sending LOG information over UART
ISRTimerData logISRTimer = {send_log,   LOGGING_TIMER_INTERVAL,   0,  0};

ISRTimerData LedISRTimer = {update_led, LED_TIMER_INTERVAL, 0, 0};

ISRTimerData delaystartISRTimer = {update_start, STARTUP_DELAY_MS, 0 , 0};

//



void update_ticker(){
  data.time_ticker += 1;
}

void update_temperature(){
  flags.flagUpdateTemperature = true;
}

void update_pid(){
  flags.flagUpdatePID = true;
}

void send_log(){
  flags.flagSendLog = true;
}

void send_vh_max_temp(void) {
      Serial.print(F("VH_MAX: "));
      Serial.println(data.valve_max_temperature_c);
      Serial.print(F("valve_ramp_time: "));
      Serial.println(data.valve_ramp_time);
}

void update_led() {
  //digitalWrite(LED1,!digitalRead(LED1));
  flags.flagUpdateLed = true;
}

void update_start() {
  flags.flagDelayedStart = true;
}

// PID structure holder
pid_controller_t sample_zone;
pid_controller_t valve_zone;

// Reinit Controller based on STATE MACHINE
void pid_init(pid_controller_t &pid, CONTROL pid_settings){
  pid_controller_init(
    &pid, 
    pid_settings.setpoint,
    pid_settings.kp,
    pid_settings.ki,
    pid_settings.kd,
    ATTINY_8BIT_PWM_MAX,
    SLEW_RATE_LIMIT);
}

//const char startupMsg[] PROGMEM = "Startup";
const char startupMsg[] = "NAATOS_V2";

void setup() {

  // initalize STATE MACHINE
  data.state = low_power;
  data.alarm = no_alarm;

  Serial.begin(9600);
  delay(10);

  check_reset_cause();
  Serial.println(startupMsg);
  Serial.println(BUILD_HW_STR);
  Serial.println(FW_VERSION_STR);


  //INIT peripherial I/O
  pinMode(LED, OUTPUT);
  digitalWrite(LED,true);
  #ifdef LED1
    pinMode(LED1, OUTPUT);
  #endif

  pinMode(PIN_BUTTON_BUILTIN, INPUT);

  // INIT PWM I/O
  pinMode(SH_CTRL,OUTPUT);
  pinMode(VH_CTRL,OUTPUT);

  // INIT temperature sensor I/O
  TMP1.set_analog_pin(SH_ADC_PIN);
  TMP2.set_analog_pin(VH_ADC_PIN);
  TMP1.set_adc_reference();
  TMP2.set_adc_reference();

  // INIT PID Structure
  pid_init(sample_zone,sample_amp_control[data.state]);
  pid_init(valve_zone,valve_amp_control[data.state]);
  data.valve_max_temperature_c = 0;
  data.valve_ramp_time = 0;

  #if board == ATtiny1607
    // attaches PULL-UP and INTERUPT on FALLING EDGE
    PORTC.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
  #endif
  // instantiate timer oject
  CurrentTimer.init();


  tickTimerNumber = ISR_Timer1.setInterval(tickISRTimer.TimerInterval,tickISRTimer.irqCallbackFunc);  
  ISR_Timer1.setInterval(pidISRTimerData.TimerInterval,pidISRTimerData.irqCallbackFunc);  
  ISR_Timer1.setInterval(temperatureISRTimer.TimerInterval,temperatureISRTimer.irqCallbackFunc);  
  logISRNumber = ISR_Timer1.setInterval(logISRTimer.TimerInterval,logISRTimer.irqCallbackFunc);  
  //ISR_Timer1.setTimer(LedISRTimer.TimerInterval,LedISRTimer.irqCallbackFunc,25);
  delayStartNumber = ISR_Timer1.setInterval(delaystartISRTimer.TimerInterval,delaystartISRTimer.irqCallbackFunc);  
  //ISR_Timer1.setTimer(delaystartISRTimer.TimerInterval,delaystartISRTimer.irqCallbackFunc,1);
  
  ledTimerNumber = ISR_Timer1.setInterval(LedISRTimer.TimerInterval,LedISRTimer.irqCallbackFunc);

  // initally disable TICK timer
  ISR_Timer1.disable(tickTimerNumber);
}

void loop() {

    ISR_Timer1.run();

    #if board == ATtiny1607
    // button based START TIMER
    if (interrupt1) {
      // required for use with TEST BED UNITS with BUTTONS
      /*
      START TEST!
      ENABLE tickISRTimer
      */
      Serial.println("I1 fired");
      interrupt1 = 0;
      led_value = (led_value + 25) % 255;
      analogWrite(LED, led_value);

      // start TICK
      if (ISR_Timer1.isEnabled(tickTimerNumber)){
        ISR_Timer1.restartTimer(tickTimerNumber);
      } else {
        ISR_Timer1.enable(tickTimerNumber);
      }
      data.time_ticker = 0;
      tickISRTimer.previousMillis = millis();

      // start state machine
      //data.state = amplification;
    }
    #endif

    if (flags.flagDelayedStart) {
      // reset flag
      flags.flagDelayedStart = false;
      flags.flagUpdateLed = false;
      // disable TIMERs
      ISR_Timer1.disable(delayStartNumber);
      ISR_Timer1.disable(ledTimerNumber);

      // enable STATE MACHINE TIMER
      
      if (ISR_Timer1.isEnabled(tickTimerNumber)){
        ISR_Timer1.restartTimer(tickTimerNumber);
      } else {
        ISR_Timer1.enable(tickTimerNumber);
      }
    
      // Change UI (one LED is on during amplification)
      digitalWrite(LED,false);      
      digitalWrite(LED1,true);

      // Update STATE MACHINE counter for timekeeping
      data.time_ticker = 0;
      tickISRTimer.previousMillis = millis();

    }
    

    if (ISR_Timer1.isEnabled(tickTimerNumber)){
      /*ENTER STATE MACHINE*/
      // update TIME handlers
      unsigned long currentMillis  = millis();
      tickISRTimer.deltaMillis = currentMillis - tickISRTimer.previousMillis;

      if ((tickISRTimer.deltaMillis / 60000) >= AMPLIFICATION_TIME_MIN) {
        if ((tickISRTimer.deltaMillis / 60000) >= AMPLIFICATION_TIME_MIN + ACUTATION_TIME_MIN) {
          // in detection
          if (data.state != detection) {
              data.state = detection;
              pid_init(sample_zone,sample_amp_control[data.state]);
              pid_init(valve_zone,valve_amp_control[data.state]);
              // turn both LEDs off during detection
              digitalWrite(LED, true);
              digitalWrite(LED1, true);              
          }
          if ((tickISRTimer.deltaMillis / 60000)  >= AMPLIFICATION_TIME_MIN + ACUTATION_TIME_MIN + DETECTION_TIME_MIN) {
            // final state -- END
            if (data.state != low_power) {
              // SHUT DOWN LOADS!!!!
              data.state = low_power;
              pid_init(sample_zone,sample_amp_control[data.state]);
              pid_init(valve_zone,valve_amp_control[data.state]);
              ISR_Timer1.disable(tickTimerNumber);
              ISR_Timer1.disable(logISRNumber);   // stop logging

              send_vh_max_temp();
              if (data.valve_max_temperature_c < VALVE_ZONE_MIN_VALID_TEMP_C) {
                  // blink both LEDs to indicate that the minimum valve temperature was not reached.
                  data.alarm = valve_min_temp_not_reached;
              } 
              // change LEDs to opposite states so they can blink opposite of each other (in alarm)
              digitalWrite(LED, true);
              digitalWrite(LED1, false);

              // Enable the LED timer so they can blink at the end
              ISR_Timer1.enable(ledTimerNumber);
            }
          }
        } else {
          // in actuation ( 35 > TIME > 30)
          if (data.state != actuation) {
            data.state = actuation;
            pid_init(sample_zone,sample_amp_control[data.state]);
            pid_init(valve_zone,valve_amp_control[data.state]);

            // Change UI (both LEDs are on during valve activation)
            digitalWrite(LED,false);      
            digitalWrite(LED1,false);
          }
        }
        
      } else {
        // in Amplification (ie < 30)
        if (data.state != amplification) {
          data.state = amplification;
          pid_init(sample_zone,sample_amp_control[data.state]);
          pid_init(valve_zone,valve_amp_control[data.state]);
        }
      }
    }

    /*UPDATE INPUT::TEMPERATURE SENSORS*/
    if (flags.flagUpdateTemperature){
      flags.flagUpdateTemperature = false;

      // Disable the PWM outputs during the temperature and ADC measurements.
      // The current spikes on the heaters interfere with the temperature sensors.

      if (sample_zone.out < 25) {
        analogWrite(SH_CTRL,0);
        analogWrite(VH_CTRL,0);
      } else {
        analogWrite(SH_CTRL,0xFF);
        analogWrite(VH_CTRL,0xFF);
      }

      data.sample_temperature_c = TMP1.read_temperature_C();
      data.sample_temperature_c = TMP1.read_temperature_C();
      data.valve_temperature_c = TMP2.read_temperature_C();

      analogWrite(SH_CTRL,sample_zone.out);
      analogWrite(VH_CTRL,valve_zone.out);

      if (data.valve_temperature_c > data.valve_max_temperature_c) {
        data.valve_max_temperature_c = data.valve_temperature_c;
      }
      data.battery_voltage = TMP2.read_supply_voltage();

      // Measure the number of seconds it takes to ramp to the minimum valve temperature:
      if (data.state == actuation && data.valve_ramp_time == 0 && data.valve_temperature_c >= VALVE_ZONE_MIN_VALID_TEMP_C) {
        data.valve_ramp_time = tickISRTimer.deltaMillis / 1000 - (AMPLIFICATION_TIME_MIN * 60);
      }
    }

    /*UPDATE OUTPUT:HEATER LOAD*/
    if (flags.flagUpdatePID) {
      flags.flagUpdatePID = false;
      
      //compute(&sample_zone,data.sample_temperature_c);
      pid_controller_compute(&sample_zone,data.sample_temperature_c);
      pid_controller_compute(&valve_zone,data.valve_temperature_c);
    
      analogWrite(SH_CTRL,sample_zone.out);
      analogWrite(VH_CTRL,valve_zone.out);
      //analogWrite(SH_CTRL, (int) SH_FIXED_PWM_TEST);
      //analogWrite(VH_CTRL, (int) VH_FIXED_PWM_TEST);

      data.sample_heater_pwm_value = sample_zone.out;
      data.valve_heater_pwm_value = valve_zone.out;
      //data.sample_heater_pwm_value = SH_FIXED_PWM_TEST;
      //data.valve_heater_pwm_value = VH_FIXED_PWM_TEST;
    }

    /*UPDATE LED1*/
    if (flags.flagUpdateLed) {
      flags.flagUpdateLed = false;
      if (data.alarm == valve_min_temp_not_reached) {
        // blink both LEDs if there is a temperature error
        digitalWrite(LED,!digitalRead(LED));
        digitalWrite(LED1,!digitalRead(LED1));
      } else {
        // blink one LED if there are no errors
        digitalWrite(LED, true);
        digitalWrite(LED1,!digitalRead(LED1));
      }
    }

    /*UPDATE LOG::Serial COMM*/
    if (flags.flagSendLog) {
      flags.flagSendLog = false;

      Serial.print(tickISRTimer.deltaMillis);
      Serial.print(F(", "));
      Serial.print(data.sample_temperature_c);
      Serial.print(F(", "));
      Serial.print(sample_zone.setpoint);
      Serial.print(F(", "));
      Serial.print(data.sample_heater_pwm_value);
      Serial.print(F(", "));
      Serial.print(data.valve_temperature_c);
      Serial.print(F(", "));
      Serial.print(valve_zone.setpoint);
      Serial.print(F(", "));
      Serial.print(data.valve_heater_pwm_value);
      Serial.print(F(", "));
      Serial.print(data.battery_voltage);
      Serial.print(F(", "));
      Serial.println(data.state);
    }
} // LOOP

#if board == ATtiny1607
// ISR, PORTC
ISR(PORTC_PORT_vect)
{
    // clear ISR Flags
    uint8_t intflags = PORTC.INTFLAGS;
    PORTC.INTFLAGS = intflags;

    // trigger APP flags
    if (intflags & 0x10) {
      interrupt1 = 1;
    }
}
#endif