#include <Arduino.h>

#define LED 13                      // led pin
#define BUT 23                      // button pin

volatile uint8_t poll = 0;          // polling variable
volatile uint8_t leds = 0;          // led state variable

// interrupt routine
void blink(void){
    poll = 1;                       // free the polling wait
}

void setup(){
    pinMode(LED, OUTPUT);
    pinMode(BUT, INPUT_PULLDOWN);
    
    attachInterrupt(
        digitalPinToInterrupt(BUT),
        blink,
        RISING);

    digitalWrite(LED, 1);
}

void loop(){
    while(poll == 0)                // polling wait for interrupt
        delayMicroseconds(10);
    
    delayMicroseconds(200);         // wait time in order to not read capacitive touch pulses in input

    if(digitalRead(BUT) == HIGH){
        leds = !leds;
        digitalWriteFast(LED, leds);
    }

    poll = 0;
}