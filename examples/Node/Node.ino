#include <TheThingsNetwork.h>
#include <TheThingsNode.h>

// Set your AppEUI and AppKey
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

#define loraSerial Serial1
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
TheThingsNode *node;

#define PORT_SETUP 1
#define PORT_INTERVAL 2
#define PORT_MOTION 3
#define PORT_BUTTON 4

void setup() {
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000);

  // Config Node
  node = TheThingsNode::setup();
  node->configLight(true);
  node->configInterval(true, 60000);
  node->onWake(wake);
  node->onInterval(interval);
  node->onSleep(sleep);
  node->onMotionStart(onMotionStart);
  node->onButtonRelease(onButtonRelease);

  // Test sensors and set LED to GREEN if it works
  node->showStatus();
  node->setColor(TTN_GREEN);

  debugSerial.println("-- TTN: STATUS");
  ttn.showStatus();

  debugSerial.println("-- TTN: JOIN");
  ttn.join(appEui, appKey);

  debugSerial.println("-- SEND: SETUP");
  sendData(PORT_SETUP);
}

void loop() {
  node->loop();
}

void interval() {
  node->setColor(TTN_BLUE);
  
  debugSerial.println("-- SEND: INTERVAL");
  sendData(PORT_INTERVAL);
}

void wake() {
  node->setColor(TTN_GREEN);
}

void sleep() {
  node->setColor(TTN_BLACK);
}

void onMotionStart() {
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: MOTION");
  sendData(PORT_MOTION);
}

void onButtonRelease(unsigned long duration) {
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: BUTTON");
  debugSerial.println(duration);

  sendData(PORT_BUTTON);
}

void sendData(uint8_t port) {
  ttn.showStatus();
  node->showStatus();

  byte* bytes;
  byte payload[6];

  uint16_t battery = node->getBattery();
  bytes = (byte*) &battery;
  payload[0] = bytes[1];
  payload[1] = bytes[0];

  uint16_t light = node->getLight();
  bytes = (byte*) &light;
  payload[2] = bytes[1];
  payload[3] = bytes[0];

  int16_t temperature = round(node->getTemperatureAsFloat() * 100);
  bytes = (byte*) &temperature;
  payload[4] = bytes[1];
  payload[5] = bytes[0];

  ttn.sendBytes(payload, sizeof(payload), port);
}
