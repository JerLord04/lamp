#if !defined(ESP8266)
#error This code is intended to run only on the ESP8266 boards ! Please check your Tools->Board setting.
#endif
#define _WEBSOCKETS_LOGLEVEL_2
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient_Generic.h>
#include <SocketIOclient_Generic.h>
#include <Hash.h>
ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;
IPAddress serverIP(192, 168, 1, 7);
uint16_t serverPort = 3000;
String statusData = "";
uint8_t relay = D1;

void socketIOEvent(const socketIOmessageType_t& type, uint8_t* payload, const size_t& length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      Serial.println("[IOc] Disconnected");
      break;
    case sIOtype_CONNECT:
      Serial.print("[IOc] Connected to url: ");
      Serial.println((char*)payload);
      socketIO.send(sIOtype_CONNECT, "/");
      break;
    case sIOtype_EVENT:
      {
        Serial.print("[IOc] Get event: ");
        statusData = String((char*)payload);
        int indexDot = statusData.indexOf('.');
        int length = statusData.length();
        String sub_S = statusData.substring(indexDot + 1, length - 2);
        if (sub_S == "Light ON1") {
          digitalWrite(relay, 0);
          Serial.println("Light bulb has been on");
        } else if (sub_S == "Light OFF1") {
          digitalWrite(relay, 1);
          Serial.println("Light bulb has been off");
        }
      }
      // Serial.println(index);
      break;
    case sIOtype_ACK:
      Serial.print("[IOc] Get ack: ");
      Serial.println(length);
      hexdump(payload, length);
      break;
    case sIOtype_ERROR:
      Serial.print("[IOc] Get error: ");
      Serial.println(length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_EVENT:
      Serial.print("[IOc] Get binary: ");
      Serial.println(length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_ACK:
      Serial.print("[IOc] Get binary ack: ");
      Serial.println(length);
      hexdump(payload, length);
      break;
    case sIOtype_PING:
      Serial.println("[IOc] Get PING");
      break;
    case sIOtype_PONG:
      Serial.println("[IOc] Get PONG");
      break;
    default:
      break;
  }
}

void setup() {
  pinMode(relay, OUTPUT);
  Serial.begin(115200);
  while (!Serial);
  Serial.print("\nStart ESP8266_WebSocketClientSocketIO on ");
  Serial.println(ARDUINO_BOARD);
  Serial.println(WEBSOCKETS_GENERIC_VERSION);
  if (WiFi.getMode() & WIFI_AP) {
    WiFi.softAPdisconnect(true);
  }
  WiFiMulti.addAP("Meesuk_2.4G", "51552105315");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("WebSockets Client started @ IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Connecting to WebSockets Server @ IP address: ");
  Serial.print(serverIP);
  Serial.print(", port: ");
  Serial.println(serverPort);
  socketIO.setReconnectInterval(10000);
  socketIO.setExtraHeaders("Authorization: 1234567890");
  socketIO.begin(serverIP, serverPort);
  socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;

void loop() {
  uint64_t now = millis();
  socketIO.loop();
  if (now - messageTimestamp > 30000) {
    messageTimestamp = now;
    String txt = "Connection is openned";
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();
    array.add("light_bulb_control_livingRoom");
    JsonObject param1 = array.createNestedObject();
    param1["battery_percent"] = (String)txt;
    String output;
    serializeJson(doc, output);
    socketIO.sendEVENT(output);
    Serial.println(output);
  }
}
