// LCD Panel Backlight Fixture Controller
// Copyright 2024, François Revol <revol@free.fr>

// Number of supported panels
// Or rather, channels, since can actually control
// multiple panels as one from the same channel.
#define NP 4

// PWM outputs
const int outputs[NP] = { 9, 10, 11, 3 };

// BL_ON
#define OUT_ON 12
// TODO: add an inverted output, maybe some displays need it?
//#define OUT_OFF 12

// Potentiometer input
#define IN1 A0

int settings[NP+1][2] {
  // update from the dump command
  // One entry per channel, with min/max allowed values.
  // Last entry is the global bounds (min is ON_THRESHOLD, max currently unused)
  { 40, 1023 },
  { 0, 1023 },
  { 0, 1023 },
  { 0, 1023 },
  { 20, 1023 },
};


float rawValues[4];

// Since the original controller board drives
// the backlight PWM at 20 to 30kHz, we reconfigure
// the timers to be in that range for platforms
// we know about. There is a fallback code that
// should still work thouh.

// TODO: test fallback

// cf. https://arduino.stackexchange.com/questions/19892/list-of-arduino-board-preprocessor-defines
#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)

void platformSetup() {
  //TCCR2B = TCCR2B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz
  // https://docs.arduino.cc/tutorials/generic/secrets-of-arduino-pwm/
  // https://www.reddit.com/r/arduino/comments/14tnk7c/arduino_pwm_frequency_of_25khz/
  //25kHz has a period of 40uS is 640 ticks of a 16MHz clock
  //  use prescaler = 1, WGM14
  //
  //pinMode( pinOC1A, OUTPUT );
  ICR1 = 640-1;     //640 ticks of 16MHz clock == 40uS == 25kHz
  //OCR1A = 320;    //50% duty cycle
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);  
  //TCCR1B = 0x18; // 0001 1000, Disable Timer 
  //TCCR1A = 0x82; // 1000 0010
  // TODO
  // 31kHz, reloads with 0xff
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B &= 0xf8;
  TCCR2B |= /*_BV(WGM22) |*/ _BV(CS20);  
}

// we accept floats to avoid loosing precision when recalculating
void myAnalogWrite(int pin, float value) {
  int i;
  // scale value from 256 to 1024
  value *= 4;
  for (i = 0; i < NP; i++) {
    if (outputs[i] == pin) {
      // scale the value to the range defined for this panel
      value *= (float)settings[i][1] / (1023 - settings[i][0]);
      value += settings[i][0];
      rawValues[i] = value;
      break;
    }
  }
  switch (pin) {
    case 9:
      OCR1A = (int) (ICR1 * value/1024);// adjust Duty
      break;
    case 10:
      OCR1B = (int) (ICR1 * value/1024);// adjust Duty
      break;
    case 11:
      // TODO:
      OCR2A = (int) (0xff * value/1024);// adjust Duty
      break;
    case 3:
      OCR2B = (int) (0xff * value/1024);// adjust Duty
      break;
    default:
      // fallback
      analogWrite(pin, (int)value);
  }
}

#else
#error "This code is platform specific"
// we accept floats to avoid loosing precision when recalculating
void myAnalogWrite(int pin, float value) {
  analogWrite(pin, (int)value);
}
#endif

uint8_t output = 0;
uint8_t setting = 0;
int pot;
float outValue;
bool debugOut = false;

void help() {
  Serial.println("1..4  Select output");
  Serial.println("l Select low setting");
  Serial.println("h Select high setting");
  Serial.println("+/- Change selected");
  //Serial.println("w wizard");
  Serial.println("d dump settings");
  Serial.println("D toggle debug");
  Serial.println("? for help");
}

void dump() {
  for (int i = 0; i < NP+1; i++) {
    Serial.print("  { ");
    Serial.print(settings[i][0]);
    Serial.print(", ");
    Serial.print(settings[i][1]);
    Serial.println(" },");
  }
}

void handleSerial() {
  char rc = Serial.read();
  Serial.println("");

  switch (rc) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      output = rc - '1';
      Serial.print("Panel ");
      Serial.print(rc);
      Serial.println(" selected.");
      break;
    case 'l':
      setting = 0;
      break;
    case 'h':
      setting = 1;
      break;
    case '-':
      settings[output][setting] = max(0,settings[output][setting]-1);
      break;
    case '+':
      settings[output][setting] = min(1023,settings[output][setting]+1);
      break;
    case '?':
      help();
      break;
    case 'v':
      Serial.println(pot);
      Serial.println(outValue);
      Serial.println("");
      for (int i = 0; i < 4; i++) {
        Serial.print(rawValues[i]);
        Serial.print(" ");
      }
      Serial.println("");
      break;
    case 'd':
      dump();
      break;
    case 'D':
      debugOut != debugOut;
      break;
    case '\n':
    case '\r':
      break;
    default:
      Serial.println("Unknown code");
      //help();
      break;
  }
}


void setup() {
  Serial.begin(9600);
  Serial.println("<LCD Panel Fixture>");
  Serial.println("? for help");

  pinMode(IN1, INPUT);
  pinMode(OUT_ON, OUTPUT);
  for (int i = 0; i < NP; i++) {
    pinMode(outputs[i], OUTPUT);
  }
  platformSetup();
  pot = 0;
}


void loop() {
  // We average with the previous value to smooth the transitions
  pot += analogRead(IN1); // 1024
  pot /= 2;
  // TODO: compare with cached value
  outValue = (float)pot * 256 / 1024;
  if (debugOut)
    Serial.println(outValue);
  //analogWrite(OUT1, v);
  for (int i = 0; i < NP; i++) {
      myAnalogWrite(outputs[i], outValue);
  }
  // power on only above the threshold
  digitalWrite(OUT_ON, pot > settings[NP][0]);

  //v++;
  if (Serial.available() > 0)
    handleSerial();
  delay(20);
}
