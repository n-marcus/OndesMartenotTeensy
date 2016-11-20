/*
   Gametrack pins:
   Black = 5V
   Green = Ground

   Orange = cord
   Yellow and red are x & Y
*/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>
//#include <SmootherClass.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=281,173
AudioFilterBiquad        biquad1;        //xy=572,164
AudioOutputI2S           i2s1;           //xy=793,163
AudioConnection          patchCord1(waveform1, biquad1);
AudioConnection          patchCord2(biquad1, 0, i2s1, 0);
AudioConnection          patchCord3(biquad1, 0, i2s1, 1);
// GUItool: end automatically generated code

//Smoother volSmooth();

AudioControlSGTL5000 audioShield;

float freq = 440;

float sensorPin1 = A0;    // select the input pin for the potentiometer
int sensorPin2 = A1;    // select the input pin for the potentiometer
int sensorPin3 = A2;    // select the input pin for the potentiometer
int sensorPin4 = A3;

int sensorValue1;
int sensorValue2;
int sensorValue3;
int sensorValue4;

int ledPin1 = 13;      // select the pin for the LED
int ledPin2 = 21;      // select the pin for the LED

//Smoothing stuff
const int numReadings = 1000;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

float vol = 0.5;
int filterFreq = 10000;
float freqOffset = 0.0;

float midi[127];
int a = 440; // a is 440 hz...



void setup() {

  // declare the ledPin as an OUTPUT:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  Serial.begin(9600);
  Serial.println("Hello, I am alive!");

  initAudio();
  initSmoothing();

  //The "I am alive" blink and sound
  digitalWrite(ledPin1, HIGH);   // set the LED on
  delay(500);
  digitalWrite(ledPin1, LOW);   // set the LED on

  for (int x = 0; x < 127; ++x)
  {
    midi[x] = a * pow(2.0, (x - 69) / 12.0);
  }

  for (int x = 0; x < 127; ++x)
  {
    Serial.println("MIDI NOTE " + String(x) + " = " + String(midi[x]));
  }
}



void loop() {
  analogReadings();
  mapData();
  smoothValue1();
  tuner();
  audio();
  visual();
}

void visual() {
  digitalWrite(ledPin2, (1.0 - vol) * 255);
}

void mapData() {
  //mapping data to audio
  vol = (map(sensorValue4, 0, 1023, 100, 0) / 75.0);
  if (freq < 20) {
    vol = 0.0;
  }
  //  vol = volSmooth.smooth(vol,0.5);
  //Serial.println("Vol = " + String(vol));
  filterFreq = map(sensorValue2, 0, 1023, 10000, 10);
  freqOffset = (sensorValue3 - (1023.0 / 2.0)) / 25.0;
  //Serial.println("Freqoffset = " + String(freqOffset));
  freq = midi[int((sensorValue1 * 0.5 ) / 5.0) + 30];
  Serial.println(freq);
  //freq = (sensorValue1 * 0.5) + freqOffset;

  //Serial.println("Freq = " + String(freq));
}

void audio() {
  waveform1.amplitude(vol);
  //setting freq
  waveform1.frequency(freq);

  // Butterworth filter, 12 db/octave
  biquad1.setLowpass(0, filterFreq, 0.707);
}

void tuner() {
  //tuner
  double midi_note = (log(freq / 440.0) / log(2) * 12 + 57);
  double detune = midi_note - round(midi_note);
  if (detune < 0.075) {
    digitalWrite(ledPin1, HIGH);   // set the LED on
  } else {
    digitalWrite(ledPin1, LOW);   // set the LED off
  }
  //printDouble(midi_note - round(midi_note),100);
}


void printDouble( double val, unsigned int precision) {
  // prints val with number of decimal places determine by precision
  // NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
  // example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

  Serial.print (int(val));  //prints the int part
  Serial.print("."); // print the decimal point
  unsigned int frac;
  if (val >= 0)
    frac = (val - int(val)) * precision;
  else
    frac = (int(val) - val ) * precision;
  Serial.println(frac, DEC) ;
}

void initAudio() {
  AudioMemory(12);

  audioShield.enable();
  audioShield.volume(0.8);
  //waveform1.pulseWidth(0.5);
  waveform1.begin(0.4, 440, WAVEFORM_SQUARE);
  biquad1.setLowpass(0, 800, 0.707);
}

void analogReadings() {
  // read the value from the sensors:
  sensorValue1 = analogRead(sensorPin1);
  sensorValue2 = analogRead(sensorPin2);
  sensorValue3 = analogRead(sensorPin3);
  sensorValue4 = analogRead(sensorPin4);
  //Serial.println("Analog 0 = "  + String(sensorValue1));
  //Serial.println("Analog 1 = "  + String(sensorValue2));
  //Serial.println("Analog 2 = "  + String(sensorValue3));
  //Serial.println("Analog 3 = "  + String(sensorValue4));
}

void initSmoothing() {
  //Smoothing stuff
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}


void smoothValue1() {
  //Smoothing stuff
  total = total - readings[readIndex];
  readings[readIndex] = sensorValue1;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the average:
  average = total / numReadings;
  //Serial.println("Average = " + String(average));

}


