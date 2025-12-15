#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>


// store and manipulate the motor windings as a 2-bit counter
uint8_t step(bool dir, uint8_t* state)
{
    *state = (dir ? (*state + 1) : (*state - 1)) & 0x3;
    return *state;
}


/*
stepper has two coils A and B, each with + and - pins
pinout for phases is [A-, B-, A+, B+]
*/
uint8_t phases(uint8_t state)
{
    // make binary reflected gray code out of the 2 bit counter
    // https://en.wikipedia.org/wiki/Gray_code
    // state : 00, 01, 10, 11
    // gray  : 00, 01, 11, 10
    uint8_t gray = (state & 0x3) ^ ((state & 0x3)>>1);
    // make an inverted copy of the gray on the next two bits
    uint8_t phases = ((~gray) & 0x2)<<2 | (gray & 0x2);
    return phases;
}

// make one step in the specified direction and write it to the IO port
void step_motor(bool dir, unit8_t* state, uint8_t *port)
{
    *port = phases(step(dir, state));
}


/*
static uint8_t stepperA_state = 0; // static or global, one per motor
static uint8_t stepperB_state = 0; // static or global, one per motor

uint8_t value0 = 0;

value0 = value0 | phases(step(dir, stepperA_state)) << 4;    //upper 4 bits
value0 = value0 | phases(step(dir, stepperB_state));         //lower 4 bits


// https://docs.arduino.cc/language-reference/en/functions/advanced-io/shiftOut/
shiftOut(pins... , value0) // reg0
shiftOut(pins... , value1) // reg1
digitalWrite(shiftreg_rclk, HIGH);  // write the shifted data to the output reg
digitalWrite(shiftreg_rclk, LOW);   // ready for the next transaction



*/

#endif // MOTOR_H
