#define TXRX_BUF_LEN              20 //max number of bytes
#define RX_BUF_LEN                20 //max number of bytes
uint8_t rx_buf[RX_BUF_LEN];
int rx_buf_num, rx_state = 0;
uint8_t rx_temp_buf[20];
uint8_t outBufMidi[128];

//Buffer to hold 5 bytes of MIDI data. Note the timestamp is forced
uint8_t midiData[] = {0x80, 0x80, 0x00, 0x00, 0x00};

//Loads up buffer with values for note On
void noteControl(char control, char note, char vel) 
{
  midiData[2] = control;
  midiData[3] = note;
  midiData[4] = vel;
}


//BLEPeripheral midiDevice; // create peripheral instance

BLEService midiSvc("03B80E5A-EDE8-4B33-A751-6CE34EC4C700"); // create service

// create switch characteristic and allow remote device to read and write
BLECharacteristic midiChar("7772E5DB-3868-4112-A1A9-F2669D106BF3",  BLEWrite | BLEWriteWithoutResponse | BLENotify | BLERead,5);

//LED Pins
#define RED_LED 9
#define YEL_LED 10
#define GRE_LED 11 
// mux outs
#define TOP_FRONT 2
#define BOTTOM_FRONT 3
#define BACK 4
#define PINKIES 5
// MUX ins
#define TOP_BUTTONS 6
#define MID_BUTTONS 7
#define BOT_BUTTONS 8

#include <stdio.h>
char string[100];

unsigned int prevButtons = 0;
unsigned int currentButtons = 0;
unsigned int changedButtons = 0; 
//midi variables
char chromatic_offset = 0; 
int octave_offset = 0;
unsigned int channel = 5; 
int noteButtons[7] = {5,4,3,9,2,1,0};
int noteOffsets [7] = {0,2,4,5,7,9,11};
//enum MidiTypes{
//  CC
//  NOTE_TOGGLE
//}

enum Events{
    NOTE_DETECTED,
    MODE_CHANGE,
    OCTAVE_UP,
    OCTAVE_DOWN
};

unsigned int risingEdge(unsigned int button)
{
    return (changedButtons&(1<<button))&(currentButtons&(1<<button));
}
unsigned int fallingEdge(unsigned int button)
{
    return (changedButtons&(1<<button))&(currentButtons&(!(1<<button)));
}

//unsigned int makeMidiData(unsigned int channel, unsigned int note_offset, unsigned int octave_offset, unsigned int chromatic_offset,int type)
//{
//  return ()<<8 + ()<<4 + 
//}

void wakey_wakey(int waitTime)
{
  for (int i = 0; i++; i < 255)
  {
    analogWrite(RED_LED, i);
    delayMicroseconds(waitTime);
  }
  for (int i = 255; i--; i >= 0)
  {
    analogWrite(RED_LED, i);
    delayMicroseconds(waitTime);
  }
  for (int i = 0; i++; i < 255)
  {
    analogWrite(YEL_LED, i);
    delayMicroseconds(waitTime);
  }
  for (int i = 255; i--; i >= 0)
  {
    analogWrite(YEL_LED, i);
    delayMicroseconds(waitTime);
  }
  for (int i = 0; i++; i < 255)
  {
    analogWrite(GRE_LED, i);
    delayMicroseconds(waitTime);
  }
  for (int i = 255; i--; i >= 0)
  {
    analogWrite(GRE_LED, i);
    delayMicroseconds(waitTime);
  }
}

unsigned int readButtons( int del)
{
  unsigned int buttons=0;
  //TOP FRONT TRIPLET
  digitalWrite(TOP_FRONT, HIGH);
  digitalWrite(BOTTOM_FRONT, LOW);
  digitalWrite(BACK, LOW);
  digitalWrite(PINKIES, LOW);
  buttons += digitalRead(TOP_BUTTONS);
  buttons += digitalRead(MID_BUTTONS)<<1;
  buttons += digitalRead(BOT_BUTTONS)<<2;
  delay(del);

  //BOTTOM FRONT TRIPLET
  digitalWrite(TOP_FRONT, LOW);
  digitalWrite(BOTTOM_FRONT, HIGH);
  digitalWrite(BACK, LOW);
  digitalWrite(PINKIES, LOW);
  buttons += digitalRead(TOP_BUTTONS)<<3;
  buttons += digitalRead(MID_BUTTONS)<<4;
  buttons += digitalRead(BOT_BUTTONS)<<5;
  delay(del);

  //BACK TRIPLET
  digitalWrite(TOP_FRONT, LOW);
  digitalWrite(BOTTOM_FRONT, LOW);
  digitalWrite(BACK, HIGH);
  digitalWrite(PINKIES, LOW);
  buttons += digitalRead(TOP_BUTTONS)<<6;
  buttons += digitalRead(MID_BUTTONS)<<7;
  buttons += digitalRead(BOT_BUTTONS)<<8;
  delay(del);


  //STANDALONE CIRCULAR BUTTONS
  digitalWrite(TOP_FRONT, LOW);
  digitalWrite(BOTTOM_FRONT, LOW);
  digitalWrite(BACK, LOW);
  digitalWrite(PINKIES, HIGH);
  buttons += digitalRead(TOP_BUTTONS)<<9;
  buttons += digitalRead(MID_BUTTONS)<<10;
  buttons += digitalRead(BOT_BUTTONS)<<11;
  delay(del);

  return buttons;
}

void setup() {
  // put your setup code here, to run once:
  //led setup
  pinMode(RED_LED, OUTPUT);
  pinMode(YEL_LED, OUTPUT);
  pinMode(GRE_LED, OUTPUT);
  // mux pins
  pinMode(TOP_FRONT, OUTPUT);
  pinMode(BOTTOM_FRONT, OUTPUT);
  pinMode(BACK, OUTPUT);
  pinMode(PINKIES, OUTPUT);
  pinMode(TOP_BUTTONS, INPUT);
  pinMode(MID_BUTTONS, INPUT);
  pinMode(BOT_BUTTONS, INPUT);

  Serial.begin(9600);

  BLESetup();
    // advertise the service
  BLE.advertise();
  Serial.println(("Bluetooth device active, waiting for connections..."));
  
}

void BLESetup()
{
  BLE.begin();
  // set the local name peripheral advertises
  BLE.setLocalName("Melody");

  // set the UUID for the service this peripheral advertises
  BLE.setAdvertisedServiceUuid(midiSvc.uuid());

  // add service and characteristic
  
  midiSvc.addCharacteristic(midiChar);
  BLE.addService(midiSvc);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, midiDeviceConnectHandler);
  BLE.setEventHandler(BLEDisconnected, midiDeviceDisconnectHandler);

  // assign event handlers for characteristic
  midiChar.setEventHandler(BLEWritten, midiCharacteristicWritten);
  // set an initial value for the characteristic
  midiChar.setValue(midiData, 5);


}

void loop() {

  BLE.poll();
  
  // put your main code here, to run repeatedly:
  prevButtons = currentButtons;
  currentButtons = readButtons(4);
  changedButtons = prevButtons^currentButtons; 
  if((changedButtons)!=0)
  {
    // whose patch select
    if(risingEdge(6)) // keren 
    {
      Serial.print("got to the correct place");
      channel = 3;
    }
    if(risingEdge(7)) // emma mode
    {
      channel = 4;
    }
    if(risingEdge(8)) // eric mode
    {
      channel = 5;
    }
      //chromatic offset key
    if(currentButtons&(1<<11))
    {
      chromatic_offset = 1; 
    }
    else
    {
      chromatic_offset = 0; 
    }
    //octave up/down logic 
    if(currentButtons&(1<<10))//enable octave change mode
    {
      if(risingEdge(9))// octave up rising edge triggered on button 11 
      {
        octave_offset += 12;
      }
      if(risingEdge(11))// octave up rising edge triggered on button 11 
      {
        octave_offset -= 12;
      }
    }
    //notes logic
    for(int i = 0; i<7;i++)
    {
      if(risingEdge(noteButtons[i]))
      {
        
        //midi to send
        int control = (0x90 + channel);
        int note = 60+noteOffsets[i] + octave_offset + chromatic_offset;
        int vel = 0x7F;
        Serial.println("note");
        noteControl(char(control), char(note), char(vel));
        midiChar.setValue(midiData, 5);//midiData); //posts 5 bytes
      }
      if(fallingEdge(noteButtons[i]))
      {
        //midi to send
        int control = (0x80 + channel);
        int note = 60+noteOffsets[i] + octave_offset + chromatic_offset;
        int vel = 0x7F;
        noteControl(char(control), char(note), char(vel));
        midiChar.setValue(midiData, 5);//midiData); //posts 5 bytes
      }
    }
    
  }
  
}

void midiDeviceConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void midiDeviceDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}

void midiCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written: ");
}
