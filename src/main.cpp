#include <Arduino.h>

/*
 * Improved Arduino Clone of Paterson Bleep Timer
 * Enhanced precision timer for darkroom test strips with 7-segment display
 * 
 * Hardware:
 * - Arduino Nano 328P (https://amzn.eu/d/10km6x2)
 * - TM1637 4-digit RED 0.36" Display (https://amzn.eu/d/fvB8t1u)
 * - 5mm RED LED, Active 5V Buzzer, Switch, 100KΩ Potentiometer, 2x Buttons
 * 
 * Darkroom Usage:
 * - Set exposure interval 0-60 seconds (SS:DD format on RED 0.36" display)
 * - Two modes: Sound+Light / Light Only (for different darkroom conditions)
 * - Signals every second + special signals at interval completion (move cover cue)
 * - Reverse exposure method: start with paper covered, move cover at each signal
 * - Power: USB (power bank) or external batteries
 * 
 * Libraries: arduino-timer, OneButton, TM1637Display
 */

#include <arduino-timer.h>
#include <OneButton.h>

// Pin Configuration per Arduino Nano 328P
#define DISPLAY_CLK 2        // Pin D2 -> Display TM1637 CLK
#define DISPLAY_DIO 3        // Pin D3 -> Display TM1637 DIO
#define LED_PIN 4            // Pin D4 -> RED LED 5mm + 220Ω Resistor
#define BUZZER_PIN 5         // Pin D5 -> Active 5V Buzzer
#define MODE_SWITCH_PIN 6    // Pin D6 -> Mode switch (internal pull-up)
#define POTENTIOMETER_PIN A0 // Pin A0 -> 100KΩ Potentiometer (center tap)
#define START_STOP_BUTTON_PIN 7  // Pin D7 -> START/STOP Button (internal pull-up)
#define RESET_BUTTON_PIN 8       // Pin D8 -> RESET Button (internal pull-up)

// TM1637 4-digit RED 0.36" 7-segment display (3.3~5.5V compatible)
#include <TM1637Display.h>
TM1637Display display(DISPLAY_CLK, DISPLAY_DIO);

// Precision timer and button handlers
Timer<10> timer;
OneButton startStopButton(START_STOP_BUTTON_PIN, true);
OneButton resetButton(RESET_BUTTON_PIN, true);

// Timer state variables
int targetSeconds = 0;           // Target exposure interval (0-60 seconds)
int currentSecond = 0;           // Current absolute second counter
int displaySecond = 0;           // Second shown on RED 0.36" display (resets at intervals)
int currentTenths = 0;           // Current tenths of second (0-9)
int lastCompletedInterval = 0;   // Last completed exposure interval
bool isRunning = false;          // Timer active state
bool soundEnabled = true;        // Sound+Light vs Light Only mode
unsigned long lastSecondTime = 0;// Timestamp of last second
bool timerActive = false;        // Timer callback active state
bool showingFinalResult = false; // Display showing final result
bool displayBlinkState = false;  // Display blink state for final result
bool displayVisible = true;      // Display visibility during blink
unsigned long lastDisplayBlink = 0; // Last display blink timestamp
const int DISPLAY_BLINK_INTERVAL = 500; // Display blink interval (ms)

// Potentiometer reading variables
int lastPotValue = 0;            // Last potentiometer reading
int potReadInterval = 100;       // Potentiometer read interval (ms) - back to original for responsiveness
unsigned long lastPotRead = 0;   // Last potentiometer read timestamp

// Signal timing constants
const int SHORT_BLINK_DURATION = 50;   // 50ms for short blink
const int LONG_BLINK_DURATION = 200;   // 200ms for long blink
const int SHORT_BEEP_DURATION = 30;    // 30ms for short beep
const int LONG_BEEP_DURATION = 150;    // 150ms for long beep
const int DOUBLE_BLINK_INTERVAL = 100; // Interval between double blinks
const int DISPLAY_BLINK_DURATION = 100; // Display blink duration during signals

// Prototipi funzioni
void setupButtons();
void setupDisplay();
bool timerCallback(void*);
bool tenthsCallback(void*);
void startTimer();
void stopTimer();
void shortSignal();
void longSignal();
void doubleSignal();
void blinkDisplay();
void updateDisplay();
void startStopButtonClick();
void resetButtonClick();
void readModeSwitch();
void readPotentiometer();

void setup() {
  Serial.begin(9600);
  
  // Inizializzazione pin Arduino Nano 328P
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(POTENTIOMETER_PIN, INPUT);
  
  // Inizializzazione componenti
  setupDisplay();
  setupButtons();
  
  // Read initial potentiometer value
  readPotentiometer();
  
  // Set initial state
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  updateDisplay();
  
  Serial.println("Improved Arduino Clone of Paterson Bleep Timer - Ready!");
  Serial.println("Display: TM1637 4-digit RED 0.36\" (3.3~5.5V)");
  Serial.println("Potentiometer: Set exposure interval 0-60 seconds");
  Serial.println("Display format: SS:DD (seconds:tenths)");
  Serial.println("START/STOP Button: Start/Stop timer");
  Serial.println("RESET Button: Complete reset");
  Serial.println("Darkroom usage: Move cover at each signal for test strips");
}

void loop() {
  // Update precision timer
  timer.tick();
  
  // DIRECT BUTTON TEST - bypass OneButton library for debugging
  static bool lastPin7State = HIGH;
  static bool lastPin8State = HIGH;
  bool currentPin7 = digitalRead(START_STOP_BUTTON_PIN);
  bool currentPin8 = digitalRead(RESET_BUTTON_PIN);
  
  // Detect pin state changes directly
  if (currentPin7 != lastPin7State) {
    Serial.print("DIRECT PIN7 CHANGE: ");
    Serial.print(lastPin7State ? "HIGH" : "LOW");
    Serial.print(" -> ");
    Serial.println(currentPin7 ? "HIGH" : "LOW");
    if (currentPin7 == LOW) {
      Serial.println("*** START/STOP BUTTON PRESSED (direct detection) ***");
    }
    lastPin7State = currentPin7;
  }
  
  if (currentPin8 != lastPin8State) {
    Serial.print("DIRECT PIN8 CHANGE: ");
    Serial.print(lastPin8State ? "HIGH" : "LOW");
    Serial.print(" -> ");
    Serial.println(currentPin8 ? "HIGH" : "LOW");
    if (currentPin8 == LOW) {
      Serial.println("*** RESET BUTTON PRESSED (direct detection) ***");
    }
    lastPin8State = currentPin8;
  }
  
  // Update button states - high priority for responsiveness
  startStopButton.tick();
  resetButton.tick();
  
  // Debug button state every 2 seconds to avoid serial spam
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 2000) {
    lastDebugTime = millis();
    Serial.print("Button debug - Pin7: ");
    Serial.print(digitalRead(START_STOP_BUTTON_PIN));
    Serial.print(", Pin8: ");
    Serial.print(digitalRead(RESET_BUTTON_PIN));
    Serial.print(", Target: ");
    Serial.print(targetSeconds);
    Serial.print("s, Running: ");
    Serial.println(isRunning ? "YES" : "NO");
  }
  
  // Read potentiometer ONLY when not running (prevents interference during timer)
  if (!isRunning) {
    readPotentiometer();
  }
  
  // Handle display blinking when showing final result
  if (showingFinalResult) {
    if (millis() - lastDisplayBlink > DISPLAY_BLINK_INTERVAL) {
      displayVisible = !displayVisible;
      lastDisplayBlink = millis();
    }
  } else {
    displayVisible = true; // Always visible when not showing final result
  }
  
  // Read mode switch state
  readModeSwitch();
  
  // Update RED 0.36" display
  updateDisplay();
  
  delay(1); // Small delay for system stability
}

void setupButtons() {
  startStopButton.attachClick(startStopButtonClick);
  resetButton.attachClick(resetButtonClick);
  
  // Modern OneButton API - using Ms methods instead of deprecated Ticks
  startStopButton.setDebounceTicks(50);
  startStopButton.setClickTicks(300);
  
  resetButton.setDebounceTicks(50);
  resetButton.setClickTicks(300);
}

void setupDisplay() {
  // TM1637 4-digit RED 0.36" display - Medium brightness (8 levels available)
  display.setBrightness(0x0a); // Medium brightness for RED 0.36" display
  display.clear();
}

bool timerCallback(void*) {
  if (!isRunning) return false;
  
  currentSecond++;
  displaySecond++;
  currentTenths = 0; // Reset tenths at each second
  
  // Check if we reached a multiple of target interval (exposure complete)
  if (currentSecond % targetSeconds == 0) {
    // Update last completed exposure interval
    lastCompletedInterval = currentSecond;
    
    // Reset display counter to start from zero
    displaySecond = 0;
    
    // Special signal (double) - always when exposure interval reached
    doubleSignal();
    Serial.print("EXPOSURE INTERVAL REACHED: ");
    Serial.print(currentSecond);
    Serial.print(" seconds (");
    Serial.print(currentSecond / targetSeconds);
    Serial.println("x interval) - MOVE COVER - Display reset to 0");
  } else {
    // LED blinks every second in BOTH modes (Light Only and Sound+Light)
    shortSignal();
    // Sound differs: beep only if sound enabled, LED always blinks
  }
  
  Serial.print("Absolute second: ");
  Serial.print(currentSecond);
  Serial.print(" - RED Display 0.36\": ");
  Serial.print(displaySecond);
  Serial.print(".");
  Serial.println(currentTenths);
  
  return true; // Continue timer
}

bool tenthsCallback(void*) {
  if (!isRunning) return false;
  
  currentTenths++;
  if (currentTenths > 9) {
    currentTenths = 0; // Safety check, should never happen
  }
  
  return true; // Continue timer
}

void startTimer() {
  if (isRunning) return;
  
  isRunning = true;
  currentSecond = 0;
  displaySecond = 0;
  currentTenths = 0;
  lastCompletedInterval = 0;
  showingFinalResult = false;
  timerActive = true;
  
  // Set timer callback every second (1000ms)
  timer.every(1000, timerCallback);
  
  // Set timer callback for tenths of second (100ms)
  timer.every(100, tenthsCallback);
  
  Serial.println("DARKROOM TIMER STARTED");
  Serial.print("Exposure interval: ");
  Serial.print(targetSeconds);
  Serial.println(" seconds");
  Serial.print("Mode: ");
  Serial.println(soundEnabled ? "Sound + Light" : "Light Only");
  Serial.println("Display: TM1637 4-digit RED 0.36\" format SS:DD");
}

void stopTimer() {
  if (!isRunning) return;
  
  // Calculate last completed exposure interval (nearest multiple below)
  int finalInterval = (currentSecond / targetSeconds) * targetSeconds;
  if (finalInterval == 0 && currentSecond > 0) {
    finalInterval = 0; // Never reached first interval
  }
  
  isRunning = false;
  timerActive = false;
  showingFinalResult = true;
  lastCompletedInterval = finalInterval;
  
  // Cancel all timers
  timer.cancel();
  
  // Turn off LED and buzzer
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("DARKROOM TIMER STOPPED");
  Serial.print("Final time: ");
  Serial.print(currentSecond);
  Serial.print(" seconds - Last completed exposure: ");
  Serial.println(finalInterval);
  Serial.println("RED Display 0.36\" shows last completed exposure time");
  
  // Reset counters for next use
  currentSecond = 0;
  displaySecond = 0;
  currentTenths = 0;
}

void shortSignal() {
  // Short LED blink (progress indication) - always active
  digitalWrite(LED_PIN, HIGH);
  timer.in(SHORT_BLINK_DURATION, [](void*) -> bool {
    digitalWrite(LED_PIN, LOW);
    return false;
  });
  
  // Blink display along with LED
  blinkDisplay();
  
  // Short beep (only if sound enabled)
  if (soundEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
    timer.in(SHORT_BEEP_DURATION, [](void*) -> bool {
      digitalWrite(BUZZER_PIN, LOW);
      return false;
    });
  }
}

void longSignal() {
  // Long LED blink
  digitalWrite(LED_PIN, HIGH);
  timer.in(LONG_BLINK_DURATION, [](void*) -> bool {
    digitalWrite(LED_PIN, LOW);
    return false;
  });
  
  // Long beep (if sound enabled)
  if (soundEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
    timer.in(LONG_BEEP_DURATION, [](void*) -> bool {
      digitalWrite(BUZZER_PIN, LOW);
      return false;
    });
  }
}

void doubleSignal() {
  // First signal LED (always active when exposure interval reached)
  digitalWrite(LED_PIN, HIGH);
  if (soundEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  
  // Blink display for first signal
  blinkDisplay();
  
  // Turn off after short duration
  timer.in(SHORT_BLINK_DURATION, [](void*) -> bool {
    digitalWrite(LED_PIN, LOW);
    if (soundEnabled) {
      digitalWrite(BUZZER_PIN, LOW);
    }
    return false;
  });
  
  // Second signal after pause (LED always, buzzer only if enabled)
  timer.in(DOUBLE_BLINK_INTERVAL, [](void*) -> bool {
    digitalWrite(LED_PIN, HIGH);
    if (soundEnabled) {
      digitalWrite(BUZZER_PIN, HIGH);
    }
    
    // Blink display for second signal (delayed)
    timer.in(10, [](void*) -> bool {
      blinkDisplay();
      return false;
    });
    
    // Turn off second signal
    timer.in(SHORT_BLINK_DURATION, [](void*) -> bool {
      digitalWrite(LED_PIN, LOW);
      if (soundEnabled) {
        digitalWrite(BUZZER_PIN, LOW);
      }
      return false;
    });
    
    return false;
  });
}

void blinkDisplay() {
  // Turn off display briefly for blink effect
  display.clear();
  timer.in(DISPLAY_BLINK_DURATION, [](void*) -> bool {
    // Display will be updated normally in the main loop
    return false;
  });
}

void updateDisplay() {
  // TM1637 4-digit RED 0.36" display format SS:DD (seconds:tenths)
  int displaySeconds = 0;
  int displayTenths = 0;
  
  if (showingFinalResult) {
    // Show last completed exposure interval format SS:00 (blinking)
    if (displayVisible) { // Only show when visible (for blinking effect)
      displaySeconds = lastCompletedInterval;
      displayTenths = 0;
    } else {
      // Turn off display during blink
      display.clear();
      return;
    }
  } else if (isRunning) {
    // Show current seconds:tenths format SS:DD during execution
    displaySeconds = displaySecond;
    displayTenths = currentTenths;
  } else {
    // Show target value when stopped format SS:00
    displaySeconds = targetSeconds;
    displayTenths = 0;
  }
  
  // Create 4-digit display value: SSDD (but we'll show SS:DD)
  int displayValue = displaySeconds * 100 + displayTenths * 10;
  
  // Show with colon separator (0b01000000 = colon between 2nd and 3rd digit)
  display.showNumberDecEx(displayValue, 0b01000000, true, 4, 0);
}

void resetButtonClick() {
  Serial.println("RESET BUTTON PRESSED!");
  // Complete reset of all settings
  stopTimer();
  showingFinalResult = false;
  displayVisible = true;
  lastCompletedInterval = 0;
  
  // Force potentiometer read to get current setting
  readPotentiometer();
  
  Serial.println("COMPLETE RESET");
  Serial.println("All settings cleared - reading potentiometer");
  Serial.println("RED Display 0.36\" back to setting mode");
}

void readPotentiometer() {
  // Read only if enough time has passed
  if (millis() - lastPotRead < potReadInterval) {
    return;
  }
  
  lastPotRead = millis();
  
  // Read multiple times and average for stability (reduced from 5 to 3)
  int sum = 0;
  for (int i = 0; i < 3; i++) {
    sum += analogRead(POTENTIOMETER_PIN);
    delay(1); // Small delay between reads
  }
  int rawValue = sum / 3; // Average of 3 readings
  
  int newTarget = map(rawValue, 0, 1023, 0, 60);
  
  // Minimal hysteresis: allow gradual changes (0->1->2->3, not 0->7)
  if (abs(newTarget - targetSeconds) > 0) {  // Back to 0 for gradual changes
    targetSeconds = newTarget;
    
    // If showing final result, return to setting mode
    if (showingFinalResult) {
      showingFinalResult = false;
      Serial.print("Potentiometer moved - Exposure interval changed: ");
      Serial.print(targetSeconds);
      Serial.println(" seconds - RED Display back to setting mode");
    }
  }
}

void startStopButtonClick() {
  Serial.println("START/STOP BUTTON PRESSED!");
  
  if (showingFinalResult) {
    // If showing final result, return to normal setting mode
    showingFinalResult = false;
    displayVisible = true;
    // Force potentiometer read to get current setting
    readPotentiometer();
    Serial.println("Returned to setting mode - reading potentiometer");
    return;
  }
  
  if (isRunning) {
    Serial.println("Timer running - STOPPING timer");
    stopTimer();
  } else {
    Serial.println("Timer stopped - STARTING timer");
    startTimer();
  }
}

void readModeSwitch() {
  static bool lastSwitchState = true;
  bool currentSwitchState = digitalRead(MODE_SWITCH_PIN);
  
  if (currentSwitchState != lastSwitchState) {
    soundEnabled = !currentSwitchState; // Switch to ground = sound disabled
    Serial.print("Darkroom mode changed: ");
  Serial.println(soundEnabled ? "Sound + Light" : "Light Only");
    lastSwitchState = currentSwitchState;
  }
} 