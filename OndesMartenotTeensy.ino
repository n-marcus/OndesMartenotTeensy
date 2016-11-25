/*
   Gametrack pins:
   Black = 5V
   Green = Ground
   Orange = cord
   Yellow and red are x & Y
*/
#include <math.h>
#include <AnalogSmooth.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSineHires sine_hires1;    //xy=77,91
AudioSynthWaveform       waveform2;      //xy=78,156
AudioSynthWaveform       waveform1;      //xy=79,122
AudioSynthWaveform       waveform3;      //xy=81,193
AudioMixer4              mixer3;         //xy=253,145
AudioFilterBiquad        biquad1;        //xy=400,146
AudioMixer4              mixer2;         //xy=552,164
AudioEffectDelay         delay1;         //xy=555,333
AudioMixer4              mixer1;         //xy=901,251
AudioOutputI2S           i2s1;           //xy=1069,236
AudioConnection          patchCord1(sine_hires1, 0, mixer3, 0);
AudioConnection          patchCord2(waveform2, 0, mixer3, 2);
AudioConnection          patchCord3(waveform1, 0, mixer3, 1);
AudioConnection          patchCord4(waveform3, 0, mixer3, 3);
AudioConnection          patchCord5(mixer3, biquad1);
AudioConnection          patchCord6(biquad1, 0, mixer2, 0);
AudioConnection          patchCord7(mixer2, 0, mixer1, 0);
AudioConnection          patchCord8(mixer2, delay1);
AudioConnection          patchCord9(delay1, 0, mixer1, 1);
AudioConnection          patchCord10(delay1, 0, mixer2, 1);
AudioConnection          patchCord11(mixer1, 0, i2s1, 0);
AudioConnection          patchCord12(mixer1, 0, i2s1, 1);
// GUItool: end automatically generated code



//Smoother volSmooth();
AnalogSmooth freqSmooth = AnalogSmooth(100);
AnalogSmooth volumeSmooth = AnalogSmooth(100000);

AudioControlSGTL5000 audioShield;

float sensorPin1 = A0;    // select the input pin for the potentiometer
int sensorPin2 = A1;    // select the input pin for the potentiometer
int sensorPin3 = A2;    // select the input pin for the potentiometer
int sensorPin4 = A3;

int sensorValue1;
int _sensorValue1;
int sensorValue2;
int sensorValue3;
int sensorValue4;

int ledPin1 = 13;      // select the pin for the LED
int ledPin2 = 21;      // select the pin for the LED

int buttonPin1 = 2;
int buttonState1 = 0;
int _buttonState1 = 0;

float vol = 0.5;
float freq = 440;
float midiFreq = 0;
int filterFreq = 10000;
float freqOffset = 0.0;

int waveform = 0;

float midi[127];
int a = 440; // a is 440 hz...




void setup() {

  // declare the ledPin as an OUTPUT:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buttonPin1, INPUT);
  Serial.begin(9600);
  Serial.println("Hello, I am alive!");

  initAudio();

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
  tuner();
  audio();
  visual();
}

void initAudio() {
  AudioMemory(120);

  audioShield.enable();
  audioShield.volume(0.8);
  mixer2.gain(0, 0.8); //dry output (first mixer pass)
  mixer2.gain(1, 0.4); //delay feedback amount
  mixer1.gain(0, 0.8); //dry output (second mixer pass)
  mixer1.gain(1, 0.2); //delay (no feedback)
  mixer3.gain(0, 0.9); //waveform1 to filter etc...
  mixer3.gain(1, 0.9); //sine_hires1 to filter etc.
  delay1.delay(0, 250);
  waveform1.begin(0.4, 440, WAVEFORM_TRIANGLE);
  waveform2.begin(0.4, 440, WAVEFORM_SAWTOOTH);
  waveform3.begin(0.4, 440, WAVEFORM_SQUARE);
  biquad1.setLowpass(0, 800, 0.707);
}

void visual() {
  digitalWrite(ledPin2, (1.0 - vol) * 255);
  if (sensorValue1 == _sensorValue1) {
    digitalWrite(ledPin1, LOW);
  }
  else {
    digitalWrite(ledPin1, HIGH);
  }
  _sensorValue1 = sensorValue1;
}

void mapData() {
  //mapping data to audio
  vol = volumeSmooth.smooth(sensorValue4);
  vol = (map(vol, 0, 1023, 100, 0) / 75.0); //map the volume from 100 to 0 and divide by 75 to create range(0 - 1.33) and smooth it
  if (freq < 20) {
    vol = 0.0;
  }
  //  vol = volSmooth.smooth(vol,0.5);
  Serial.print("Vol = " );
  for (int i = 0; i < vol* 40.0; i ++) { 
    Serial.print("*");
  }
  Serial.println("");
  

  filterFreq = map(sensorValue2, 0, 1023, 10000, 10);

  freqOffset = (sensorValue3 - (1023.0 / 2.0)) / 25.0;
  //Serial.println("Freqoffset = " + String(freqOffset));

  //quantizing to midi notes
  midiFreq = midi[int((sensorValue1 * 0.5 ) / 5.0) + 30]; //expand physical range and offset by 30 midi notes
  //Serial.println(sensorValue1);
  //smoothing the midi notes
  freq = freqSmooth.smooth(midiFreq);
  freq = freq + freqOffset;
  //Serial.println(freq);
  //freq = (sensorValue1 * 0.5) + freqOffset;

  if (buttonState1 != _buttonState1 && buttonState1 == HIGH) {
    waveform += 1;
    waveform = waveform % 4;
    Serial.println("waveform = " + String(waveform));
  }
}

void audio() {
  if (waveform == 0) {
    sine_hires1.amplitude(vol);
    sine_hires1.frequency(freq);
  } else {
    sine_hires1.amplitude(0.0);
  }

  if (waveform == 1) {
    waveform1.amplitude(vol);
    //setting freq
    waveform1.frequency(freq);
  } else {
    waveform1.amplitude(0.0);
  }

  if (waveform == 2) {
    waveform2.amplitude(vol);
    //setting freq
    waveform2.frequency(freq);
  } else {
    waveform2.amplitude(0.0);
  }

  if (waveform == 3) {
    waveform3.amplitude(vol);
    //setting freq
    waveform3.frequency(freq);
  } else {
    waveform3.amplitude(0.0);
  }

  // Butterworth filter, 12 db/octave
  biquad1.setLowpass(0, filterFreq, 0.707);
}

void tuner() {
  //tuner
  double midi_note = (log(freq / 440.0) / log(2) * 12 + 57);
  double detune = midi_note - round(midi_note);
  if (detune < 0.075) {
    //digitalWrite(ledPin1, HIGH);   // set the LED on
  } else {
    //digitalWrite(ledPin1, LOW);   // set the LED off
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

void analogReadings() {
  // read the value from the sensors:
  sensorValue1 = analogRead(sensorPin1); //
  sensorValue2 = analogRead(sensorPin2); //x
  sensorValue3 = analogRead(sensorPin3); //y
  sensorValue4 = analogRead(sensorPin4); //volume sensor

  _buttonState1 = buttonState1;
  buttonState1 = digitalRead(buttonPin1);

  //Serial.println("Analog 0 = "  + String(sensorValue1));
  //Serial.println("Analog 1 = "  + String(sensorValue2));
  //Serial.println("Analog 2 = "  + String(sensorValue3));
  //Serial.println("Analog 3 = "  + String(sensorValue4));
}


