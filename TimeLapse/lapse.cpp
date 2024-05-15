#include "Arduino.h"
#include "camera.h"
#include <stdio.h>
#include "file.h"

unsigned long fileIndex = 0;
unsigned long lapseIndex = 0;
unsigned long volatile frameInterval = 5000;
bool mjpeg = true;
bool lapseRunning = false;
unsigned long lastFrameDelta = 0;

bool setInterval(unsigned long delta) {
  frameInterval = delta;
  Serial.printf("\nUpdated frameInterval=%ld\n", frameInterval);
  return true;
}

bool startLapse() {
  Serial.println("\nStarting timelapse");
  if (lapseRunning) return true;

  fileIndex = 0;
  char path[32];
  for (; lapseIndex < 10000; lapseIndex++) {
    sprintf(path, "/lapse%03d", lapseIndex);
    if (!fileExists(path)) {
      createDir(path);
      lastFrameDelta = 0;
      lapseRunning = true;
      return true;
    }
  }
  return false;
}

bool stopLapse() {
  Serial.println("\STOPPING timelapse");
  lapseRunning = false;
  return true;
}

//dt can be removed; kept for backward compatibility with the old code
bool processLapse(unsigned long dt) {
  if (!lapseRunning){
    Serial.println("\nWarning: called while lapseRunning=false");
    return false;
  }

  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }

  char path[32];
  sprintf(path, "/lapse%03d/pic%05d.jpg", lapseIndex, fileIndex);
  //Serial.println(path);
  if (!writeFile(path, (const unsigned char *)fb->buf, fb->len)) {
    lapseRunning = false;
    return false;
  }
  fileIndex++;
  esp_camera_fb_return(fb);

  return true;
}
