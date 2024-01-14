/*

ESP32 PIN		Output Chanel
-----------------------------
26				0
18				1
25				2
19				3
21				4
32				5
22				6
23				7
16				8
15				9

*/

#include "Arduino.h"
#include "colourObject.h"
#include "I2SClocklessLedDriver.h"


byte maxValue=64;
const unsigned short int numLeds=250;	//number of pixels per chanel
char colourList[8*3]={maxValue,0,0, maxValue,maxValue,0, 0,maxValue,0, 0,maxValue,maxValue, 0,0,maxValue, maxValue,0,maxValue, maxValue,maxValue,maxValue, maxValue,0,0};
colourObject dynColObject(maxValue, 8, colourList);

const byte numOfStrands=10;	//Number of outpur chanels
unsigned short int strandCnt=0;
uint8_t leds[numOfStrands*numLeds*3];
int pins[numOfStrands]={26,18,25,19,21,32,22,23,16,15};

unsigned short int cIndex=0, innerCIndex=0;
short int pIndex=0;
byte tempCol[3] = {0,0,0};

I2SClocklessLedDriver driver;

void setup() 
{
    Serial.begin(115200);

    driver.initled(leds,pins,numOfStrands,numLeds,ORDER_GRB);
    
    for(strandCnt=0; strandCnt<numOfStrands*numLeds; strandCnt++)
    {
      driver.setPixel(strandCnt, tempCol[0], tempCol[1], tempCol[2]);
    }
   
    renderLEDs();
    
}
void loop() 
{
    
    // Init data with only one led ON
    for(strandCnt=0; strandCnt<numOfStrands*numLeds; strandCnt++)
    {
      dynColObject.getColour(innerCIndex%dynColObject._bandWidth, tempCol);
      driver.setPixel(strandCnt, tempCol[0], tempCol[1], tempCol[2]);
      innerCIndex+=5;
    }
    renderLEDs();
    delay(10);
    cIndex+=5;
    innerCIndex = cIndex;
}

void renderLEDs()
{
    driver.showPixels();
}
