#include <Arduino.h>
#include <CorsairLightingProtocol.h>
#include <FastLED.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <CustomFastLEDControllerStorageEEPROM.h>
#include <CustomCorsairLightingFirmwareStorageEEPROM.h>

#define EEPROM_I2C_ADDRESS 0x50

#define MODE_SEL_PIN_0 3
#define MODE_SEL_PIN_1 2
#define MODE_SEL_PIN_2 1

#define MODE_0 "111" // CORSAIR_LIGHTING_NODE_PRO
#define MODE_1 "011" // CORSAIR_COMMANDER_PRO
#define MODE_2 "001" // CORSAIR_LIGHTING_NODE_CORE
#define MODE_3 "000" // CORSAIR_SMART_LIGHTING_CONTROLLER
#define MODE_4 "100" // CORSAIR_SMART_LIGHTING_TOWERS

#define SDA_PIN 4
#define SCL_PIN 5

#define DATA_PIN_CHANNEL_1 16
#define DATA_PIN_CHANNEL_2 29

char MODE[4];

CRGB ledsChannel1[96];
CRGB ledsChannel2[96];

corsair_product_enum_t PRODUCT = CORSAIR_LIGHTING_NODE_PRO;

I2C_eeprom ee(EEPROM_I2C_ADDRESS, I2C_DEVICESIZE_24LC256);
CustomCorsairLightingFirmwareStorageEEPROM firmwareStorage(ee);
CorsairLightingFirmware *firmware = nullptr;
CustomFastLEDControllerStorageEEPROM *storage = nullptr;
FastLEDController *ledController = nullptr;
CorsairLightingProtocolController *cLP = nullptr;
CorsairLightingProtocolTinyUSBHID *cHID = nullptr;

void initMode()
{
	pinMode(MODE_SEL_PIN_0, INPUT_PULLUP);
	pinMode(MODE_SEL_PIN_1, INPUT_PULLUP);
	pinMode(MODE_SEL_PIN_2, INPUT_PULLUP);

	PinStatus m0 = digitalRead(MODE_SEL_PIN_0);
	PinStatus m1 = digitalRead(MODE_SEL_PIN_1);
	PinStatus m2 = digitalRead(MODE_SEL_PIN_2);

	Serial.printf("Mode Sel: %d %d %d\n", m0, m1, m2);

	MODE[0] = m0 ? '1' : '0';
	MODE[1] = m1 ? '1' : '0';
	MODE[2] = m2 ? '1' : '0';
	MODE[3] = '\0';

	if (strcmp(MODE, MODE_1) == 0)
	{
		PRODUCT = CORSAIR_COMMANDER_PRO;
	}
	else if (strcmp(MODE, MODE_2) == 0)
	{
		PRODUCT = CORSAIR_LIGHTING_NODE_CORE;
	}
	else if (strcmp(MODE, MODE_3) == 0)
	{
		PRODUCT = CORSAIR_SMART_LIGHTING_CONTROLLER;
	}
	else if (strcmp(MODE, MODE_4) == 0)
	{
		PRODUCT = CORSAIR_SMART_LIGHTING_TOWERS;
	}
	else
	{
		PRODUCT = CORSAIR_LIGHTING_NODE_PRO;
	}
}

void setup()
{
	Serial.begin(115200);
	Wire.setSDA(SDA_PIN);
	Wire.setSCL(SCL_PIN);
	Wire.begin();
	ee.begin();

	initMode();

	firmware = new CorsairLightingFirmware(PRODUCT, &firmwareStorage);
	storage = new CustomFastLEDControllerStorageEEPROM(ee);
	ledController = new FastLEDController(storage);
	cLP = new CorsairLightingProtocolController(ledController, firmware);
	cHID = new CorsairLightingProtocolTinyUSBHID(cLP);

	cHID->setup();

	FastLED.addLeds<WS2812B, DATA_PIN_CHANNEL_1, GRB>(ledsChannel1, 96);
	FastLED.addLeds<WS2812B, DATA_PIN_CHANNEL_2, GRB>(ledsChannel2, 96);

	ledController->addLEDs(0, ledsChannel1, 96);
	ledController->addLEDs(1, ledsChannel2, 96);
}

void loop()
{
	if (cHID)
	{
		cHID->update();

		if (ledController->updateLEDs())
		{
			FastLED.show();
		}
	}
}