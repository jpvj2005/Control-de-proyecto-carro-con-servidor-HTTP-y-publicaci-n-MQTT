#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

const char* ssid = "TU_WIFI";
const char* password = "TU_PASSWORD";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32/car/move";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
WebServer server(80);

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    mqttClient.connect(clientId.c_str());
    delay(2000);
  }
}

void handleStatus() {
  StaticJsonDocument<100> doc;
  doc["status"] = "OK";
  doc["message"] = "Servidor en ejecución";
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleMove() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Método no permitido\"}");
    return;
  }

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"JSON requerido\"}");
    return;
  }

  String body = server.arg("plain");
  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, body)) {
    server.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
    return;
  }

  String direction = doc["direction"];
  int speed = doc["speed"];
  int duration = doc["duration"];
  String clientIP = server.client().remoteIP().toString();

  if (duration < 1 || duration > 5) {
    server.send(400, "application/json", "{\"error\":\"duration debe ser 1-5\"}");
    return;
  }

  Serial.println("Instrucción recibida →");
  Serial.println(body);
  Serial.println("IP Cliente: " + clientIP);

  StaticJsonDocument<200> mqttDoc;
  mqttDoc["direction"] = direction;
  mqttDoc["speed"] = speed;
  mqttDoc["duration"] = duration;
  mqttDoc["client_ip"] = clientIP;

  String mqttMsg;
  serializeJson(mqttDoc, mqttMsg);
  mqttClient.publish(mqtt_topic, mqttMsg.c_str());

  StaticJsonDocument<150> r;
  r["message"] = "Instrucción recibida";
  r["client_ip"] = clientIP;

  String out;
  serializeJson(r, out);
  server.send(200, "application/json", out);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) delay(100);

  mqttClient.setServer(mqtt_server, mqtt_port);

  server.on("/status", handleStatus);
  server.on("/move", handleMove);
  server.begin();
}

void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();
  server.handleClient();
}

