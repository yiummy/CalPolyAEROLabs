#include <SPI.h>
#include "Adafruit_MAX31855.h"

// venturi pins 
const int oVHPin = 21;  // stands for oxygen venturi high pin
const int oVLPin = 20;
const int ethVHPin = 18; // eth venturi high pin 
const int ethVLPin = 19;
// chamber pins 
const int ECPin = 38; 
const int OCPin = 39; 
const int PCPin = 40; 
// load cell 
const int loadPin = 14;
// TC pins
const int MAXDO = 10; 
const int MAXCS = 11; 
const int MAXCLK = 12;

const float adcConvert = 3300/1023;  // multiply to convert adc to mV reading 
// TC object 
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Spark igniter control pins 
const int sparkInput = 16; // input reads when the button is pressed 
const int sparkOutput = 15; // output sends spark pattern to msd 
bool sparking = false; 

const long gapTime = 45000;  // ms, minimum time difference between button presses 

// values 
float oH; 
float oL; 
float eH; 
float eL; 
float load; 
float EC; 
float OC; 
float PC; 
double temp; 

// timing for readings 
long currentTime; 
long prevTime = 0; 
long pressTime; // time when the spark ignition button is pressed 


void setup() {
  // Spark Igniter signal pinmode
  pinMode(sparkInput, INPUT);
  pinMode(sparkOutput, OUTPUT);

  // initialize serial communications at 38400 bps:
  Serial.begin(38400); 
  Serial.println("oH, oL, eH, eL, EC, OC, PC, load, temp"); 

  delay(500);
  Serial.print("Initializing sensor...");
  if (!thermocouple.begin()) {
    Serial.println("ERROR.");
    while (1) {
      Serial.println("TC connection Error"); 
    };
  }

  // OPTIONAL: Can configure fault checks as desired (default is ALL)
  // Multiple checks can be logically OR'd together.
  // thermocouple.setFaultChecks(MAX31855_FAULT_OPEN | MAX31855_FAULT_SHORT_VCC);  // short to GND fault is ignored

  Serial.println("DONE.");

}

void loop() {
  //get currenttime 
  currentTime = millis(); 
  
  // read sensors   
  // only read sensors every 100 ms 
  // also check the spark input every 100 ms - change sparking state 
  if (currentTime - prevTime > 100) {
    readSensors(); 
    // start sparking if input is high and if spark was off previously 
    if (digitalRead(sparkInput) == true && !sparking) {
      sparking = true; 
      pressTime = currentTime; 
    }
    prevTime = currentTime; 
  }

  // send sparks if sparking is true for 3 seconds, cooldown is gaptime(45) seconds 
  // spark is 3 ms on, 2 ms off 

  // delay total time 
  if (sparking && currentTime - pressTime > gapTime) {
    sparking = false;
    digitalWrite(sparkOutput, LOW); 
  }

  // sparking total time 
  if (sparking && currentTime - pressTime < 3000) {
    // going to use delays anyway, will only be for 5 ms total 
    digitalWrite(sparkOutput, HIGH); 
    delay(3); 
    digitalWrite(sparkOutput, LOW); 
    delay(2); 
  }

}

void readSensors() {
  //reading sensors and printing to serial connection 

  // currently prints only the adc voltage values 
  printOH(); 
  printOL(); 
  printEH(); 
  printEL(); 
  printEC(); 
  printOC(); 
  printPC(); 
  printLoad(); 
  printTemp(); 
  
  Serial.println(); 
}

void printOH() {
  oH = analogRead(oVHPin); 
  oH = (oH-2.58)/0.279; 
  Serial.print(oH); 
  Serial.print(", "); 
}

void printOL() {
  oL = analogRead(oVLPin); 
  oL = (oL-0.258)/0.277; 
  Serial.print(oL); 
  Serial.print(", "); 
}

void printEH() {
  eH = analogRead(ethVHPin); 
  eH = (eH-16.7)/0.296; 
  Serial.print(eH); 
  Serial.print(", "); 
}

void printEL() {
  eL = analogRead(ethVLPin); 
  eL = (eL-18.4)/0.293); 
  Serial.print(eL); 
  Serial.print(", "); 
}

void printEC() {
  EC = analogRead(ECPin);
  if (EC < 100) {
    Serial.print("0.00"); 
  } else {
    EC = 3.0234*EC - 369.6448; 
    Serial.print(EC); 
  } 
  Serial.print(", "); 
}

void printOC() {
  OC = analogRead(OCPin); 
  if (OC < 100) {
    Serial.print("0.00"); 
  } else {
    OC = 3.0234*OC - 369.6448; 
    Serial.print(OC); 
  }
  Serial.print(", "); 
}

void printPC() {
  PC = analogRead(PCPin);
  if (PC < 100) {
    Serial.print("0.00"); 
  } else {
    PC = 3.0234*PC - 369.6448; 
    Serial.print(PC); 
  }
  Serial.print(", "); 
}

void printLoad() {
  load = analogRead(loadPin); 
  load = load*adcConvert/1000; 
  if (load <= 0.03) {
    load = 0; 
  } else {
    load = (load + 0.1237) / 0.0075; 
  }  
  Serial.print(load); 
  Serial.print(", "); 
}

void printTemp() {
  temp = thermocouple.readCelsius();
  Serial.print(temp); 
  Serial.print(", "); 
}



