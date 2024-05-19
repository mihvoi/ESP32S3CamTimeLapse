#include "FS.h"
#include "SD_MMC.h"

//For matching the SD pind by CAMERA_MODEL_*
#include "camera.h"


bool writeFile(const char *path, const unsigned char *data, unsigned long len)
{
	Serial.printf("\nWriting file len=%d path=%s\n", len, path);
  long startMs = millis();
	File file = SD_MMC.open(path, FILE_WRITE);
	if (!file)
	{
		Serial.println("\nFailed to open file for writing");
		return false;
	}
	if (file.write(data, len))
	{
		//Serial.printf("File written len=%d path=%s\n", len, path);
	}
	else
	{
		Serial.println("Write failed");
		return false;
	}
  long writeMs = millis() - startMs;
  if(writeMs > 1){
    Serial.printf(" [SDwrite=%Ldms]", writeMs);
  }
	file.close();
	return true;
}

bool appendFile(const char *path, const unsigned char *data, unsigned long len)
{
	Serial.printf("Appending to file: %s\n", path);

	File file = SD_MMC.open(path, FILE_APPEND);
	if (!file)
	{
		Serial.println("Failed to open file for writing");
		return false;
	}
	if (file.write(data, len))
	{
		Serial.println("File written");
	}
	else
	{
		Serial.println("Write failed");
		return false;
	}
	file.close();
	return true;
}

bool initFileSystem()
{

// non default pins configured for SD card on given camera board
#if defined(CAMERA_MODEL_ESP32S3_EYE) || defined(CAMERA_MODEL_FREENOVE_ESP32S3_CAM)
#define SD_MMC_CLK 39 
#define SD_MMC_CMD 38
#define SD_MMC_D0 40
#elif defined(CAMERA_MODEL_XIAO_ESP32S3)
#define SD_MMC_CLK 7 
#define SD_MMC_CMD 9
#define SD_MMC_D0 8
#elif defined(CAMERA_MODEL_TTGO_T_CAMERA_PLUS)
#define SD_MMC_CLK 21 // SCLK
#define SD_MMC_CMD 19 // MOSI
#define SD_MMC_D0 22  // MISO
#endif



#if defined(SD_MMC_CLK) 
    int sd_clk = SD_MMC_CLK;
    int sd_cmd = SD_MMC_CMD;
    int sd_data = SD_MMC_D0;
    
    Serial.printf("Remapping SD_MMC card pins to sd_clk=%d sd_cmd=%d sd_data=%d\n", sd_clk, sd_cmd, sd_data);
    if(! SD_MMC.setPins(sd_clk, sd_cmd, sd_data)){
        Serial.println("Pin change failed!");
        return false;
    } else {
        Serial.println("Pin change success!");
    }

    //Mounting with mode1bit=true
    if(!SD_MMC.begin("/sdcard", true)){
        Serial.println("Card Mount Failed");
        return false;
    }else{
        Serial.println("Card Mount SUCCESS");
    }

  Serial.println("Card Mount OK");

#else

  Serial.printf("NOT remapping SD_MMC card pins, trying to SD_MMC.begin()...\n");
	if (!SD_MMC.begin())
	{
		Serial.println("Card Mount Failed");
		return false;
	} else {
    Serial.println("Success SD_MMC.begin()");
  }
#endif

	uint8_t cardType = SD_MMC.cardType();

	if (cardType == CARD_NONE)
	{
		Serial.println("No SD card attached");
		return false;
	}
	Serial.print("SD Card Type: ");
	if (cardType == CARD_MMC)
		Serial.println("MMC");
	else if (cardType == CARD_SD)
		Serial.println("SDSC");
	else if (cardType == CARD_SDHC)
		Serial.println("SDHC");
	else

		Serial.println("UNKNOWN");

	uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
	Serial.printf("SD Card Size: %lluMB\n", cardSize);
	Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
	Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
	return true;
}

bool createDir(const char *path)
{
	Serial.printf("\nCreating Dir: %s\n", path);
	if (SD_MMC.mkdir(path))
	{
		Serial.printf("Dir created dir=%s\n", path);
	}
	else
	{
		Serial.println("\nmkdir failed");
		return false;
	}
	return true;
}

bool fileExists(const char *path)
{
	return SD_MMC.exists(path);
}

void print_SD_free_space(){
    uint64_t totalMB = SD_MMC.totalBytes() / (1024 * 1024);
    uint64_t usedMB = SD_MMC.usedBytes() / (1024 * 1024);
    uint64_t freeMB = totalMB - usedMB;
    uint64_t percFull = usedMB*100/totalMB;
    Serial.printf("SD card used=%2d%% ; freeMB=%llu\n", percFull, freeMB);
}
