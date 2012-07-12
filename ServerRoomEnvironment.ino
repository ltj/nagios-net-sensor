/* Environment sensor sketch with simple web
*  server. Should work with both Arduino Ethernet
*  and the Ethenet Shield.
*  Author: Lars Toft Jacobsen (http://boxed.dk)
*  Version 0.1a (still prototyping)
*  Software comes with no warranty
*  Original work - CC BY-SA
* 
*  TSL2561 code is adapted from the TSL3661 library
*  example by Adafruit Industries
*  Ethernet code is adapted from the example sketch
*  by Tom Igoe.
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <SHT1x.h>
#include "TSL2561.h"

#define DEBUG 0		// if set, readings are sent to serial
#define SHTDATA 7
#define SHTCLOCK 6

// variables
float temp_c, humidity;
uint32_t lux;
int senseInterval = 10000; // sensor measurement interval in ms
long previousMillis = 0;

// Configure Ethernet - remember to input correct values here!
byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
IPAddress ip(192,168,1,10);
IPAddress gateway(192,168,1,1);	
IPAddress subnet(255, 255, 255, 0);

// Instantiate sensor objects
SHT1x sht1x(SHTDATA, SHTCLOCK);
TSL2561 tsl(TSL2561_ADDR_FLOAT);

// Initialize the Ethernet server library
EthernetServer server(80);

void measure() {
  // Read values from the SHT11 sensor
  temp_c = sht1x.readTemperatureC();
  humidity = sht1x.readHumidity();
  
  // Read values from TLS2561 and calculate lux
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  lux = tsl.calculateLux(full, ir);
  
  #if DEBUG
    Serial.print("Temperature: ");
    Serial.print(temp_c, DEC);
    Serial.print("C / ");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Lux: "); 
    Serial.println(lux);
  #endif
}

void listenForEthernetClients() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    #if DEBUG
      Serial.println("Got a client");
    #endif
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          // print the current readings, in HTML format:
          client.print("T:");
          client.print(temp_c);
          client.print(",H:");
          client.print(humidity);
          client.print(",L:");
          client.print(lux);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

void setup() {
  // initialize the ethernet device and start server
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  
  #if DEBUG
    Serial.begin(9600);
  #endif
  
  // Start TSL2561
  tsl.begin();
  
  tsl.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
  //tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)
  
  // allow time to set up
  delay(1000);
}

void loop() {
  
  // measure only every 'senseInterval' millis
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > senseInterval) { 
    previousMillis = currentMillis;
    measure();
  }
  
  // listen for incoming Ethernet connections:
  listenForEthernetClients();
  
}

