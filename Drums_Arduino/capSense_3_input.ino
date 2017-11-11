#include <CapacitiveSensor.h>

/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10M between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50K - 50M. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 */
 
const int TRIGGER_THRESHOLD = 125; 
 
const int NUMBER_OF_TRIGGERS = 3;

byte note;//storage for currently playing note

byte noteONs[NUMBER_OF_TRIGGERS] = {144,145,146};

//byte noteON = 144;

byte noteNums[NUMBER_OF_TRIGGERS] = {60,61,62};

boolean currentStates[NUMBER_OF_TRIGGERS] = {LOW,LOW,LOW};

boolean lastStates[NUMBER_OF_TRIGGERS] = {LOW,LOW,LOW};

int timecheck = millis();

CapacitiveSensor sensors[NUMBER_OF_TRIGGERS] = { 
  CapacitiveSensor(2,3),
  CapacitiveSensor(2,4),
  CapacitiveSensor(2,5), 
};

void setup()                    
{
   Serial.begin(9600);
}

void loop()                    
{
  for (int i=0; i<NUMBER_OF_TRIGGERS; i++) {
    int thisSensorReading = sensors[i].capacitiveSensor(30);
    boolean thisSensorIsTouched = thisSensorReading > TRIGGER_THRESHOLD;
    
    currentStates[i] = thisSensorIsTouched;
    
    if (currentStates[i] == HIGH && lastStates[i] == LOW){//if button has just been pressed
      if(millis() - timecheck >= 50)
      {
        MIDImessage(noteONs[i], noteNums[i], 127);//turn note on with 127 velocity
        Serial.println("pressed");
//        delay(2);//crude form of button debouncing
        timecheck = millis();
      }
      
    } 
    else if(currentStates[i] == LOW && lastStates[i] == HIGH){
      MIDImessage(noteONs[i], noteNums[i], 0);//turn note off
      delay(2);//crude form of button debouncing
    }
    lastStates[i] = currentStates[i];
  }
     
  delay(10);                             // arbitrary delay to limit data to serial port 
}

//send MIDI message
void MIDImessage(byte command, byte data1, byte data2) {
  Serial.write(command);
  Serial.write(data1);
  Serial.write(data2);
}
