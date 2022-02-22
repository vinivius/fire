#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "fauxmoESP.h"
#include "credentials.h"

#define SERIAL_BAUDRATE 9600

const char* ssid = WiFi_SSID;
const char* password = WiFi_PASSWORD;

#define WIFI_SSID 
#define WIFI_PASS 

#define DEVICE_NAME "fireplace"

fauxmoESP fauxmo;

#define aA1 23
#define aB1 22

//double ti
uint8_t currentState = 0;
uint8_t statee = 2;
long timer =0;


void stopMotor(){
  digitalWrite(aA1, LOW);
  digitalWrite(aB1, LOW);
}

void firePlaceOn(){
  digitalWrite(aA1, HIGH);
  digitalWrite(aB1, LOW);   
}

void firePlaceOff(){
  digitalWrite(aA1, LOW);
  digitalWrite(aB1, HIGH);         
}




// Wi-Fi Connection
void wifiSetup() {
  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void setup() {
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();

  // Wi-Fi connection
  wifiSetup();

  
  pinMode(aA1, OUTPUT);
  pinMode(aB1, OUTPUT);
  stopMotor();

  // By default, fauxmoESP creates it's own webserver on the defined port
  // The TCP port must be 80 for gen3 devices (default is 1901)
  // This has to be done before the call to enable()
  fauxmo.createServer(true); // not needed, this is the default value
  fauxmo.setPort(80); // This is required for gen3 devices

  // You have to call enable(true) once you have a WiFi connection
  // You can enable or disable the library at any moment
  // Disabling it will prevent the devices from being discovered and switched
  fauxmo.enable(true);
  // You can use different ways to invoke alexa to modify the devices state:
  // "Alexa, turn lamp two on"

  // Add virtual devices
  fauxmo.addDevice(DEVICE_NAME);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    // Callback when a command from Alexa is received. 
    // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
    // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
    // Just remember not to delay too much here, this is a callback, exit as soon as possible.
    // If you have to do something more involved here set a flag and process it in your main loop.
        
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    if ( (strcmp(device_name, DEVICE_NAME) == 0) ) {
      // this just sets a variable that the main loop() does something about
      Serial.println("RELAY 1 switched by Alexa");
      //digitalWrite(RELAY_PIN_1, !digitalRead(RELAY_PIN_1));
      if (state) {
        if(currentState!=1){
        statee=1;
        }
        
      } else {
        if(currentState!=2){
        statee=2;
        }
      }
    }
  });

  if(EEPROM.read(0) == 0xbb){
    EEPROM.write(0, 0xaa);
    statee = EEPROM.read(1);
  } else {
    statee = 2;
  }

}

void loop() {
  // fauxmoESP uses an async TCP server but a sync UDP server
  // Therefore, we have to manually poll for UDP packets
  fauxmo.handle();

  switch (statee)
  {
  case 0:
    /* code */
    break;

    case 1:
      timer = millis()+10000;
      firePlaceOn();
      statee=3u;
      currentState=1;
    break;

    case 2:
    timer = millis()+13000;
    firePlaceOff();
    statee=3u;
    currentState=2;
    break;

    case 3:
    if(millis()>timer){
      stopMotor();
      statee = 0;
    }

  
  default:
    break;
  }

   if(WiFi.status() != WL_CONNECTED){
     EEPROM.write(0, 0xbb);
     EEPROM.write(1, statee);
     EEPROM.commit();
     ESP.restart();
   }

  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
  }
  
}

