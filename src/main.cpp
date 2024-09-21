#include <Arduino.h>
#include <CorsairLightingProtocol.h>
#include <FastLED.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <CustomFastLEDControllerStorageEEPROM.h>
#include <CustomCorsairLightingFirmwareStorageEEPROM.h>

#define EEPROM_I2C_ADDRESS 0x50

#define MODE_SEL_PIN_0 1
#define MODE_SEL_PIN_1 2
#define MODE_SEL_PIN_2 3

#define SDA_PIN 4
#define SCL_PIN 5

#define DATA_PIN_CHANNEL_1 16
#define DATA_PIN_CHANNEL_2 29

CRGB ledsChannel1[96];
CRGB ledsChannel2[96];

// DeviceID deviceID = {0x9A, 0xDA, 0xA7, 0x8C};
// CorsairLightingFirmwareStorageStatic firmwareStorage(deviceID);

I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC256);
CustomCorsairLightingFirmwareStorageEEPROM firmwareStorage(ee);
CorsairLightingFirmware firmware(CORSAIR_LIGHTING_NODE_CORE, &firmwareStorage);
CustomFastLEDControllerStorageEEPROM storage(ee);
FastLEDController ledController(&storage);
CorsairLightingProtocolController cLP(&ledController, &firmware);
CorsairLightingProtocolTinyUSBHID cHID(&cLP);

void loadMode()
{
	PinStatus m0 = digitalRead(MODE_SEL_PIN_0);
	PinStatus m1 = digitalRead(MODE_SEL_PIN_1);
	PinStatus m2 = digitalRead(MODE_SEL_PIN_2);

	Serial.printf("Mode Sel: %d %d %d\n", m0, m1, m2);
}

void setup()
{
	Wire.setSDA(SDA_PIN);
	Wire.setSCL(SCL_PIN);
	Wire.begin();
	ee.begin();

	cHID.setup();

	Serial.begin(115200);

	FastLED.addLeds<WS2812B, DATA_PIN_CHANNEL_1, GRB>(ledsChannel1, 96);
	FastLED.addLeds<WS2812B, DATA_PIN_CHANNEL_2, GRB>(ledsChannel2, 96);

	ledController.addLEDs(0, ledsChannel1, 96);
	ledController.addLEDs(1, ledsChannel2, 96);

	pinMode(MODE_SEL_PIN_0, INPUT_PULLUP);
	pinMode(MODE_SEL_PIN_1, INPUT_PULLUP);
	pinMode(MODE_SEL_PIN_2, INPUT_PULLUP);
}
unsigned long INTERVAL = 1000;
unsigned long lastUpdate = 0;
void loop()
{
	cHID.update();

	if (ledController.updateLEDs())
	{
		FastLED.show();
	}

	if (millis() - lastUpdate > INTERVAL)
	{
		loadMode();
		lastUpdate = millis();
	}
}
