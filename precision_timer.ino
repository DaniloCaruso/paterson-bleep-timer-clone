/*
 * Timer di Precisione Arduino Mini
 * Conta secondi da 1 a 60 con decimi di secondo
 * Librerie: arduino-timer, OneButton
 * 
 * Funzionalità:
 * - Impostazione secondi 1-60 (formato SS.D)
 * - Modalità: Solo Luce / Suono + Luce
 * - Segnali ogni secondo + segnali speciali ai multipli del valore impostato
 * - Display 4 cifre: SS.D (secondi.decimi)
 */

#include <arduino-timer.h>
#include <OneButton.h>

// Pin Configuration
#define DISPLAY_CLK 2
#define DISPLAY_DIO 3
#define LED_PIN 4
#define BUZZER_PIN 5
#define MODE_SWITCH_PIN 6
#define POTENTIOMETER_PIN A0
#define START_STOP_BUTTON_PIN 7
#define RESET_BUTTON_PIN 8

// Display 7 segmenti (TM1637)
#include <TM1637Display.h>
TM1637Display display(DISPLAY_CLK, DISPLAY_DIO);

// Timer e pulsanti
Timer<10> timer;
OneButton startStopButton(START_STOP_BUTTON_PIN, true);
OneButton resetButton(RESET_BUTTON_PIN, true);

// Variabili di stato
int targetSeconds = 5;           // Secondi target (1-60)
int currentSecond = 0;           // Secondo corrente assoluto
int displaySecond = 0;           // Secondo mostrado sul display (si resetta)
int currentTenths = 0;           // Decimi di secondo correnti (0-9)
int lastCompletedInterval = 0;   // Ultimo intervallo completo raggiunto
bool isRunning = false;          // Timer attivo
bool soundEnabled = true;        // Modalità suono+luce vs solo luce
unsigned long lastSecondTime = 0;// Timestamp ultimo secondo
bool timerActive = false;        // Timer callback attivo
bool showingFinalResult = false; // Mostra risultato finale

// Variabili potenziometro
int lastPotValue = 0;            // Ultimo valore letto dal potenziometro
int potReadInterval = 100;       // Intervallo lettura potenziometro (ms)
unsigned long lastPotRead = 0;   // Timestamp ultima lettura

// Costanti timing
const int SHORT_BLINK_DURATION = 50;   // 50ms per blink breve
const int LONG_BLINK_DURATION = 200;   // 200ms per blink lungo
const int SHORT_BEEP_DURATION = 30;    // 30ms per beep breve
const int LONG_BEEP_DURATION = 150;    // 150ms per beep lungo
const int DOUBLE_BLINK_INTERVAL = 100; // Intervallo tra doppi blink

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
void updateDisplay();
void startStopButtonClick();
void resetButtonClick();
void readModeSwitch();
void readPotentiometer();

void setup() {
  Serial.begin(9600);
  
  // Inizializzazione pin
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(POTENTIOMETER_PIN, INPUT);
  
  // Inizializzazione componenti
  setupDisplay();
  setupButtons();
  
  // Lettura iniziale potenziometro
  readPotentiometer();
  
  // Stato iniziale
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  updateDisplay();
  
  Serial.println("Timer di Precisione - Pronto!");
  Serial.println("Potenziometro: Imposta target 1-60 secondi");
  Serial.println("Display formato: SS.D (secondi.decimi)");
  Serial.println("Pulsante START/STOP: Avvia/Ferma timer");
  Serial.println("Pulsante RESET: Reset completo");
}

void loop() {
  // Aggiorna timer
  timer.tick();
  
  // Aggiorna pulsanti
  startStopButton.tick();
  resetButton.tick();
  
  // Leggi potenziometro (solo se non in esecuzione o se in modalità risultato)
  if (!isRunning) {
    readPotentiometer();
  }
  
  // Leggi modalità switch
  readModeSwitch();
  
  // Aggiorna display
  updateDisplay();
  
  delay(1); // Piccolo delay per stabilità
}

void setupButtons() {
  startStopButton.attachClick(startStopButtonClick);
  resetButton.attachClick(resetButtonClick);
  
  // Imposta timing pulsanti per maggiore responsività
  startStopButton.setDebounceTicks(50);
  startStopButton.setClickTicks(300);
  
  resetButton.setDebounceTicks(50);
  resetButton.setClickTicks(300);
}

void setupDisplay() {
  display.setBrightness(0x0a); // Luminosità media
  display.clear();
}

bool timerCallback(void*) {
  if (!isRunning) return false;
  
  currentSecond++;
  displaySecond++;
  currentTenths = 0; // Reset decimi ad ogni secondo
  
  // Controllo se siamo a un multiplo del valore target
  if (currentSecond % targetSeconds == 0) {
    // Aggiorna ultimo intervallo completato
    lastCompletedInterval = currentSecond;
    
    // Reset del display counter per ripartire da zero
    displaySecond = 0;
    
    // Segnale speciale (doppio) - sempre quando raggiungiamo il target
    doubleSignal();
    Serial.print("INTERVALLO RAGGIUNTO: ");
    Serial.print(currentSecond);
    Serial.print(" (");
    Serial.print(currentSecond / targetSeconds);
    Serial.println("x target) - Display reset a 0");
  } else {
    // Segnale normale ogni secondo SOLO se suono abilitato
    if (soundEnabled) {
      shortSignal();
    }
    // Se solo luce: nessun segnale ogni secondo
  }
  
  Serial.print("Secondo assoluto: ");
  Serial.print(currentSecond);
  Serial.print(" - Display: ");
  Serial.print(displaySecond);
  Serial.print(".");
  Serial.println(currentTenths);
  
  return true; // Continua il timer
}

bool tenthsCallback(void*) {
  if (!isRunning) return false;
  
  currentTenths++;
  if (currentTenths > 9) {
    currentTenths = 0; // Sicurezza, non dovrebbe mai accadere
  }
  
  return true; // Continua il timer
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
  
  // Imposta timer per callback ogni secondo (1000ms)
  timer.every(1000, timerCallback);
  
  // Imposta timer per decimi di secondo (100ms)
  timer.every(100, tenthsCallback);
  
  Serial.println("Timer AVVIATO");
  Serial.print("Target: ");
  Serial.print(targetSeconds);
  Serial.println(" secondi");
  Serial.print("Modalità: ");
  Serial.println(soundEnabled ? "Suono + Luce" : "Solo Luce");
}

void stopTimer() {
  if (!isRunning) return;
  
  // Calcola l'ultimo intervallo completo raggiunto (multiplo più vicino in basso)
  int finalInterval = (currentSecond / targetSeconds) * targetSeconds;
  if (finalInterval == 0 && currentSecond > 0) {
    finalInterval = 0; // Se non ha mai raggiunto il primo intervallo
  }
  
  isRunning = false;
  timerActive = false;
  showingFinalResult = true;
  lastCompletedInterval = finalInterval;
  
  // Cancella tutti i timer
  timer.cancel();
  
  // Spegni LED e buzzer
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("Timer FERMATO");
  Serial.print("Tempo finale: ");
  Serial.print(currentSecond);
  Serial.print(" secondi - Ultimo intervallo completo: ");
  Serial.println(finalInterval);
  
  // Reset contatori per il prossimo uso
  currentSecond = 0;
  displaySecond = 0;
  currentTenths = 0;
}

void shortSignal() {
  // LED breve
  digitalWrite(LED_PIN, HIGH);
  timer.in(SHORT_BLINK_DURATION, [](void*) -> bool {
    digitalWrite(LED_PIN, LOW);
    return false;
  });
  
  // Suono breve (se abilitato)
  if (soundEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
    timer.in(SHORT_BEEP_DURATION, [](void*) -> bool {
      digitalWrite(BUZZER_PIN, LOW);
      return false;
    });
  }
}

void longSignal() {
  // LED lungo
  digitalWrite(LED_PIN, HIGH);
  timer.in(LONG_BLINK_DURATION, [](void*) -> bool {
    digitalWrite(LED_PIN, LOW);
    return false;
  });
  
  // Suono lungo (se abilitato)
  if (soundEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
    timer.in(LONG_BEEP_DURATION, [](void*) -> bool {
      digitalWrite(BUZZER_PIN, LOW);
      return false;
    });
  }
}

void doubleSignal() {
  // Primo segnale LED (sempre attivo al raggiungimento target)
  digitalWrite(LED_PIN, HIGH);
  if (soundEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  
  // Spegni dopo breve durata
  timer.in(SHORT_BLINK_DURATION, [](void*) -> bool {
    digitalWrite(LED_PIN, LOW);
    if (soundEnabled) {
      digitalWrite(BUZZER_PIN, LOW);
    }
    return false;
  });
  
  // Secondo segnale dopo pausa (LED sempre, buzzer solo se abilitato)
  timer.in(DOUBLE_BLINK_INTERVAL, [](void*) -> bool {
    digitalWrite(LED_PIN, HIGH);
    if (soundEnabled) {
      digitalWrite(BUZZER_PIN, HIGH);
    }
    
    // Spegni il secondo segnale
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

void updateDisplay() {
  if (showingFinalResult) {
    // Mostra l'ultimo intervallo completo raggiunto formato SS.0
    int displayValue = lastCompletedInterval * 10; // Moltiplica per 10 per formato SS.D
    display.showNumberDecEx(displayValue, 0b01000000, false, 4, 0); // Punto dopo 2° cifra
  } else if (isRunning) {
    // Mostra secondi.decimi formato SS.D durante l'esecuzione
    int displayValue = displaySecond * 10 + currentTenths;
    display.showNumberDecEx(displayValue, 0b01000000, false, 4, 0); // Punto dopo 2° cifra
  } else {
    // Mostra valore target quando fermo formato SS.0
    int displayValue = targetSeconds * 10; // Moltiplica per 10 per formato SS.D
    display.showNumberDecEx(displayValue, 0b01000000, false, 4, 0); // Punto dopo 2° cifra
  }
}

void resetButtonClick() {
  // Reset completo di tutto
  stopTimer();
  targetSeconds = 1;
  showingFinalResult = false;
  lastCompletedInterval = 0;
  
  Serial.println("RESET COMPLETO");
  Serial.println("Tutte le impostazioni azzerate");
}

void readPotentiometer() {
  // Leggi solo se è passato abbastanza tempo
  if (millis() - lastPotRead < potReadInterval) {
    return;
  }
  
  lastPotRead = millis();
  
  // Leggi valore analogico (0-1023) e converti in 1-60
  int rawValue = analogRead(POTENTIOMETER_PIN);
  int newTarget = map(rawValue, 0, 1023, 1, 60);
  
  // Solo se il valore è cambiato significativamente
  if (abs(newTarget - targetSeconds) > 0) {
    targetSeconds = newTarget;
    
    // Se stavo mostrando il risultato finale, torna alla modalità impostazione
    if (showingFinalResult) {
      showingFinalResult = false;
      Serial.print("Potenziometro mosso - Target cambiato: ");
      Serial.println(targetSeconds);
    }
  }
}

void startStopButtonClick() {
  if (isRunning) {
    stopTimer();
  } else {
    startTimer();
  }
}

void readModeSwitch() {
  static bool lastSwitchState = true;
  bool currentSwitchState = digitalRead(MODE_SWITCH_PIN);
  
  if (currentSwitchState != lastSwitchState) {
    soundEnabled = !currentSwitchState; // Switch a massa = suono disabilitato
    Serial.print("Modalità cambiata: ");
    Serial.println(soundEnabled ? "Suono + Luce" : "Solo Luce");
    lastSwitchState = currentSwitchState;
  }
} 