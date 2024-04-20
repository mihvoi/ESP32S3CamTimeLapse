/**
  * This project helps to create a "timelapse" video by saving a camera picture on SD card at a configurable interval
  * First you need to connect to the camera URL (check console) and adjust the resolution and other knobs
  * Then you can start the timelapse capture from the same web interface
  * The SD card must be formatted FAT or FAT32
  * After incomplete writing the SD card mounting might fail
  * Big SD cards (>32GB) might not be supported.
  * 
  * This is a fork from https://github.com/bitluni/ESP32CamTimeLapse. AFAIK the ESP32CamTimeLapse project is based on CameraWebServer.ino project
  * 
  * I fixed the timelapse for ESP32-S3-WROOM1 CAM camera, that resembles "Freenove ESP32-Wrover CAM". 
  * Should still work with the original ESP32-CAM - tested with one. 
  *
  * Steps:
  *  - format a SD card up to 32GB (not more) as FAT or FA32
  *  - insert the SD card in the board  
  *  - Set the Port and Board in Arduino IDE
  *      - For ESP32-S3-WROOM1 CAM, set the board to "ESP32S3 Dev Module"
  *      - For ESP32-CAM (old version) set the board to "AI Thinker ESP32-CAM"
  *  - only if you have a camera other than the 2 above
  *     - set the camera in camera.cpp
  *     - change the SD card gpios in file.cpp -> initFileSystem()
  *  - configure the WiFi with ssid and password - only 2.4Ghz works usually
  *  - activate Tools/PSRAM/OPI PSRAM - if available
  *  - burn the image to the board; you might need to keep the Boot/IO0 button pushed before connecting the USB cable to the board.
  *  - boot/reset the ESP32 board
  *  - check the console for Camera Ready! Use 'http://xx.xx.xx.xx' to connect
  *  - connect to the URL
  *  - check Still/Stream
  *  - increase resolution to the maximum that does not freeze
  *  - start the Time-Lapse from the same web menu
  *  - you can now disconnect from the web interface
  *  - after board restart, you need to re-start Time-Lapse
  *  - if you want to autostat Time-Lapse, you can edit lapse.cpp and comment if(!lapseRunning) return false; - however you might need to change defaults for resolution also
  *
  *    
  * No warranties.
  * I am a noob in ESP32, hope this help. Feel free to suggest more elegant solutions.
  *
  */

#include <WiFi.h>
#include "file.h"
#include "camera.h"
#include "lapse.h"

const char *ssid = "changeme";
const char *password = "changeme";

void startCameraServer();

void setup()
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println();
	initFileSystem();
	initCamera();
  
  Serial.printf("Connecting to WIFI ssid=%s\n", ssid);
	WiFi.begin(ssid, password);
	
  while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	startCameraServer();
	Serial.print("Camera Ready! Use 'http://");
	Serial.print(WiFi.localIP());
	Serial.println("' to connect");
}

/**
  * Returns elapsed period in ms from the previous return
  */

long sleepTo(long nextCapture){
    unsigned long now;
    static unsigned long previousCapture = 0;

    now = millis();
    
    long toSleep = nextCapture - now - 1;
    if(toSleep < 0){
      Serial.printf("\nWarn: will still sleep 1ms even if toSleep=%ld ms\n", toSleep);
      toSleep=1;
    } 
    
    Serial.printf(" [sleep=%Ldms]", toSleep);
    delay(toSleep);

  //Verify the wake delay
  now = millis();
  long diff = nextCapture - now;
  if( diff > 2 ){
    Serial.printf("\nWarn: woke up too early with diff=%Ld ms\n", diff);
    sleepTo(nextCapture);
  }

  //We are not early, are we late?
  now = millis();
  long late = now - nextCapture;
    if(late>1){
      Serial.printf("\nWarn: late capture by %Ld ms\n", late);
    }

  long dt = now - previousCapture;
  previousCapture = millis();
  return dt;
}

extern unsigned long frameInterval;
static unsigned long nextCapture = 0;
long dt = 5000; //Can be 0; just for backward compatibility with original processLapse()

void loop()
{

	processLapse(dt);

  nextCapture += frameInterval;

  //Adjustment at start and when we are very behind
  unsigned long now = millis();
  if(nextCapture < now-1000){
    Serial.printf("\nWarn: Adjusting from nextCapture=%Ld to now=%Ld\n", nextCapture, now);
    nextCapture = now;
  }

  dt = sleepTo(nextCapture);
  Serial.printf(" [elapsed=%Ldms]", dt);

}
