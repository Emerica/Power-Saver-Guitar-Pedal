#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

volatile boolean f_wdt=1;

#define GUITAR A3
#define POLOLU_OFF 1
#define POLOLU_CTRL A2

void setup() {
  //Set the inputs and outputs
  pinMode(GUITAR, INPUT); //Guitar input on A1 Pin7
  pinMode(POLOLU_OFF, INPUT); //Pololu Power Switch OFF BUTTON
  pinMode(POLOLU_CTRL, INPUT); //Pololu Power Switch CTRL INPUT
  //Datasheet says to set rest of pins to inputs to help lower consumption
  pinMode(0, INPUT); //Set to Lower Power Consumption
  pinMode(2, INPUT); //Set to Lower Power Consumption
  //Setup a watchdog for sleeping
  setup_watchdog();
}

void power_save(){
  cbi(ADCSRA, ADEN); //Disable the adc
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Set to the highest sleep value
  sleep_enable(); //enable sleep mode
  sleep_mode(); //enter sleep
  sleep_disable(); //disable sleep mode
  sbi(ADCSRA, ADEN); //Enable the adc again
}

void setup_watchdog() {
  byte bb;
  int ww;
  bb=9 & 7;
  if (9 > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

//This fires when the watchdog returns
ISR(WDT_vect) {
  f_wdt=1;
}

//Create Some variables
int counter = 0;
long value = 0;
int reset = 0;

void loop() {
  if (f_wdt){
    //reset the watchdog flag
    f_wdt = 0;
    if(analogRead(POLOLU_CTRL) > 10){
      if (reset){
        counter = 0;
        reset = 0;
      }
      //Get a value from the guitar, twice to have a slightly better chace at catching the high side
      if(analogRead(GUITAR) < 2){
        if(analogRead(GUITAR) < 2){
          counter++;
        }
      }else{
          counter=0;
      }
      if(counter >= 6){
        pinMode(POLOLU_OFF, OUTPUT);
        digitalWrite(POLOLU_OFF, HIGH); 
        delay(50); //Exagerated due to clock
        digitalWrite(POLOLU_OFF, LOW); 
        pinMode(POLOLU_OFF, INPUT);
        counter=0;
        reset=1;
      }
    }
    power_save();
  }
}
