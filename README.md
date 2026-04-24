# Team Members:

Ahana Sharan  PES1UG24AM020\
Anarghyaa Kashyap  PES1UG24AM039

# LiFi Data Transfer System

A LiFi-based data transmission system built on Arduino UNO that receives 
text messages transmitted as Morse code light pulses via a mobile phone 
flashlight, decodes them, and displays the result on a 16x2 LCD. Also 
monitors and displays real-time temperature and humidity using a DHT11 sensor.

## How It Works

A mobile app (LiFi Transmitter) encodes a text message into Morse code and 
transmits it by flashing the phone torch on and off. An LDR sensor detects 
the light pulses and sends analog readings to the Arduino. The Arduino 
measures pulse durations to classify dots and dashes, decodes each Morse 
sequence into a character using a lookup table, and displays the assembled 
message on the LCD.

## Features

- LiFi data reception using LDR sensor
- Morse code decoding (A-Z, 0-9)
- Real-time temperature and humidity display via DHT11
- 16x2 LCD output
- Serial Monitor debug output with pulse timings

## Components

- Arduino UNO R3
- LDR (Light Dependent Resistor)
- DHT11 Temperature and Humidity Sensor
- 16x2 LCD Display (HD44780)
- 10k ohm potentiometer (LCD contrast)
- 10k ohm resistor (LDR voltage divider)
- Breadboard and jumper wires
- 9V battery

## Wiring

| Arduino Pin | Component         |
|-------------|-------------------|
| A0          | LDR middle point  |
| D2          | LCD D7 (Pin 14)   |
| D3          | LCD D6 (Pin 13)   |
| D4          | LCD D5 (Pin 12)   |
| D5          | LCD D4 (Pin 11)   |
| D7          | DHT11 DATA        |
| D11         | LCD EN (Pin 6)    |
| D12         | LCD RS (Pin 4)    |
| 5V          | LCD Pin 2, DHT11  |
| GND         | LCD Pin 1, DHT11  |

## Libraries Required

Install these via Arduino IDE Library Manager:

- LiquidCrystal (built-in)
- DHT sensor library by Adafruit

## Setup

1. Wire components as per the table above
2. Install required libraries in Arduino IDE
3. Upload main.ino to Arduino UNO
4. Open Serial Monitor at 9600 baud to view calibration readings
5. Note the LDR values with torch ON and torch OFF
6. Set LIGHT_THRESHOLD in the code to the midpoint of those two values
7. Install LiFi Transmitter app (by Ard Blog) on Android phone
8. Hold phone torch 2-3 cm above the LDR and send a message

## Calibration

The timing constants in the code were measured experimentally:

- LIGHT_THRESHOLD 900  (LDR analog cutoff between light and dark)
- DASH_MIN 600ms       (ON pulse longer than this = dash)
- LETTER_GAP_MIN 1000ms
- WORD_GAP_MIN 2300ms
- NEW_MSG_GAP 3000ms

These values may need adjustment depending on your LDR, App Speed and External Light in the Environment.

## Project Structure

lifi-morse-receiver-arduino/
├── main.ino
└── README.md
