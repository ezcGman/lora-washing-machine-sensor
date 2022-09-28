#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include "EmonLib.h"

#include "utils.h"
#include "config.h"
#include "lora-config.h"

// LoRa pins
#define PIN_LORA_NSS 15
#define PIN_LORA_RST -1
#define PIN_LORA_DIO0 3

// Multiplexer pins
#define DISABLE_CHANNELS 16
// A: IO4, B: IO5, C: IO2
#define SELECT_A 4
#define SELECT_B 5
#define SELECT_C 2
const int selectPins[3] = {SELECT_A, SELECT_B, SELECT_C};

bool loraInitDone = false;
const byte maxConnTries = 15;

EnergyMonitor emon;
const byte ctClampCalibration = 14;

const byte voltage = 235; // Hardcode. Our home voltage is always around 235-238v
double sumAmps = 0;
float amps = 0;
float watts = 0;

void connectLoRa() {
  if (!loraInitDone) {
    int loraConnTries = 0;

    // LoRa.setSPIFrequency(100000);
    // delay(1000);
    LoRa.setPins(PIN_LORA_NSS, PIN_LORA_RST, PIN_LORA_DIO0);
    delay(1000);

    if (Serial) Serial.print("Connecting LoRa...");
    while (!LoRa.begin(868E6) && loraConnTries < maxConnTries) {
      if (Serial) Serial.print(".");
      loraConnTries++;
      delay(1000);
    }
    if (Serial) Serial.println("Done!");

    if (loraConnTries >= maxConnTries) {
      ESP.restart();
    }

    loraInitDone = true;
    LoRa.setSyncWord(LORA_SYNC_WORD);
  }
}

void turnOffWifi() {
  if (Serial) Serial.println("Turning off Wi-Fi...");
  // WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  byte waitTries = 0;
  // Give it a max of 30 seconds to turn off, after we just accept it didn't work...
  while (WiFi.status() != WL_NO_SHIELD && WiFi.status() != WL_DISCONNECTED && waitTries++ <= 30) {
    if (Serial) Serial.println(WiFi.status());
    delay(1000);
  }
}

void enableOutput(int output)
{
  switch (output) {
    case 0:
      digitalWrite(SELECT_A, LOW); digitalWrite(SELECT_B, LOW); digitalWrite(SELECT_C, LOW);
      break;

    case 1:
      digitalWrite(SELECT_A, HIGH); digitalWrite(SELECT_B, LOW); digitalWrite(SELECT_C, LOW);
      break;
  }

  delay(100);
}


void setup() {
  #ifdef ENABLE_SERIAL_DEBUG
  Serial.begin(115200);
  while (!Serial);
  #endif

  // ----- Pin setup, Wi-Fi connect, sensor init... -----
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  turnOffWifi();

  connectLoRa();

  // Set up the select pins as outputs:
  for (int i=0; i < 3; i++) {
    pinMode(selectPins[i], OUTPUT);
  }
  pinMode(DISABLE_CHANNELS, OUTPUT);
  digitalWrite(DISABLE_CHANNELS, LOW);

  emon.current(A0, ctClampCalibration);

  // Give the LoRa module time to settle
  delay(1000);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (loraInitDone) {
    byte deviceId = LORA_DEVICE_ID;
    for (int i = 0; i <= 1; i++) {
      enableOutput(i);
      if (Serial) {
        Serial.printf("Sensor #%d:", i);
        Serial.println("");
      }

      sumAmps = 0;
      for (int j = 0; j <= 9; j++) {
        amps = emon.calcIrms(1480);
        sumAmps += amps;

        if (Serial) {
          Serial.printf("%f Amps - %f Watts", amps, amps*voltage);
          Serial.println("");
        }
    
        delay(10);
      }
      amps = sumAmps/10;
      watts = amps * voltage;

      LoRaMessagePowerConsumption *loraMessage = new LoRaMessagePowerConsumption;
      loraMessage->voltage = voltage;
      loraMessage->amps = amps;
      loraMessage->watts = watts;

      if (i == 1) deviceId = LORA_DEVICE_ID_B;
      sendLoRaMessage(LORA_MESSAGE_ID_POWER_CONSUMPTION, loraMessage, LORA_GATEWAY_ID, deviceId);
    }
  }

  delay(10000);
}
