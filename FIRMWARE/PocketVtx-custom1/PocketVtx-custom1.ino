#include "iivx_leo.h"
#include "Encoder.h"
iivxReport_t report;
#include "Keyboard.h"
#include "Mouse.h"
#include <EEPROM.h>

// Number of microseconds between HID reports
// 2000 = 500hz
#define REPORT_DELAY 2000

int currMode = EEPROM.read(0); // Read eeprom address 0. Value stored indicated mode. Mode1=joystick, Mode2=keyboard.
uint8_t buttonCount = 7;

// Button Keybinds
#define BIND_ST   KEY_RETURN
#define BIND_A    'a'
#define BIND_B    's'
#define BIND_C    'd'
#define BIND_D    'f'
#define BIND_FX_L 'z'
#define BIND_FX_R 'x'

// Button Pinout
#define BT_ST 4
#define BT_A  5
#define BT_B  6
#define BT_C  7
#define BT_D  8
#define FX_L  9
#define FX_R  10

// 0 = reactive lighting, 1 = HID lighting
uint8_t lightMode = 0;

uint8_t buttonPins[] = { 4, 5, 6, 7, 8, 9, 10};
uint8_t ledPins[] = {15, 15, 15, 15, 15, 15, 15, 15, 15};

// encoder sensitivity = number of positions per rotation times 4 (24*4) / number of positions for HID report (256)
#define ENCODER_SENSITIVITY (double) 0.375
Encoder encL(3, 2), encR(1, 0);
float knob1 = 0;
float knob2 = 0;
float old_knob1 = 0;
float old_knob2 = 0;

void lights(uint8_t lightDesc) {
  for (int i = 0; i < buttonCount; i++) {
    if ((lightDesc >> i) & 1) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

void setup() {
  delay(1000);
  
  // Setup I/O for pins
  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }

  // startup mode
  int Button1State = digitalRead(5); //Read Btn-A
  int Button2State = digitalRead(6); //Read Btn-B
    // button 1 is held down: Joystick Mode
    if (Button1State == LOW && Button2State == HIGH) {
      if (currMode != 1) { //if eeprom=2
        EEPROM.update(0, 1); 
        delay(200);
      }
    } else if (Button2State == LOW && Button1State == HIGH) {
      // button 2 is held down: Keyboard Mode
      if (currMode != 2) { //if eeprom=1
        EEPROM.update(0, 2);
        delay(200);
      }
    }
}

void loop() {
  if (EEPROM.read(0) == 1) {
    joy_mode();
  } else if (EEPROM.read(0) == 2) {
    keyboard_mode();
  }
}

void keyboard_mode() {
  // read encoders
  knob1 =  (float)(encL.read());
  knob2 = (float)encR.read();

  if(knob1 != old_knob1) {
    // if there's a difference in encoder movement from last pass, move the mouse
    if(knob1 < old_knob1) {
      Mouse.move(-5, 0);
    } else {
      Mouse.move(5, 0);
    }
    
    // we count the difference in the encoders, but we must not go over arduino's int limit
    if(knob1 < -255) {
      encL.write(0);
      old_knob1 = 0;
    } else if (knob1 > 255) {
      encL.write(0);
      old_knob1 = 0;
    } else {
      old_knob1 = knob1;
    }
  }
  
  if(knob2 != old_knob2) {
    if(knob2 > old_knob2) {
      Mouse.move(0, 5);
    } else {
      Mouse.move(0, -5);
    }
    
    if(knob2 < -255) {
      encR.write(0);
      old_knob2 = 0;
    } else if(knob2 > 255) {
      encR.write(0);
      old_knob2 = 0;
    } else {
      old_knob2 = knob2;
    }
  }
  
  // read the buttons for low, if it's low, output a keyboard press  
  if(digitalRead(BT_A) == LOW) {
    Keyboard.press(BIND_A);
  } else {
    Keyboard.release(BIND_A);
  }
  
  if(digitalRead(BT_B) == LOW) {
    Keyboard.press(BIND_B);
  } else {
    Keyboard.release(BIND_B);
  }
  
  if(digitalRead(BT_C) == LOW) {
    Keyboard.press(BIND_C);
  } else {
    Keyboard.release(BIND_C);
  }
  
  if(digitalRead(BT_D) == LOW) {
    Keyboard.press(BIND_D);
  } else {
    Keyboard.release(BIND_D);
  }
  
  if(digitalRead(FX_L) == LOW) {
    Keyboard.press(BIND_FX_L);
  } else {
    Keyboard.release(BIND_FX_L);
  }
  
  if(digitalRead(FX_R) == LOW) {
    Keyboard.press(BIND_FX_R);
  } else {
    Keyboard.release(BIND_FX_R);
  }
  
  if(digitalRead(BT_ST) == LOW) {
    Keyboard.press(BIND_ST);
  } else {
    Keyboard.release(BIND_ST);
  }
}

void joy_mode() {
  // Read buttons
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(buttonPins[i]) != HIGH) {
      report.buttons |= (uint16_t)1 << i;
    } else {
      report.buttons &= ~((uint16_t)1 << i);
    }
  }
  // Read Encoders
  report.xAxis = (uint8_t)((int32_t)(encL.read() / ENCODER_SENSITIVITY) % 256);
  report.yAxis = (uint8_t)((int32_t)(encR.read() / ENCODER_SENSITIVITY) % 256);
  // Light LEDs
  if (lightMode == 0) {
    lights(report.buttons);
  } else {
    lights(iivx_led);
  }

/* 
   ------- KEYBOARD ASCII CODES -------
  Top row keys: (use these if you don't have a numpad on your pc)
  48 - 0
  49 - 1
  50 - 2
  51 - 3
  52 - 4
  53 - 5
  54 - 6
  55 - 7
  56 - 8
  57 - 9
  45 - - (minus)(default service button)
  61 - = (equals)(default test button)
  8 - backspace
  
  
  220 NumPad /
  221 NumPad *
  222 NumPad -
  223 NumPad +
  224 NumPad ENTER
  225 NumPad 1 and End
  226 NumPad 2 and Down Arrow
  227 NumPad 3 and PageDn
  228 NumPad 4 and Left Arrow
  229 NumPad 5
  230 NumPad 6 and Right Arrow
  231 NumPad 7 and Home
  232 NumPad 8 and Up Arrow
  233 NumPad 9 and PageUp
  234 NumPad 0 and Insert
  235 NumPad . and Delete
  */
  
  iivx.setState(&report);
  delayMicroseconds(REPORT_DELAY);
}
