# Improved Arduino Clone of Paterson Bleep Timer ⏱️

Enhanced precision timer for darkroom test strips with audio/visual signals. Used to create test strips by starting with paper completely covered and moving the cover at each beep/light signal (reverse of traditional method). Works with enlarger on, providing precise timing intervals for optimal exposure testing.

## 📋 Required Components

### Main Components
- **Arduino Nano 328P** - [Amazon](https://amzn.eu/d/10km6x2)
- **TM1637 4-digit RED 0.36" Display** - [Amazon](https://amzn.eu/d/fvB8t1u)
- **5mm RED LED**
- **Active 5V Buzzer**
- **ON-OFF Switch** (2 positions)
- **100KΩ Potentiometer**
- **2x Tactile Buttons** (6x6mm)

### Passive Components
- 1x 220Ω Resistor (for LED)
- 2x 10KΩ Resistors (button pull-ups)
- 2x 100nF Capacitors (decoupling)
- Breadboard or PCB
- Jumper wires

## 🔌 Wiring Diagram

### Arduino Nano 328P Pins
```
Pin D2  → TM1637 Display CLK
Pin D3  → TM1637 Display DIO
Pin D4  → LED Anode → 220Ω Resistor → GND
Pin D5  → Buzzer (+)
Pin D6  → Switch (center) + Internal pull-up
Pin D7  → START/STOP Button + Internal pull-up
Pin D8  → RESET Button + Internal pull-up
Pin A0  → Potentiometer (center)
5V      → Components power supply
GND     → Common ground
```

### TM1637 Display
```
VCC → Arduino 5V
GND → Arduino GND
CLK → Pin D2
DIO → Pin D3
```

### LED and Buzzer
```
LED: Anode → Pin D4 → 220Ω Resistor → GND
Buzzer: (+) → Pin D5, (-) → GND
```

### Controls
```
Potentiometer: Side1 → 5V, Center → A0, Side2 → GND
Switch: Center → Pin D6, Side → GND
START/STOP Button: Pin D7 ↔ GND
RESET Button: Pin D8 ↔ GND
```

## 🔧 Software Installation

### Required Libraries
Automatically configured in `platformio.ini`:

1. **contrem/arduino-timer@^3.0.1** - Non-blocking precision timer
2. **mathertel/OneButton@^2.6.1** - Advanced button handling with debounce
3. **smougenot/TM1637** - TM1637 7-segment display control

### PlatformIO Configuration
```ini
[env:ATmega328P]
platform = atmelavr
board = ATmega328P
framework = arduino
lib_deps = 
	smougenot/TM1637
	mathertel/OneButton@^2.6.1
	contrem/arduino-timer@^3.0.1
```

### Code Upload
1. Connect Arduino Nano via USB
2. Open project in PlatformIO
3. Compile and upload with `platformio run --target upload`
4. Or use Upload button in PlatformIO IDE

## ⚡ Power Supply

- **Recommended**: USB Power Bank (direct connection to Nano)
- **Alternative**: 4xAA Batteries + 5V regulator

## 🎮 Operation

### Darkroom Test Strip Method
This timer implements the **reverse exposure method**: start with photographic paper completely covered, then move the cover to expose a new strip section at each signal. This allows precise test strips with enlarger constantly on.

### Target Setting
- **Potentiometer**: Rotate to set exposure interval from 1 to 60 seconds
- **Display**: Shows target in `SS.0` format (e.g., `05.0` for 5-second intervals)

### Controls
- **START/STOP**: Start or stop the timer
- **RESET**: Reset everything to initial settings
- **Switch**: Change between Sound+Light / Light Only modes (for different darkroom conditions)

### Operating Modes

#### "Sound + Light" Mode (Standard Darkroom)
- LED blink + Beep every second (progress indication)
- Double blink/beep when exposure interval reached (move cover signal)
- Display resets to `00.0` at each interval

#### "Light Only" Mode (Silent Darkroom)
- Silent during counting (no disturbance)
- Only double LED blink when exposure interval reached (move cover signal)
- Display resets to `00.0` at each interval

### Display (SS.D Format)
- **Setting**: Shows target (e.g., `05.0`)
- **Running**: Counts `00.0→00.1→...→05.0→00.0` (reset)
- **Result**: Shows last complete interval reached

## 📖 Darkroom Usage Example

**Test Strip Creation - 5-second intervals**

```
🎛️ Set exposure interval: 05.0 seconds
📺 Display: 05.0

🖼️ SETUP: Place paper under enlarger, completely covered
💡 Turn enlarger ON
▶️ START timer

Sec 1-4: 00.0→01.0→02.0→03.0→04.0 (+ progress signals if sound active)
Sec 5: 05.0 → 🔴🔴 MOVE COVER → Reset to 00.0 (1st strip: 5s exposure)
Sec 6-9: 00.0→01.0→02.0→03.0→04.0 
Sec 10: 05.0 → 🔴🔴 MOVE COVER → Reset to 00.0 (2nd strip: 10s total)
Sec 11-14: Continue...
Sec 15: 05.0 → 🔴🔴 MOVE COVER → Reset to 00.0 (3rd strip: 15s total)

⏹️ STOP at sec 13 → Display shows: 10.0 (last completed exposure time)

Result: Test strip with sections exposed for 5s, 10s, 15s... intervals
```

## 🔍 Technical Specifications

### Precision
- **Timing**: ±1-2ms per second
- **Stability**: No drift over time
- **Multitasking**: Simultaneous handling of timer, buttons, and display

### TM1637 Display
- **Size**: 0.36" (9.1mm digit height)
- **Color**: High visibility red
- **Brightness**: 8 adjustable levels
- **Power**: 3.3-5.5V compatible

### Arduino Nano 328P
- **Processor**: ATmega328P 16MHz
- **Memory**: 32KB Flash, 2KB SRAM
- **I/O**: 14 digital pins, 8 analog pins
- **Power**: USB 5V or external 7-12V

## 🛠️ Assembly Notes

1. **Capacitors**: Place 100nF near Arduino and Display for stability
2. **Soldering**: Soldered connections for better reliability
3. **Case**: Protective enclosure recommended for practical use
4. **Display**: Use the 4 mounting holes for stable fixing

## 🐛 Debug

The code includes complete serial output:
- Open Serial Monitor at 9600 baud
- Real-time status and timing monitoring
- Messages for all operations

## 📝 License

Open source project - Use and modify freely

---

**Built with PlatformIO and standard components** 🚀
