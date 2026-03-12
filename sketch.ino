/*
OLD LCD Setup Code, only used for WOKWI sims
#include <LiquidCrystal.h>
// LCD Pins: RS=12, E=11, D4=7, D5=6, D6=5, D7=4
LiquidCrystal lcd(13, 12, 7, 6, 5, 4);
*/

// TO DO: Test, Fix overflow fault glitch/error (probable causes: sensor reading functions calibrated incorrectly/inconsistently formatted i.e. 100 - percentage value, not mapped to a percentage and just reading a dist value, epsilons improperly calibrated, etc.), make sure the changes to amirs sensor reading functions didnt cause syntax/compilation issues, calibrate code for physical dimensions provided

// 41.4 (OUT), 39.0 (IN)

// ---------------------------------------------------------
// 1. LCD SETUP
// ---------------------------------------------------------
#include <LiquidCrystal.h>
// LCD Pins: RS=12, E=11, D4=7, D5=6, D6=5, D7=4
LiquidCrystal lcd(13, 12, 7, 6, 5, 4);


// ---------------------------------------------------------
// 2. PIN DEFINITIONS
// ---------------------------------------------------------
// Ultrasonic Sensor 1 (Filtered Water Tank)
const int trig1 = 3;
const int echo1 = 2;

// Ultrasonic Sensor 2 (Output Tank)
// NOTE: Pins 0 and 1 are used for RX/TX (Serial communication) on standard Arduinos.
const int trig2 = 1; 
const int echo2 = 0;

// Analog Sensors
const int phSens1 = A0; // Filtered Tank pH
const int phSens2 = A1; // Output Tank pH

// Actuators (Outputs)
const int pump = 8;
const int spigot = 9;

// User Inputs
const int switchPin = A4;
const int displayPin = A5;

// Epsilon creates a buffer to account for sensor noise and water ripples.
const int epsilon = 5; // 5% margin of error

// 100% is physically empty, but we treat it as empty at 95% to protect the pump.
const int tankEmpty = 100 - epsilon; 

// 0% is physically full, but we treat it as full at 5% to prevent overflowing.
const int tankFull = 0 + epsilon;

// depth of tank in cm
const long tankDepth = 22.86;

//water level has to be 2x epsilon to start filling
const int fillThreshold = 100 - epsilon * 2;


// ---------------------------------------------------------
// 3. STATE MACHINE DEFINITIONS
// ---------------------------------------------------------
enum class States {
  IDLE,
  FILLING,
  READY,
  DISPENSE,
  FAULT
};


// ---------------------------------------------------------
// 3.5. FAULT STATES DEFINITIONS
// ---------------------------------------------------------
enum class Faults {
  NO_FAULT,// No fault (yay!)
  OVERFLOW,// When either tank passes over the epsilon buffer + runOffPercent
  INPUT_EMPTY,// When Tank 1 is empty to prevent dry pumping
  ACIDIC, // When the water in Tank 2 is too acidic
  BASIC, // When the water in Tank 2 is too basic
  IN_SENSOR, // When the sensor above the input tank is bugged out
  OUT_SENSOR // When the sensor above the output tank is bugged out
};

States tankState; // Tracks the current active mode of the system
Faults faultState;


// ---------------------------------------------------------
// 4. SYSTEM TRACKING VARIABLES
// ---------------------------------------------------------
boolean pumpRunning = false;    // Tracks if the pump is currently actively running
boolean dispensing = false;     // Tracks if the spigot is currently open
boolean right = false;          // Used to toggle the LCD display between tanks
boolean lastButtonState = LOW;  // Tracks the previous state of the spigot switch
boolean lastDisplayButtonState = LOW; // Tracks the previous state of the display switch
boolean currentDisplayButtonState = LOW;
boolean currentButtonState = LOW;
long acidic = 4;
long basic = 9;
long previousTime = 0;          //keep track of time
long intervel = 10000;          // the time it will check if tank 1 waterlevel change and there is some water in it
int lastD1WaterLevel = 0;       // Tracks if the tank1 waterlevel changes so no water is left behind
int lastD2WaterLevel = 100;     // Used to track the previous waterlevel to see if tank2 is dispensing

void setup() { 
  // Boot LCD
  lcd.begin(16, 2);
  
  // Initialize Ultrasonic Sensor Pins
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
  
  // Initialize Actuator Pins (Pump and Spigot)
  pinMode(pump, OUTPUT);
  pinMode(spigot, OUTPUT); 
  
  // Initialize pH Sensors and Switches (Inputs)
  pinMode(phSens1, INPUT);
  pinMode(phSens2, INPUT);
  pinMode(switchPin, INPUT); 
  pinMode(displayPin, INPUT); 

  // Start in a safe state and let the loop() logic decide what to do
  tankState = States::IDLE;
  faultState = Faults::NO_FAULT;
  
  // Ensure hardware is explicitly turned off at boot for safety
  digitalWrite(pump, LOW);
  digitalWrite(spigot, LOW);
}


void loop() { // runs forever

  // ---------------------------------------------------------
  // 1. SENSOR & INPUT READING
  // ---------------------------------------------------------
  long ph1 = readPHSensor(phSens1);
  long ph2 = readPHSensor(phSens2);

  int d1 = getDist(trig1, echo1); // Filtered Tank (100% = empty, 0% = full)
  int d2 = getDist(trig2, echo2); // Output Tank (100% = empty, 0% = full)

  int sens1 = checkSensor(trig1, echo1); // Fault detection variable for tank 1 sensor
  int sens2 = checkSensor(trig2, echo2); // Fault detection variable for tank 2 sensor

  // --- Display Button Debounce & Edge Detection ---
  currentButtonState = analogRead(switchPin) >= 1000;
  
  if (tankState == States::FAULT) {
    // Clear fault manually with a button press (edge detection so it only clears once)
    if (currentButtonState == HIGH && lastButtonState == LOW) {
      lcd.clear();
      tankState = States::IDLE;
      faultState = Faults::NO_FAULT;
    }
    dispensing = false; // Never dispense while in fault
  } else {
    // Momentary dispense: Only true while the button is held AND there is water
    if (currentButtonState == HIGH && lastButtonState == LOW && d2 < tankEmpty) {
      dispensing = !dispensing;
    }
  }
  lastButtonState = currentButtonState;

  // --- Display Button Debounce & Edge Detection ---
  currentDisplayButtonState = analogRead(displayPin) >= 1000;
  
  if (currentDisplayButtonState == HIGH && lastDisplayButtonState == LOW) {
    right = !right; // Toggle the display state between Left (false) and Right (true)
    lcd.clear();    // Clear the screen to prevent leftover characters
  }
  lastDisplayButtonState = currentDisplayButtonState;
  // Only for no dispense button
  /*
  int currentD2WaterLevel = d2;
  if (currentD2WaterLevel>lastD2WaterLevel){
    dispensing = true;
  } else {
    dispensing = false;
  }
  lastD2WaterLevel = currentD2WaterLevel;
*/
  
  // ---------------------------------------------------------
  // 2. STATE MACHINE LOGIC
  // ---------------------------------------------------------
  switch (tankState) {
    
    case States::IDLE:
      // ACTIONS: Everything off
      delay(500);
      digitalWrite(pump, LOW);
      pumpRunning = false;
      digitalWrite(spigot, LOW);
      dispensing = false;

      // TRANSITIONS:
      if (d1 < fillThreshold && d2 >= tankEmpty) { 
        tankState = States::FILLING; // Filtered has water, Output is empty
      } else if (d2 < tankEmpty) {
        tankState = States::READY;   // Output somehow got water, go to ready
      }
      break;

    case States::FILLING:
      // ACTIONS: Pump on, Spigot off
      digitalWrite(pump, HIGH);
      pumpRunning = true;
      digitalWrite(spigot, LOW);

      // TRANSITIONS:
      if (d2 <= tankFull) {
        tankState = States::READY; // Output tank is full (0%)
      } else if (d1 >= tankEmpty) {
        tankState = States::IDLE;  // Filtered tank ran out before output filled
      } else if (d2 < tankEmpty) {
        tankState = States::READY; // Output tank has enough water to be ready
      }
      break;

    case States::READY:
      // ACTIONS: Spigot off. Pump depends on Filtered tank level
      digitalWrite(spigot, LOW);
      
      // Ready(2) -> Filtered has water, keep topping up Output (unless Output is full)
      if (d1 < tankEmpty && d2 > tankFull) {
        digitalWrite(pump, HIGH);
        pumpRunning = true;
      } 
      // Ready(1) -> Filtered is empty OR Output is totally full
      else {
        delay(500);
        digitalWrite(pump, LOW);
        pumpRunning = false;
      }

      // TRANSITIONS:
      if (dispensing) {
        tankState = States::DISPENSE;
      } else if (d2 >= tankEmpty && d1 < tankEmpty) {
        tankState = States::FILLING; // Output empty, Filtered has water
      } else if (d2 >= tankEmpty && d1 >= tankEmpty) {
        tankState = States::IDLE;    // Both empty
      }
      break;

    case States::DISPENSE:
      // ACTIONS: Spigot Open. Pump depends on Filtered tank (same as Ready)
      digitalWrite(spigot, HIGH);
      
      if (d1 < tankEmpty && d2 > tankFull) {
        digitalWrite(pump, HIGH);
        pumpRunning = true;
      } else {
        digitalWrite(pump, LOW);
        pumpRunning = false;
      }

      // TRANSITIONS:
      if (!dispensing) {
        // User let go of switch OR tank ran empty (handled by input logic)
        if (d2 < tankEmpty) tankState = States::READY;
        else if (d1 < tankEmpty) tankState = States::FILLING;
        else tankState = States::IDLE;
      }
      break;

    case States::FAULT:
      // ACTIONS: Safety mode - everything forced off
      digitalWrite(pump, LOW);
      pumpRunning = false;
      digitalWrite(spigot, LOW);
      dispensing = false;

      // TRANSITIONS:
      // Handled by the button press logic at the top of the loop
      break;
  }

  // ---------------------------------------------------------
  // 2.5 --- Global Safety & Fault Triggers ---
  // ---------------------------------------------------------
  if (tankState != States::FAULT) {
    
    // CASE 1: Imminent Overflow. 
    // If either tank reads runOffPercent or less, it breached the 5% safety buffer!
    if (d1 <= tankFull || d2 <= tankFull) {
      tankState = States::FAULT;
      faultState = Faults::OVERFLOW;
    }
    
    // CASE 2: Dry-Run Protection.
    // If the supply tank is totally empty but the state machine is trying to pump
    else if (d1 >= 100 && pumpRunning == true) {
      tankState = States::FAULT;
      faultState = Faults::INPUT_EMPTY;
    }

    // CASE 3: Unfiltered Filtered Water
    // If the water in the output tank is outside of the acceptable PH range, either too acidic or too basic respectively
    else if (ph2<acidic){
      tankState = States::FAULT;
      faultState = Faults::ACIDIC;
    }
    else if (ph2>basic){
      tankState = States::FAULT;
      faultState = Faults::BASIC;
    }

    // CASE 4: Incorrect Sensor Readings
    // The ultrasonic sensor has a tendency to read things other than the water level, returning some value between 19.0 and 22.0 in all of these cases
    else if (sens1 > 19.0 && sens1 < 22.0) {
      tankState = States::FAULT;
      faultState = Faults::IN_SENSOR;
    }
    else if (sens2 > 19.0 && sens2 < 22.0) {
      tankState = States::FAULT;
      faultState = Faults::OUT_SENSOR;
    }
  }
  // ---------------------------------------------------------
  // 3. LCD DISPLAY LOGIC
  // ---------------------------------------------------------
  lcd.setCursor(0, 0);
  if(right) {
    lcd.setCursor(0, 0);
    lcd.print("IN: "); lcd.print(d1); lcd.print("%   PH: "); lcd.print(readPHSensor(phSens1)); lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(tankStateToString(tankState));
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("OUT: "); lcd.print(d2); lcd.print("%  PH: "); lcd.print(readPHSensor(phSens2)); lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(tankStateToString(tankState));
  }

  if(tankState != States::FAULT) {
    lcd.print("P: ");
    if(pumpRunning) {
      lcd.print("ON ");
    } else {
      lcd.print("OFF");
    }
  }
  else {
    lcd.print(faultStateToString(faultState));
  }
  
  // ---------------------------------------------------------
  // 4. Clear Input Tank water
  // ---------------------------------------------------------
  long currentTime = millis();
   if (currentTime - previousTime >= intervel) {
      if(d1==lastD1WaterLevel && d1!=100){
        tankState = States::FILLING;
        previousTime = currentTime;
      } else {
        lastD1WaterLevel = d1;
        previousTime = currentTime;
      }
   }
}

long checkSensor(int t, int e) {
    digitalWrite(t, LOW);
    delayMicroseconds(2);
    digitalWrite(t, HIGH);
    delayMicroseconds(10);
    digitalWrite(t, LOW);
    
    // Add a 30,000 microsecond timeout to pulseIn! 
    long distance = pulseIn(e, HIGH, 30000) * .034 / 2; 
    delay(2); // Reduced from 10 to make buttons snappier
    return distance;
}

long getDist(int t, int e) { 
  digitalWrite(t, LOW);
  delayMicroseconds(2);
  digitalWrite(t, HIGH);
  delayMicroseconds(10);
  digitalWrite(t, LOW);
  
  long duration = (pulseIn(e, HIGH, 30000)* .034 / 2); 
  return duration;
  }

  long formatDist1(long dist) {
    return map(dist, 19.0, 39.0, 0.0, 100.0);
  }

  long formatDist2(long dist) {
    return map(dist, 21.4, 41.4, 0.0, 100.0);
  }

  int averageDistance = totalDistance / numSamples;
  int percentage = map(averageDistance, 0, tankDepth, 0, 100); 
  return constrain(percentage, 0, 100);
}

long readPHSensor(int pin) { 
  int numSamples = 40; // Reduced from 5 for faster response
  long totalReading = 0;

  for (int i = 0; i < numSamples; i++) {
    totalReading += analogRead(pin);
    delay(2); // Reduced from 10 to make buttons snappier
  }

  long averageReading = totalReading / numSamples;
  long phValue = map(averageReading, 0.0, 1023.0, 0.0, 14.0); 
  return constrain(phValue, 0.0, 14.0); 
}

const char* tankStateToString(States tankState) {
  switch(tankState) {
    case States::IDLE: return "IDLE      ";
    case States::FAULT: return "FAULT ";
    case States::READY: return "READY     ";
    case States::DISPENSE: return "DISPENSE  ";
    case States::FILLING: return "FILLING   ";
  }
}

const char* faultStateToString(Faults faultState) {
  switch(faultState) {
    case Faults::OVERFLOW: return "OVERFLOW";
    case Faults::INPUT_EMPTY: return "IN EMPTY";
    case Faults::ACIDIC: return "OUT ACIDIC";
    case Faults::BASIC: return "OUT BASIC";
    case Faults::IN_SENSOR: return "IN SENSOR";
    case Faults::OUT_SENSOR: return "OUT SENSOR";
  }
}
