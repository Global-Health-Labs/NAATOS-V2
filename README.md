
NAATOS-V2 development

Multiple targets underdevelopment to be handled within configuration
ATtiny1607
ATtiny1604

Mk Generation (1-4)
1: designed (not built)
1.1: 9V battery input
2: AA (1s1p)
3: AA (2s1p)
4: AA (2s1p), slim form factor

Altium Project: NAATOS-INLAY
https://global-health-laboratories-inc.365.altium.com/designs/4461EA21-563F-44E2-818D-3121326F136B


Configuration:
Mk Generation uses UPDI programming via ATMEL ICE AVR
BEWARE OF CONNECTOR PINOUT!!!!

  /platform.ini
    -build_flags
      -DBOARDCONFIG_MK1_1 {DBOARDCONFIG_MK2, DBOARDCONFIG_MK3, DBOARDCONFIG_MK4} --> PCB configuration selection (CONFIG ONLY ONE!!!)
      
