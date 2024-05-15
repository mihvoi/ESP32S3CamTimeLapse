#pragma once

#include "Arduino.h"

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE  // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// #define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM

//Try to guess the camera model based on selected board
//Please overwrite/comment the code below if you get the wrong setting
#ifdef ARDUINO_ESP32S3_DEV
#define CAMERA_MODEL_ESP32S3_EYE
#endif
#ifdef ARDUINO_ESP32_DEV
#define CAMERA_MODEL_AI_THINKER
#endif
#ifdef ARDUINO_XIAO_ESP32S3
#define CAMERA_MODEL_XIAO_ESP32S3
#endif

//Good for ESP32-S3-WROOM CAM, resembling "Freenove ESP32-Wrover CAM" (Freenove not tested)
//Like: https://github.com/Freenove/Freenove_ESP32_S3_WROOM_Board/blob/main/Datasheet/ESP32-S3%20Pinout.pdf
//#define CAMERA_MODEL_ESP32S3_EYE

//Good for ESP32-CAM with Arduino board "AI Thinker ESP32-CAM"
//#define CAMERA_MODEL_AI_THINKER


#include "camera_pins.h"
#include "esp_camera.h"

bool initCamera();
