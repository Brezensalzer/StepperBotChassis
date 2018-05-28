// StepperBotChassis.ino
//
//    Wiring
//
//    Function          Teensy LC             Beaglebone green
//    ---------------   ---------------       -----------------
//    serial            Pin 0 (RX1)           P9-13 (UART4_TXD)
//    serial            Pin 1 (TX1)           P9-11 (UART4_RXD)
//    emergency break   Pin 3 (Input, pullup) P9-15 (GPIO_48, Output)
//    DRV8834 sleep     Pin 4 (Output)        -
//    M0 Microstepping  Pin 5 (Output)        -
//    M1 Microstepping  Pin 6 (Output)        -
//    step right        Pin 7 (Output)        -
//    dir right         Pin 8 (Output)        -
//    step left         Pin 9 (Output)        -
//    dir left          Pin 10 (Output)       -

#include <AccelStepper.h>
#include <MultiStepper.h>

// sleep/enable pin
#define SLEEP_PIN 4
// microstep control for DRV8834
#define M0 5
#define M1 6
// All the wires needed for full functionality
#define STEP_RIGHT 7
#define DIR_RIGHT 8
#define STEP_LEFT 9
#define DIR_LEFT 10
#define EMERGENCY_PIN 3

// global variables
int   stepperSpeed = 4000;   // speed of the stepper (steps per second)
float wheel_dia = 61.0;      // # mm (increase = spiral out)
float wheel_base = 100.7;    // # mm (increase = spiral in) 
int   steps_rev = 3200;      // steps per revolution
long positions[2];
long steps1, steps2, delta;
int gear = 1;
   
// Serial Com
const char del = '#';     // delimiter
int  num;           // bytes received
char cmd;           // command
int  cmdFloat;      // command value as float
String command;

AccelStepper stepperLeft(AccelStepper::DRIVER, STEP_LEFT, DIR_LEFT);
AccelStepper stepperRight(AccelStepper::DRIVER, STEP_RIGHT, DIR_RIGHT);

// Up to 10 steppers can be handled as a group by MultiStepper
MultiStepper steppers;

void setup() {
  // serial com
  Serial1.begin(9600);
  Serial1.flush();
  Serial1.setTimeout(10000); 
  
  // set power on/off pin
  stepperLeft.setEnablePin(SLEEP_PIN);
  stepperRight.setEnablePin(SLEEP_PIN);

  //set target motor speed steps/s.
  stepperLeft.setMaxSpeed(stepperSpeed);
  stepperRight.setMaxSpeed(stepperSpeed);
  
  // set microstepping 16 and enable driver
  // M0 = high, M1 = high
  pinMode(M0, OUTPUT);
  digitalWrite(M0, HIGH);
  pinMode(M1, OUTPUT);
  digitalWrite(M1, HIGH);
  pinMode(SLEEP_PIN, OUTPUT);
  digitalWrite(SLEEP_PIN, HIGH);

  // Then give them to MultiStepper to manage
  steppers.addStepper(stepperLeft);
  steppers.addStepper(stepperRight);

  // energy saving
  enableOutput(false);

  // emergency stop
  pinMode(EMERGENCY_PIN, INPUT_PULLUP);
}

void loop() {
  // reset variables
  command = "";
  num = 0;
  cmd = 'X';

  // read and parse command
  command = Serial1.readStringUntil(del);
  
  if (command != "") {
    cmd = command.charAt(0);
    num = command.indexOf(del);
    command = command.substring(1,num);
    cmdFloat = command.toFloat();

    // execute command
    switch (cmd) {
      // enable output
      case 'e':
        enableOutput(true);
        break;
      // disable output
      case 'd':
        enableOutput(false);
        break;
      // forward
      case 'f':
        forward(cmdFloat);
        break;
      // backward
      case 'b':
        backward(cmdFloat);
        break;
      // turn left
      case 'l':
        left(cmdFloat);
        break;
      // turn right
      case 'r':
        right(cmdFloat);
        break;
      // set stepper speed
      case 's':
        stepperSpeed = command.toInt();
        break;
      // set wheel diameter
      case 'w':
        wheel_dia = cmdFloat;
        break;
      // set wheel track width
      case 't':
        wheel_base = command.toFloat();
        break;
      // debug info
      case 'i':
        Serial1.println("$Id$");
        Serial1.print("wheel diameter: ");
        Serial1.println(wheel_dia);
        Serial1.print("wheel_base: ");
        Serial1.println(wheel_base);
        Serial1.print("stepperSpeed: ");
        Serial1.println(stepperSpeed);
        break;
      default:
        Serial1.println("error");
        break;
      }
    Serial1.println("ok");
    Serial1.flush(); 
  }
  
}

// ----- HELPER FUNCTIONS -----------
int step(float distance){
  int steps = distance * steps_rev / (wheel_dia * 3.1412); //24.61
  return steps;  
}

void enableOutput(boolean flag){
  if(flag){
    // wakeup steppers
    stepperLeft.enableOutputs();
    stepperRight.enableOutputs();    
  }
  else{
    // energy saving
    stepperLeft.disableOutputs();
    stepperRight.disableOutputs();    
  }
}

void emergencyStop(){
  if(digitalRead(EMERGENCY_PIN) == LOW){
    stepperLeft.stop();  
    stepperRight.stop();
  }
}

void forward(float distance){
  gear = 1;
  positions[0] = step(distance);  
  positions[1] = positions[0];
  steps1 = positions[0];
  steppers.moveTo(positions);
  stepperLeft.setSpeed(stepperSpeed/4);
  stepperRight.setSpeed(-stepperSpeed/4);
  while (steps1 > 0) {
    emergencyStop();
    steps1 = stepperLeft.distanceToGo();
    steps2 = stepperRight.distanceToGo();
    // poor man's acceleration
    delta = positions[0] - steps1;
    if ((delta >= 100) && (delta < 500))
      {
        if (gear == 1) {
          stepperLeft.setSpeed(stepperSpeed/2);
          stepperRight.setSpeed(-stepperSpeed/2);
          gear = 2;
        }
       }
    else if ((delta >= 500) && (steps1 > 500))  
      {
        if (gear == 2) {
          stepperLeft.setSpeed(stepperSpeed*3/4);
          stepperRight.setSpeed(-stepperSpeed*3/4);
          gear = 3;
        }
       }
    else if ((delta >= 1000) && (steps1 > 1000))  
      {
        if (gear == 3) {
          stepperLeft.setSpeed(stepperSpeed);
          stepperRight.setSpeed(-stepperSpeed);
          gear = 4;
        }
       }
    // deceleration   
    else if ((steps1 <= 1000) && (steps1 > 500))   
      {
        if (gear == 4) {
          stepperLeft.setSpeed(stepperSpeed/2);
          stepperRight.setSpeed(-stepperSpeed/2);
          gear = 3;
        }
       }
    else if ((steps1 <= 500) && (steps1 > 100))   
      {
        if (gear == 3) {
          stepperLeft.setSpeed(stepperSpeed/2);
          stepperRight.setSpeed(-stepperSpeed/2);
          gear = 2;
        }
       }
    else if (steps1 <= 100)   
      {
        if (gear == 2) {
          stepperLeft.setSpeed(stepperSpeed/4);
          stepperRight.setSpeed(-stepperSpeed/4);
          gear = 1;
        }
       }
    steppers.run();
  }
  stepperLeft.setCurrentPosition(0);
  stepperRight.setCurrentPosition(0);
}

void backward(float distance){
  gear = 1;
  positions[1] = step(distance);  
  positions[0] = positions[1];
  steps2 = positions[1];
  steppers.moveTo(positions);
  stepperLeft.setSpeed(-stepperSpeed/4);
  stepperRight.setSpeed(stepperSpeed/4);
  while (steps2 > 0) {
    emergencyStop();
    steps1 = stepperLeft.distanceToGo();
    steps2 = stepperRight.distanceToGo();
    // poor man's acceleration
    delta = positions[1] - steps2;
    if ((delta >= 100) && (delta < 500))
      {
        if (gear == 1) {
          stepperLeft.setSpeed(-stepperSpeed/2);
          stepperRight.setSpeed(stepperSpeed/2);
          gear = 2;
        }
       }
    else if ((delta >= 500) && (steps2 > 500))  
      {
        if (gear == 2) {
          stepperLeft.setSpeed(-stepperSpeed*3/4);
          stepperRight.setSpeed(stepperSpeed*3/4);
          gear = 3;
        }
       }
    else if ((delta >= 1000) && (steps2 > 1000))  
      {
        if (gear == 3) {
          stepperLeft.setSpeed(-stepperSpeed);
          stepperRight.setSpeed(stepperSpeed);
          gear = 4;
        }
       }
    // deceleration   
    else if ((steps2 <= 1000) && (steps2 > 500))   
      {
        if (gear == 4) {
          stepperLeft.setSpeed(-stepperSpeed/2);
          stepperRight.setSpeed(stepperSpeed/2);
          gear = 3;
        }
       }
    else if ((steps2 <= 500) && (steps2 > 100))   
      {
        if (gear == 3) {
          stepperLeft.setSpeed(-stepperSpeed/2);
          stepperRight.setSpeed(stepperSpeed/2);
          gear = 2;
        }
       }
    else if (steps2 <= 100)   
      {
        if (gear == 2) {
          stepperLeft.setSpeed(-stepperSpeed/4);
          stepperRight.setSpeed(stepperSpeed/4);
          gear = 1;
        }
       }
    steppers.run();
  }
  stepperLeft.setCurrentPosition(0);
  stepperRight.setCurrentPosition(0);
}

void right(float degrees){
  float rotation = degrees / 360.0;
  float distance = wheel_base * 3.1412 * rotation;
  positions[0] = step(distance);  
  positions[1] = positions[0];
  steps1 = positions[0];
  steppers.moveTo(positions);
  stepperLeft.setSpeed(stepperSpeed/4);
  stepperRight.setSpeed(stepperSpeed/4);
  while (steps1 > 0) {
    steps1 = stepperLeft.distanceToGo();
    steps2 = stepperRight.distanceToGo();
    steppers.run();
  }
  stepperLeft.setCurrentPosition(0);
  stepperRight.setCurrentPosition(0);
}

void left(float degrees){
  float rotation = degrees / 360.0;
  float distance = wheel_base * 3.1412 * rotation;
  positions[0] = -step(distance);  
  positions[1] = positions[0];
  steps1 = positions[0];
  steppers.moveTo(positions);
  stepperLeft.setSpeed(-stepperSpeed/4);
  stepperRight.setSpeed(-stepperSpeed/4);
  while (abs(steps1) > 0) {
    steps1 = stepperLeft.distanceToGo();
    steps2 = stepperRight.distanceToGo();
    steppers.run();
  }
  stepperLeft.setCurrentPosition(0);
  stepperRight.setCurrentPosition(0);
}

