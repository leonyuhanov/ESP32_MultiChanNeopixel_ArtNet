#include "Arduino.h"
#include "SPI.h"
#include <WiFi.h>
#include "AsyncUDP.h"
#include "NeoViaSPI.h"
#include "I2SClocklessLedDriver.h"
#include "artNetPacket.h"


//PIN IO set up fro multichanel LED driver
const byte numOfFrames=10;
int pins[numOfFrames]={26,18,25,19,21,32,22,23,16,15};

//LED Setup for the LED umbrella which has 10 panels of 257 pixels each
unsigned short int numOfPixels=2570;
const byte bytesPerLED = 3;
const unsigned short int maxPixPerFrame = 257;
uint8_t masterLEDArray[numOfFrames*maxPixPerFrame*bytesPerLED];

//Init the Multichanel pixel driver object
I2SClocklessLedDriver driver;

//Networking for Artnet
const char * ssid = "YOUR_SSID";
const char * password = "YOUR_WIFI_PASSWORD";
IPAddress thisDevice(192,168,100,111);  //This is the IP address that THIS ESP32 binds to, change this to your own network 
IPAddress gateway(192,168,100,1);   	//This is the Gateway/Router IP ofTHIS ESP32, change this to your own network
IPAddress subnet(255,255,255,0);		//This is the SUBNET MASK of yoru network, change this to your own network
unsigned int artNetPort = 6454;
const short int maxPacketBufferSize = 530;
char packetBuffer[maxPacketBufferSize];
AsyncUDP udp;
short int packetSize=0;
artNetPacket dmxData;

//Artnet Set up
const byte numberOfDMXUniverses = 20;
//[Starting Universe ID, Ending Universe ID] (inclusive)
const unsigned short int universeRange[2] = {0,19}; 
unsigned short int startPixel=0;
const byte artNetMap[numberOfDMXUniverses][6] = {{0,0,0,0,169,170},{0,1,0,170,256,87},{1,2,0,0,169,170},{1,3,0,170,256,87},{2,4,0,0,169,170},{2,5,0,170,256,87},{3,6,0,0,169,170},{3,7,0,170,256,87},{4,8,0,0,169,170},{4,9,0,170,256,87},{5,10,0,0,169,170},{5,11,0,170,256,87},{6,12,0,0,169,170},{6,13,0,170,256,87},{7,14,0,0,169,170},{7,15,0,170,256,87},{8,16,0,0,169,170},{8,17,0,170,256,87},{9,18,0,0,169,170},{9,19,0,170,256,87}};

void setup() 
{
  //Init Serial Out
  Serial.begin(115200);
  Serial.printf("\r\n\r\n\r\nSystem Booting....\r\n");
  
  //Set up input
  pinMode(bootPin, INPUT);

  //Set up WIFI
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("HOSTNAME");			//change to whatever you want the hostname of this ESP32 to be
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
        delay(100);
        Serial.print(".");
  }
  WiFi.config(thisDevice, gateway, subnet);
  Serial.print("\r\nONLINE\t");
  Serial.print(WiFi.localIP());
  
  //Set up UDP Endpoint
  if(udp.listen(artNetPort))
  {
    udp.onPacket([](AsyncUDPPacket packet)
    {
      packetSize = packet.length();
      //Serial.printf("\r\n%d", packetSize);
      if(packetSize==maxPacketBufferSize)
      {
        memcpy(packetBuffer, packet.data(), packet.length());
        //Serial.printf("\r\n\tUDP Packet Received for Univirse\t[%d.%d]", packetBuffer[14], packetBuffer[15]);
        //packetBuffer[14] is the UNIVERSE byte check that it is within the range cinfigured above
        if(packetBuffer[14]>=universeRange[0] && packetBuffer[14]<=universeRange[1])
        {
          dmxData.parseArtNetPacket(packetBuffer);
          copyArtNetDataToMemory(dmxData.universe[0]);
        }
      }
    });
  }

  //Init multi chanel driver
  driver.initled(masterLEDArray,pins,numOfFrames,maxPixPerFrame,ORDER_GRB);
  renderLEDs();
}

void renderLEDs()
{
  //Push to pixels via clockless driver
  driver.showPixels();
}

void loop()
{   
  renderLEDs();
  delay(5);
}

void copyArtNetDataToMemory(byte universeID)
{
  startPixel = getStartPixelIndex(universeID)*bytesPerLED;
  //Serial.printf("\r\nStart Pixel Address\t[%d]\t for U\t[%d]\t[%d]bytes", startPixel, universeID, artNetMap[universeID][5]*bytesPerLED);
  memcpy(masterLEDArray+startPixel, dmxData.data, artNetMap[universeID][5]*bytesPerLED);

}

short int getStartPixelIndex(byte universeID)
{
  byte arrayCount=0;
  unsigned short int startPixelIndex=0;
  
  for(arrayCount=0; arrayCount<numberOfDMXUniverses; arrayCount++)
  {
    if(artNetMap[arrayCount][1]==universeID)
    {
      return startPixelIndex;
    }
    startPixelIndex+=artNetMap[arrayCount][5];
  }
  return -1;
}

