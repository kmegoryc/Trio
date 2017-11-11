#include <CurieBLE.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"

#define TXRX_BUF_LEN              20 //max number of bytes
#define RX_BUF_LEN                20 //max number of bytes
uint8_t rx_buf[RX_BUF_LEN];
int rx_buf_num, rx_state = 0;
uint8_t rx_temp_buf[20];
uint8_t outBufMidi[128];

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

//keeps track of the last pins touched so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

//Buffer to hold 5 bytes of MIDI data. Note the timestamp is forced
uint8_t midiData[] = {0x80, 0x80, 0x00, 0x00, 0x00};

void midiControl(char chan, byte controller, byte onOrOff) {
  midiData[2] = 0xB0 + chan;
  midiData[3] = controller;
  midiData[4] = onOrOff;
  
}

//BLEPeripheral midiDevice; // create peripheral instance

BLEService midiSvc("03B80E5A-EDE8-4B33-A751-6CE34EC4C700"); // create service

// create switch characteristic and allow remote device to read and write
BLECharacteristic midiChar("7772E5DB-3868-4112-A1A9-F2669D106BF3",  BLEWrite | BLEWriteWithoutResponse | BLENotify | BLERead,5);

void setup() {
  Serial.begin(9600);

  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  BLESetup();
    // advertise the service
  BLE.advertise();
  Serial.println(("Bluetooth device active, waiting for connections..."));
}

void BLESetup()
{
  BLE.begin();
  // set the local name peripheral advertises
  BLE.setLocalName("Harmony");

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

  //get the currently touched pads
  currtouched = cap.touched();

  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      //Serial.print(i); Serial.println(" touched");
      midiControl(0, i, 0x7f); //ith controller, value 127 
      midiChar.setValue(midiData, 5);//midiData); //posts 5 bytes
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      //Serial.print(i); Serial.println(" released");
      midiControl(0, i, 0x01); //ith controller, value 1 
      midiChar.setValue(midiData, 5);//midiData); //posts 5 bytes
    }
  }

  // reset our state
  lasttouched = currtouched;
  
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
