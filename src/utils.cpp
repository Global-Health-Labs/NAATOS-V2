#include "utils.h"
#include "Arduino.h"

void check_reset_cause(){
    uint8_t reset_flags = RSTCTRL.RSTFR;
    Serial.println("Reset cause:");
    Serial.print(reset_flags, 16);    
    Serial.println("");
    /*
    if (reset_flags & RSTCTRL_UPDIRF_bm) {
        Serial.println("Reset by UPDI (code just upoloaded now)");
    }
    if (reset_flags & RSTCTRL_WDRF_bm) {
        Serial.println("reset by WDT timeout");
    }
    if (reset_flags & RSTCTRL_SWRF_bm) {
        Serial.println("reset at request of user code.");
    }
    if (reset_flags & RSTCTRL_EXTRF_bm) {
        Serial.println("Reset because reset pin brought low");
    }
    if (reset_flags & RSTCTRL_BORF_bm) {
        Serial.println("Reset by voltage brownout");
    }
    if (reset_flags & RSTCTRL_PORF_bm) {
        Serial.println("Reset by power on");
    }
    */    
}