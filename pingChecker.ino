/*
 *  Project     Ping checker
 *  @author     Fabian Roa
 *  @license    MIT - Copyright (c) 
 */
#include <Arduino_DataBus.h>
#include <Arduino_G.h>
#include <Arduino_GFX.h>
#include <Arduino_GFX_Library.h>
#include <Arduino_TFT.h>
#include <Arduino_TFT_18bit.h>
#include <gfxfont.h>
#include <cstring>
//we add the pingums
#include "WiFi.h"
#include <ESP32Ping.h>
#include <ping.h>
//we include the graphics
#include "images.h"

//the constants for the connection pins
#define TFT_SCK    18
#define TFT_MOSI   23
#define TFT_MISO   19
#define TFT_CS     22
#define TFT_DC     21
#define TFT_RESET  27

//variables
char *hosts[3] = {"204.2.229.102","www.google.com", "192.168.0.1"};
int arraySize = 0;
float averagePing = 0;
char result[7]; // Buffer big enough for 7-character float
char* blankLine = "                                   ";
char* blankLine2 = "      ";

//declare the methods
void connect_wifi(char* ssid, char* password);
float pingHost(char* host);
void drawBitmap(int x, int y, int w, int h, uint16_t* image);
void printPing(int line, float value);
void printWifiStatus(char* message, int line = 300);
void printWifiStatus(String message, int line =300);

//we start the screen in a global variable because it's easier that way
Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ILI9341 display = Arduino_ILI9341(&bus, TFT_RESET);

void setup() {
  display.begin();
  display.fillScreen(BLACK);

  //first the wifi setup
  //BAE, we set the array size
  arraySize = sizeof(hosts)/sizeof(hosts[0]);
  //we open comms
  Serial.begin(115200);
  //we assign the connection properties
  char* ssid = "yourSSID";
  char* password = "password";

  // try to connect to the wifies
  connect_wifi(ssid, password);
  //wait a little while
  delay(100);

  Serial.println("Setup done");

  //let's print the images
  drawBitmap(40,10, 64, 72, ffxiv);
  drawBitmap(40,110, 64, 64, google);
  drawBitmap(40,200, 64, 64, gateway);
}

void loop() {
  //we setup the basic screen values
  display.setTextSize(1);
  display.setTextColor(BLUE);
  
  if (WiFi.status() != WL_CONNECTED){
    Serial.print("Something went wrong, so we're trying to reconnect");
    WiFi.reconnect();  
  }

  //we're gonna do the cycle here so in case that we need to update the screen, it can be done in "real time"
  for (int i=0; i<arraySize;i++){
    Serial.println();
    //we get the ping and then convert it to a char
    averagePing = pingHost(hosts[i]);
    dtostrf(averagePing, 6, 2, result);
    //lets print on the display
    printPing(i, averagePing);
  }

  
}


//the actual pinging method
float pingHost(char* host){
  float average =0;
  bool pinged = false;

  //then we cycle thru it to get the values
  pinged = Ping.ping(host);
  if (pinged){
    average = Ping.averageTime();

    return average;
  }else{
    Serial.println("there was an error pinging ");
    //Serial.print(host);
    return average;
  }

  //not needed but a failsafe
  return average;
}

//the wifi connect method
void connect_wifi(char* ssid, char* password){
  char output[50];
  //put the wifi module on client/station mode
  WiFi.mode(WIFI_STA);
  //disconnect it because we wanna make sure it's not connected to anything
  WiFi.disconnect();
  //try to connect
  WiFi.begin(ssid, password);

  strcpy(output, ssid);
  strcat(output, " is Connecting");

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    printWifiStatus(output);
  }

  strcpy(output, "Connected to: ");
  strcat(output, ssid);

  printWifiStatus(output);
  printWifiStatus(WiFi.localIP().toString(),310);
  Serial.println(WiFi.localIP().toString());
}

void printWifiStatus(char* message, int line){
  display.setTextSize(1);
  display.setTextColor(RED, BLACK);
  display.setCursor(20, line);
  display.print(blankLine);
  display.setCursor(20, line);
  display.print(message);
}

void printWifiStatus(String message, int line){
  display.setTextSize(1);
  display.setTextColor(RED, BLACK);
  display.setCursor(20, line);
  display.print(blankLine);
  display.setCursor(20, line);
  display.print(message);
}

//the position to print and the width and height of the image
void drawBitmap(int x, int y, int w, int h, uint16_t* image){
  int row, col, buffidx=0;
  for (row=0; row<h; row++) { // For each scanline...
    for (col=0; col<w; col++) { // For each pixel...
      //the values are uint16_t, so we use pgm_read_word because it is 16-bit
      display.drawPixel(x+col, y+row, pgm_read_word(image + buffidx));
      buffidx++;
    } // pixel draw complete
  }
}

void printPing(int line, float value){
  //we increment the line because we're expecting 
  line++;
  dtostrf(averagePing, 6, 2, result);
  int realLine = 0;
  switch(line){
    case 1:
      realLine=40;
      break;
    case 2:
      realLine=135;
      break;
    case 3:
      realLine=230;
      break;
  }
  if (value < 100){
      display.setTextColor(GREEN, BLACK);
    }
    if (value >99 && value < 250){
      display.setTextColor(YELLOW, BLACK);
    }
    if (value > 249){
      display.setTextColor(RED, BLACK);
    }
  display.setTextSize(2);
  display.setCursor(120, realLine);
  display.print(blankLine2);
  display.setCursor(120, realLine);
  display.print(result);
}
