//Your wifi connection
const char* ssid = "ssid";
const char* password = "password";
/*Change the port to a random (free) port for security.*/
const int webSocketPort = 2339;

//Websocket Initialization
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsServer.h>

WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(webSocketPort);

//JSON Parsing and Key Conversion
#include <Arduino_JSON.h>
int playerpause = 2;
int oldplayerpause;
uint16_t counter = 1;

/*  Struct to Hold Each Element of the JsonToUSB_HID_Map */
struct JSONMethodToCecType {
  char JSONMethod[24];
  char JSONAction[14];
  bool KeyboardAction;
  uint8_t USBHID;
};

struct IPAddressFail {
  int FailedAttempts;
  time_t lastFailure;
};

/* Try to block users who are trying to brute force a connection */
IPAddressFail IPAddressFailures[254];

/*  JsonToUSB_HID_Map */
const JSONMethodToCecType JSONMethodToCec[] = {
  {"Input.Select", "", 1, 0x28},
  {"Input.Left", "", 1, 0x50},
  {"Input.Right", "", 1, 0x4F},
  {"Input.Up", "", 1, 0x52},
  {"Input.Down", "", 1, 0x51},
  {"Input.ShowOSD", "", 1, 0x10},
  {"Input.Info", "", 0, 0},
  {"Input.Back", "", 1, 0x29},
  {"Input.Home", "", 1, 0x29},
  {"Input.SendText", "", 0, 0},
  {"VideoLibrary.Scan", "", 0, 0},
  {"Input.ContextMenu", "", 0, 0},
  {"Player.GetActivePlayers", "", 0, 0},
  //Pause then play
  //Might want o set a delay to reset the pause or stop after 30 mins
  {"Player.PlayPause", "1", 0, 0xB1},
  {"Player.PlayPause", "2", 0, 0xB0},
  {"Player.Stop", "", 0, 0xB7},
  {"Input.ExecuteAction", "play", 0, 0xB0},
  {"Input.ExecuteAction", "mute", 0, 0xE2},
  {"Input.ExecuteAction", "stepback", 0, 0xB4},
  {"Input.ExecuteAction", "stepforward", 0, 0xB3},
  {"Input.ExecuteAction", "skipprevious", 0, 0xB7},
  {"Input.ExecuteAction", "skipnext", 0, 0xB5},
  {"Input.ExecuteAction", "volumeup", 0, 0xE9},
  {"Input.ExecuteAction", "volumedown", 0, 0xEA}
};

//Keyboard Initialize
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

struct inputConsumer_t
{
  uint16_t ConsumerControl;                          // Value = 0 to 572
};

struct inputKeyboard_t
{
  uint8_t  KB_KeyboardKeyboardLeftControl  : 1;       // Usage 0x000700E0: Keyboard Left Control, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardLeftShift  : 1;         // Usage 0x000700E1: Keyboard Left Shift, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardLeftAlt    : 1;           // Usage 0x000700E2: Keyboard Left Alt, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardLeftGui    : 1;           // Usage 0x000700E3: Keyboard Left GUI, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardRightControl : 1;      // Usage 0x000700E4: Keyboard Right Control, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardRightShift   : 1;        // Usage 0x000700E5: Keyboard Right Shift, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardRightAlt   : 1;          // Usage 0x000700E6: Keyboard Right Alt, Value = 0 to 1
  uint8_t  KB_KeyboardKeyboardRightGui   : 1;          // Usage 0x000700E7: Keyboard Right GUI, Value = 0 to 1
  uint8_t  Key;                                 // Value = 0 to 101
};

const uint8_t keyboardHidDescriptor[] = {
  0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
  0x09, 0x01,                    // USAGE (Consumer Control)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x85, 0x01,                    //   REPORT_ID (1)
  0x19, 0x00,                    //   USAGE_MINIMUM (Unassigned)
  0x2a, 0x3c, 0x02,              //   USAGE_MAXIMUM (AC Format)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
  0x26, 0x3c, 0x02,              //   LOGICAL_MAXIMUM (572)
  0x95, 0x01,                    //   REPORT_COUNT (1)
  0x75, 0x10,                    //   REPORT_SIZE (16)
  0x81, 0x00,                    //   INPUT (Data,Var,Abs)
  0xc0,                          // END_COLLECTION
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
  0x09, 0x06,                    // USAGE (Keyboard)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x85, 0x02,                    //   REPORT_ID (2)
  0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
  0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,                    //   REPORT_SIZE (1)
  0x95, 0x08,                    //   REPORT_COUNT (8)
  0x81, 0x02,                    //   INPUT (Data,Var,Abs)
  0x95, 0x01,                    //   REPORT_COUNT (1)
  0x75, 0x08,                    //   REPORT_SIZE (8)
  0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
  0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
  0xc0                           // END_COLLECTION
};

static inputConsumer_t consumer_Report{};
static inputKeyboard_t keyboard_report{}; // sent to PC
BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;
BLECharacteristic* inputMedia;
BLECharacteristic* outputMedia;
bool BLEconnected = false;

void keyboard_control(uint8_t Key, const char* JSONMethod) {
  Serial.print("Sending key code :");
  Serial.println(Key);
  inputKeyboard_t keyboard{};
  if (strcmp(JSONMethod, "Input.Home") == 0) {
    keyboard.KB_KeyboardKeyboardLeftControl = 1;
  }
  keyboard.Key = Key;
  input->setValue((uint8_t*)&keyboard, sizeof(keyboard));
  input->notify();
  input->setValue((uint8_t*)(&keyboard_report), sizeof(keyboard_report));
  input->notify();
  delay(10);
}

void media_control(uint8_t Key) {
  Serial.print("Sending media code :");
  Serial.println(Key);
  inputConsumer_t keyboard{};
  keyboard.ConsumerControl = Key;
  inputMedia->setValue((uint8_t*)&keyboard, sizeof(keyboard));
  inputMedia->notify();
  inputMedia->setValue((uint8_t*)(&keyboard_report), sizeof(keyboard_report));
  inputMedia->notify();
  delay(10);
}

void processUSBHID(JSONVar JSONMethod, const char* JSONAction) {
  int JSONMethodToCecLength = 24;
  for (int i = 0; i < JSONMethodToCecLength; i++) {
    if ((strcmp(JSONMethodToCec[i].JSONMethod, (const char *)JSONMethod) == 0) && (strcmp(JSONMethodToCec[i].JSONAction, (const char *)JSONAction) == 0)) {
      Serial.print("Incoming JSON Request :");
      Serial.println((const char *)JSONMethod);
      if (JSONMethodToCec[i].KeyboardAction == 1) {
        keyboard_control(JSONMethodToCec[i].USBHID, (const char *)JSONMethod);
      } else {
        media_control(JSONMethodToCec[i].USBHID);
      }
      return;
    }
  }
}

class BLECallback : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BLEconnected = true;
      Serial.println("Bluetooth Connected");
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(true);

      BLE2902* descv = (BLE2902*)inputMedia->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      descv->setNotifications(true);
    }

    void onDisconnect(BLEServer* pServer) {
      BLEconnected = false;
      Serial.println("BLE Disonnected");
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(false);

      BLE2902* descv = (BLE2902*)inputMedia->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      descv->setNotifications(false);
    }
};

void keyTaskServer(void*) {
  BLEDevice::init("NVIDIA Control Device");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLECallback());

  hid = new BLEHIDDevice(pServer);
  inputMedia = hid->inputReport(1); // <-- input REPORTID from report map
  outputMedia = hid->outputReport(1); // <-- output REPORTID from report map

  input = hid->inputReport(2); // <-- input REPORTID from report map
  output = hid->outputReport(2); // <-- output REPORTID from report map

  std::string name = "ElectronicCats";
  hid->manufacturer()->setValue(name);

  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00, 0x02);

  hid->reportMap((uint8_t*)keyboardHidDescriptor, sizeof(keyboardHidDescriptor));
  hid->startServices();

  BLESecurity *pSecurity = new BLESecurity();
  //  pSecurity->setKeySize();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  hid->setBatteryLevel(100);
  yield();
  delay(portMAX_DELAY);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("WEBSOCKET: [%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WEBSOCKET: [%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        /* A simple system to block IP Addresses that fail connecting 5 times for a 5 minute period */
        int IPAddressPosition = ip[3]-1;
        if ((IPAddressPosition>=0) && (IPAddressPosition<253)) {
          int FailedAttempts = IPAddressFailures[IPAddressPosition].FailedAttempts;
          if (FailedAttempts<5) {
            if (strcmp((const char *)payload, "/jsonrpc") != 0) {
              Serial.println("WEBSOCKET: Client has not connected to the correct path, disconnecting...");
              Serial.printf("IP Address %d.%d.%d.%d has failed connecting\n",ip[0], ip[1], ip[2], ip[3]);
  
              IPAddressFailures[IPAddressPosition].lastFailure = time(nullptr);
              IPAddressFailures[IPAddressPosition].FailedAttempts=FailedAttempts+1;
              if ((FailedAttempts+1)==5) {
                Serial.printf("IP Address %d.%d.%d.%d has failed connecting 5 times, it will now be blocked for 300 seconds\n",ip[0], ip[1], ip[2], ip[3]);
              } 
              webSocket.disconnect(num);
            }        
          } else {
            Serial.println("Disconnecting client due to being banned");
            if ((IPAddressFailures[IPAddressPosition].lastFailure+300)<time(nullptr)) {
              Serial.println("It has now been over five minutes, disabling client block...");
              IPAddressFailures[IPAddressPosition].FailedAttempts = 0;
            }
            webSocket.disconnect(num);
          }
        }
      }
      break;
    case WStype_TEXT:
      {
        unsigned long startTime;
        startTime = millis();

        Serial.printf("WEBSOCKET: [%u] get Text: %s\n", num, payload);
        Serial.printf("Counter: %d\n",counter);
        
        counter = counter + 1;
        /*  I want to avoid filling up stack space, and this is only needed to test latency */
        if (counter==10000) {
          counter=1;
        }
        JSONVar myObject = JSON.parse((const char *)payload);

        if (strcmp(myObject["method"], "Player.GetActivePlayers") == 0) {
          webSocket.sendTXT(num, "{\"id\": 1, \"jsonrpc\": \"2.0\", \"result\": [ { \"playerid\": 1, \"type\": \"video\" } ]}");
        } else {
          webSocket.sendTXT(num, "{\"id\":1,\"jsonrpc\":\"2.0\",\"result\":\"OK\"}");
        }

        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          return;
        }
        if (BLEconnected == false) {
          Serial.println("Bluetooth not connected");
          return;
        }
        if (!myObject.hasOwnProperty("method")) {
          Serial.println("JSON parse cannot find method");
          //Need to send a response
          return;
        }
        Serial.println("Correctly parsed JSON");
        if (strcmp(myObject["method"], "Player.GetActivePlayers") == 0) {
          oldplayerpause = playerpause;
          playerpause += 1;
          if (playerpause == 3) {
            playerpause = 1;
          }
          Serial.printf("Detected GetActivePlayers, switching from %d to %d\n",oldplayerpause,playerpause);
          return;
        } else if (strcmp(myObject["method"], "Player.PlayPause") == 0) {
          char buffer[1];
          itoa(playerpause, buffer, 10);
          processUSBHID(myObject["method"], buffer);
        } else if (strcmp(myObject["method"], "Input.ExecuteAction") == 0) {
          if (!((myObject.hasOwnProperty("params")) && (myObject["params"].hasOwnProperty("action")))) {
            Serial.println("JSON parse cannot find params or anction and is required for Input.ExecuteAction");
            return;
          }
          processUSBHID(myObject["method"], myObject["params"]["action"]);
        } else {
          processUSBHID(myObject["method"], "");
        }
        Serial.printf("Function time was %d\n",millis() - startTime);
        break;
      }
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }

}

void setup() {
  Serial.begin(115200);
  /* Populate the list of struct array with IP addresses for a /24 subnet this will need to be modified if you are on something else */
  for (int i=0;i<253;i++) {
    IPAddressFailures[i].FailedAttempts = 0;
  }
  
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("WEBSOCKET: [SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("Wifi Connected");
  xTaskCreate(keyTaskServer, "server", 20000, NULL, 5, NULL);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}
