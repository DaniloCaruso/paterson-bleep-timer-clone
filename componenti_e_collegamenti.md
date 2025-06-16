# Timer di Precisione Arduino - Lista Componenti e Collegamenti

## Componenti Necessari

### Componenti Principali
1. **Arduino Mini** (o Arduino Nano se preferisci USB integrato)
2. **Display 7 Segmenti TM1637** - **4 cifre ROSSO** 🔴
   - [Display TM1637 4 cifre ROSSO Amazon - Link diretto](https://amzn.eu/d/bPaVTwq)
   - **Modulo Walfront**: Display LED rosso 0,36" con chip TM1637
   - **Specifiche**: 4 cifre, tensione 3,3-5,5V, 8 livelli luminosità
   - **Prezzo**: ~€8-12
3. **LED 5mm ROSSO** 🔴
   - [LED rossi 5mm alta luminosità - 80 pezzi Amazon](https://amzn.eu/d/ic8t58G)
   - **Ka wah core**: 2.2-2.4V, 20mA, 620-625nm, angolo 360°
   - **Prezzo**: ~€8-10 per 80 pezzi (scorta per anni!)
4. **Buzzer Attivo 5V** (per suoni chiari)
   - **CONSIGLIATO 2 pin**: [ICQUANZX 10pcs 5V Buzzer - €4,89](https://www.amazon.it/buzzer-arduino/s?k=buzzer+arduino) 
   - **Se hai buzzer 3 pin**: VCC→+5V, GND→GND, Signal→Pin 5 Arduino
5. **Switch ON-OFF** (interruttore a 2 posizioni)
6. **Potenziometro 100KΩ** (per impostazione target 1-60)
7. **2x Pulsanti Tattili** (6x6mm o simili)

### Componenti Passivi
- **1x Resistore 220Ω** (per LED rosso - calcolo: (5V-2.3V)/0.02A=135Ω→220Ω sicurezza)
- **2x Resistori 10KΩ** (pull-up per pulsanti se necessario)
- **2x Condensatori 100nF (0.1µF)** - Disaccoppiamento per stabilità alimentazione
  - 1x vicino ad Arduino Mini (VCC-GND)
  - 1x vicino al display TM1637 (VCC-GND)
- **Breadboard** o PCB per prototipazione
- **Cavi di collegamento** (jumper wires)

### Alimentazione
- **Batterie 4xAA** con portabatterie (6V) + regolatore 5V
- Oppure **Power Bank USB** con cavo USB-Mini (se Arduino Nano)

## Schema Collegamenti

### Arduino Mini Pin Assignment
```
Pin 2  -> Display TM1637 CLK
Pin 3  -> Display TM1637 DIO  
Pin 4  -> LED (anodo) -> Resistore 220Ω -> GND
Pin 5  -> Buzzer (+)
Pin 6  -> Switch (centro) + Pull-up interno
Pin 7  -> Pulsante START/STOP + Pull-up interno
Pin 8  -> Pulsante RESET + Pull-up interno
Pin A0 -> Potenziometro (centrale)
VCC    -> +5V
GND    -> Ground comune
```

### Collegamento Display TM1637
```
Display TM1637    Arduino Mini
VCC       ->      5V
GND       ->      GND  
CLK       ->      Pin 2 (Digital)
DIO       ->      Pin 3 (Digital)
```

### Collegamento LED ROSSO 5mm
```
LED Anodo (gamba lunga +)  ->  Pin 4 Arduino
LED Catodo (gamba corta -)  ->  Resistore 220Ω -> GND

Specifiche LED: 2.2-2.4V, 20mA, rosso brillante 620-625nm
Resistore: 220Ω per limitare corrente a ~12mA (sicurezza)
```

### Collegamento Buzzer
```
BUZZER 2 PIN (consigliato):
Buzzer (+) ->  Pin 5 Arduino
Buzzer (-) ->  GND

BUZZER 3 PIN (se necessario):
VCC  ->  +5V Arduino
GND  ->  GND Arduino  
Signal -> Pin 5 Arduino
```

### Collegamento Switch Modalità
```
Switch Pin Centrale  ->  Pin 6 Arduino (con pull-up interno)
Switch Pin Laterale  ->  GND
Posizione HIGH = Solo Luce
Posizione LOW  = Suono + Luce
```

### Collegamento Potenziometro
```
Potenziometro 100KΩ:
Pin laterale 1  ->  +5V
Pin centrale    ->  Pin A0 Arduino  
Pin laterale 2  ->  GND
```

### Collegamento Pulsanti
```
Pulsante START/STOP:
Un pin  ->  Pin 7 Arduino (con pull-up interno)
Altro pin  ->  GND

Pulsante RESET:
Un pin  ->  Pin 8 Arduino (con pull-up interno) 
Altro pin  ->  GND
```

## Librerie Necessarie

Installa tramite Library Manager di Arduino IDE:

1. **arduino-timer** by Michael Contreras
   - Versione: ultima disponibile
   - Per timing di precisione

2. **OneButton** by Matthias Hertel  
   - Versione: ultima disponibile
   - Per gestione avanzata pulsanti

3. **TM1637Display** by Avishay Orpaz
   - Versione: ultima disponibile
   - Per controllo display 7 segmenti

## Funzionamento

### Impostazione Target
- **Potenziometro**: Ruota per impostare target da 1 a 60 secondi
- Il display a **4 cifre ROSSO** mostra il valore target formato **SS.0** quando il timer è fermo
- **Movimento potenziometro durante risultato**: Torna alla modalità impostazione

### Avvio/Stop
- **Click Pulsante START/STOP**: Avvia o ferma il timer
- **Durante l'esecuzione**: Display mostra formato **SS.D** (secondi.decimi) che **si resetta a 00.0** ad ogni intervallo raggiunto
- **Al termine**: Display mostra l'**ultimo intervallo completo** raggiunto formato **SS.0**
- **Riavvio dopo stop**: Riparte sempre con il target originale (non dal risultato mostrato)

### Reset Completo
- **Click Pulsante RESET**: Azzera tutto e riporta alle impostazioni iniziali

### Comportamento Display (4 cifre ROSSO formato SS.D)
- **Impostazione**: Mostra target (es. **05.0** per 5 secondi)
- **In esecuzione**: Conta **00.0→00.1→00.2→...→05.0→00.0** (reset) con decimi che scorrono
- **Al termine**: Mostra ultimo multiplo raggiunto (es. **10.0** se fermato a 13 sec con target 5)

### Modalità Switch
- **Posizione 1**: Solo Luce (LED blink **SOLO al raggiungimento target**)
- **Posizione 2**: Suono + Luce (LED blink + Buzzer beep ogni secondo + al target)

### Segnalazioni

#### **Modalità "Suono + Luce":**
- **Ogni secondo**: Blink/beep breve (50ms LED, 30ms buzzer)
- **Raggiungimento intervallo**: Doppio blink/beep veloce + **reset display a 00.0**

#### **Modalità "Solo Luce":**
- **Ogni secondo**: Nessun segnale (silenzioso)
- **Raggiungimento intervallo**: Solo doppio blink LED (senza buzzer) + **reset display a 00.0**
  
**Esempio con target=5**: Segnali speciali ai secondi 5, 10, 15, 20, etc.

### Esempio Completo (Target = 5 secondi) - Display ROSSO 4 cifre SS.D

#### **Modalità "Suono + Luce":**
```
🎛️ Potenziometro: Imposto target a 05.0
📺 Display ROSSO: mostra 05.0

▶️ START: Display parte da 00.0
Sec 1: Display 00.0→00.1→00.2→...→00.9→01.0 (🔴 blink + 🔊 beep al secondo)
Sec 2-4: Display 01.0→01.1→...→04.9 (🔴 blink + 🔊 beep ogni secondo)
Sec 5: Display 05.0 → 🔴🔴 DOPPIO BLINK + 🔊🔊 DOPPIO BEEP → Reset a 00.0
Sec 6-9: Display 00.0→00.1→...→04.9 (🔴 blink + 🔊 beep ogni secondo)
Sec 10: Display 05.0 → 🔴🔴 DOPPIO BLINK + 🔊🔊 DOPPIO BEEP → Reset a 00.0
```

#### **Modalità "Solo Luce":**
```
🎛️ Potenziometro: Imposto target a 05.0
📺 Display ROSSO: mostra 05.0

▶️ START: Display parte da 00.0
Sec 1-4: Display 00.0→00.1→...→04.9 (SILENZIOSO - nessun blink)
Sec 5: Display 05.0 → 🔴🔴 SOLO DOPPIO BLINK LED → Reset a 00.0
Sec 6-9: Display 00.0→00.1→...→04.9 (SILENZIOSO - nessun blink)
Sec 10: Display 05.0 → 🔴🔴 SOLO DOPPIO BLINK LED → Reset a 00.0
```

#### **Comune a entrambe le modalità:**
```
⏹️ STOP al sec 13: Display ROSSO mostra 10.0 (ultimo intervallo completo)
🎛️ Muovo potenziometro: Display torna a mostrare target attuale
▶️ START di nuovo: Riparte da 00.0 con target originale (non da 10.0)
🔄 RESET: Azzera tutto, target torna a 01.0
```

## Precisione Temporale

Il codice utilizza la libreria `arduino-timer` che garantisce:
- **Precisione**: ±1-2ms per secondo
- **Stabilità**: Non deriva nel tempo 
- **Efficienza**: Non blocca il loop principale
- **Multitasking**: Gestisce simultaneamente timer, pulsanti e display

## Note di Montaggio

1. **Connessioni saldate**: Per maggiore affidabilità
2. **Condensatori di disaccoppiamento**: 
   - **100nF tra VCC-GND di Arduino** (massimo 1-2cm dai pin)
   - **100nF tra VCC-GND del display TM1637** (il più vicino possibile)
   - **Funzione**: Eliminano rumore elettrico e stabilizzano l'alimentazione
   - **Risultato**: Prevengono reset casuali e malfunzionamenti
3. **Case protettivo**: Consigliato per uso pratico
4. **Alimentazione stabile**: Batterie alcaline o power bank di qualità

## Debug

Il codice include output seriale per monitoraggio:
- Apertura Serial Monitor a 9600 baud
- Messaggi di stato per tutte le operazioni
- Utile per verificare funzionamento e timing 