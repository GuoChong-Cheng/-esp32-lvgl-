#include "sd.hpp"

bool g_sdcrad_ok = false;

bool sd_init(void)
{
    pinMode(PIN_SD_DET, INPUT);
    if (digitalRead(PIN_SD_DET)) {
        Serial.println("[SDCard] no SD card dectected");
        return false;
    } else {
        Serial.print("[SDCard] SD card dectected: ");
    }

    /* Init SD spi */
    static SPIClass* sd_spi = new SPIClass(FSPI);
    sd_spi->begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_NSS);
    SD.begin(PIN_SD_NSS, *sd_spi, 5e6);

    /* Get SD card info */
    uint8_t cardType = SD.cardType();
    switch (cardType) {
        case CARD_MMC:
            Serial.print("MMC");
            break;
        case CARD_SD:
            Serial.print("SDSC");
            break;
        case CARD_SDHC:
            Serial.print("MMSDHCC");
            break;
        default:
            Serial.print("ERROR");
            return false;
    }

    Serial.printf(" %d GB\r\n", (SD.cardSize() / 1073741824));
    return g_sdcrad_ok = true;
}

void sd_list(const char* path)
{
    Serial.printf("[ChappieCore] ls %s\r\n", path);
    File root = SD.open(path);
    File file = root.openNextFile();
    while (file) {
        Serial.printf("-- %s  (%d)\r\n", file.name(), file.size());
        file = root.openNextFile();
    }
    file.close();
    root.close();
}