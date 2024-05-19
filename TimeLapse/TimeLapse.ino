/**
  * This project helps to create a "timelapse" video by saving a camera picture on SD card at a configurable interval
  *
  * Select board guideline:
  * - ESP32-S3-WROOM1 CAM (resembels "Freenove ESP32-Wrover CAM") -> ESP32S3 Dev Module
  * - XIAO_ESP32S3 -> XIAO_ESP32S3 !! to activate SD might need to solder jumper, see https://wiki.seeedstudio.com/xiao_esp32s3_pin_multiplexing/ !!
  * - ESP32-CAM (old ESP32 camera board) -> AI Thinker ESP32-CAM
  * - for other boards, select the appropriate board in camera.h
  * 
  * Startup:
  * First you need to connect to the camera URL (check console) and adjust the resolution and other knobs
  * Then you can start the timelapse capture from the same web interface
  * The SD card must be formatted FAT or FAT32
  * After incomplete writing the SD card mounting might fail
  * Big SD cards (>32GB) might not be supported.
  * 
  * This is a fork from https://github.com/bitluni/ESP32CamTimeLapse. AFAIK the ESP32CamTimeLapse project is based on CameraWebServer.ino project
  * 
  * I fixed the timelapse for the board ESP32-S3-WROOM1 CAM camera, that resembles "Freenove ESP32-Wrover CAM". 
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

//A place where you can keep your wifi credentials
#include "credentials.h"


//You can overwrite WIFI credentials below or better define them in credentials.h
//#define WIFI_SSID "changeme"
//#define WIFI_PASS "changeme"

//Define AUTOSTART_TIMELAPSE above or in credentials.h to start timelapse automatically; note that UI will still show it disable at start
//#define AUTOSTART_TIMELAPSE

//Define START_AT_MAX_RESOLUTION if you want to start at maximum resolution - assure PSRAM is activated, otherwise the execution will likely break
//#define START_AT_MAX_RESOLUTION

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

void startCameraServer();

void setup()
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println("Starting ESP32 timelapse");
	initFileSystem();
  Serial.println("After initFileSystem SD");
  print_SD_free_space();

	initCamera();
  
  Serial.printf("Connecting to WIFI ssid=%s\n", ssid);
  WiFi.mode(WIFI_AP);
	WiFi.begin(ssid, password);

  int tries=100;
  while (WiFi.status() != WL_CONNECTED && tries-- > 0)
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


#ifdef START_AT_MAX_RESOLUTION
  Serial.printf("Setting resolution to FRAMESIZE_UXGA\n");
  sensor_t *s = esp_camera_sensor_get();
  //s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_framesize(s, FRAMESIZE_UXGA);
  
#endif

#ifdef AUTOSTART_TIMELAPSE
  Serial.println("Starting TIMELAPSE write on SD card");
  Serial.println("You may want to increase the resolution from web interface");
  setInterval(3000); //milliseconds interval for capture
  startLapse();
#endif

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
      Serial.printf("\nWarn: late capture by %Ldms\n", late);
    }

  long dt = now - previousCapture;
  previousCapture = millis();
  return dt;
}

extern unsigned long frameInterval;
extern bool lapseRunning;
static unsigned long nextCapture = 0;
static unsigned long loopNumber = 0;
static unsigned long startMs = millis();
long dt = 5000; //Can be 0; just for backward compatibility with original processLapse()

void loop()
{
  loopNumber++;
  dt = sleepTo(nextCapture);
  Serial.printf("\n [elapsed=%Ldms]", dt);

  if(lapseRunning){
    
    processLapse(dt);

    if(loopNumber % 10 == 0){
      printStats();
    }

  }else{
     Serial.printf(" [timelapse disabled]");
  }
 
  //Adjustment at start and when we are very behind
  unsigned long now = millis();
  if(nextCapture < now-1000){
    Serial.printf("\nWarn: Adjusting from nextCapture=%Ld to now=%Ld\n", nextCapture, now);
    nextCapture = now;
  }

    nextCapture += frameInterval;
}

void printStats(){
      long uptimeSec = (millis() - startMs)/1000;
      long uptimeMin = uptimeSec/60;
      uptimeSec = uptimeSec - (uptimeMin*60);
      Serial.printf("\nUptime: %lu min %lu sec ; loopNumber=%lu\n", uptimeMin, uptimeSec, loopNumber);
      print_SD_free_space();
}
