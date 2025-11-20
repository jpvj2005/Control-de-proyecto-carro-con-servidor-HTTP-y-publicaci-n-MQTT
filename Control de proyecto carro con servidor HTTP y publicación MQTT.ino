#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// -------------------------------------------------------
// Pines y constantes
// -------------------------------------------------------
const int PIN_LUCES = 22;
const int PIN_BOCINA = 21;

const int ENA_IZQ = 15, IN1_IZQ = 4,  IN2_IZQ = 16;
const int ENA_DER = 19, IN1_DER = 17, IN2_DER = 18;

const int PWM_FREQ = 20000;
const int PWM_BITS = 10;
const int PWM_CH_I = 0;
const int PWM_CH_D = 1;

// -------------------------------------------------------
// WiFi / AP / DNS / HTTP
// -------------------------------------------------------
WebServer server(80);
DNSServer dns;
Preferences prefs;

String ssidGuardada, passGuardada;
const char* AP_SSID = "ESP32_Setup";
bool restartFlag = false;

// MQTT
WiFiClient espClient;
PubSubClient mqtt(espClient);
const char* mqttServer = "test.mosquitto.org";
const char* mqttTopic = "carro/mov";

// -------------------------------------------------------
// Motores
// -------------------------------------------------------
void detener() {
  digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, LOW);
  digitalWrite(IN1_DER, LOW); digitalWrite(IN2_DER, LOW);
  ledcWrite(PWM_CH_I, 0);
  ledcWrite(PWM_CH_D, 0);
}

void adelante(int p) {
  digitalWrite(IN1_IZQ, HIGH); digitalWrite(IN2_IZQ, LOW);
  digitalWrite(IN1_DER, HIGH); digitalWrite(IN2_DER, LOW);
  ledcWrite(PWM_CH_I, p);
  ledcWrite(PWM_CH_D, p);
}

void atras(int p) {
  digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, HIGH);
  digitalWrite(IN1_DER, LOW); digitalWrite(IN2_DER, HIGH);
  ledcWrite(PWM_CH_I, p);
  ledcWrite(PWM_CH_D, p);
}

void izquierda(int p) {
  digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, LOW);
  digitalWrite(IN1_DER, HIGH); digitalWrite(IN2_DER, LOW);
  ledcWrite(PWM_CH_I, 0);
  ledcWrite(PWM_CH_D, p);
}

void derecha(int p) {
  digitalWrite(IN1_IZQ, HIGH); digitalWrite(IN2_IZQ, LOW);
  digitalWrite(IN1_DER, LOW); digitalWrite(IN2_DER, LOW);
  ledcWrite(PWM_CH_I, p);
  ledcWrite(PWM_CH_D, 0);
}

// -------------------------------------------------------
// WiFi y storage
// -------------------------------------------------------
void cargarCredenciales() {
  prefs.begin("wifi", true);
  ssidGuardada = prefs.getString("ssid", "");
  passGuardada = prefs.getString("pass", "");
  prefs.end();
}

void guardarCredenciales(String s, String p) {
  prefs.begin("wifi", false);
  prefs.putString("ssid", s);
  prefs.putString("pass", p);
  prefs.end();
  ssidGuardada = s;
  passGuardada = p;
}

void borrarCredenciales() {
  prefs.begin("wifi", false);
  prefs.clear();
  prefs.end();
  ssidGuardada = "";
  passGuardada = "";
}

bool conectarGuardada() {
  if (ssidGuardada == "") return false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidGuardada.c_str(), passGuardada.c_str());

  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 8000) delay(200);

  return WiFi.status() == WL_CONNECTED;
}

void iniciarAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID);
  dns.start(53, "*", WiFi.softAPIP());
}

// -------------------------------------------------------
// HTML simple
// -------------------------------------------------------
String pagina() {
  return R"(
  <html><body>
  <h2>Config WiFi</h2>
  <button onclick='fetch("/scan").then(r=>r.json()).then(d=>alert(JSON.stringify(d)))'>Escanear</button>
  <form method='POST' action='/connect'>
    SSID:<input name='s'><br>
    PASS:<input name='p'><br>
    <button>Conectar</button>
  </form>
  <button onclick='fetch("/forget")'>Olvidar</button>
  </body></html>
  )";
}

// -------------------------------------------------------
// Endpoints HTTP
// -------------------------------------------------------
void hRoot() { server.send(200, "text/html", pagina()); }

void hScan() {
  int n = WiFi.scanNetworks();
  JsonDocument js;
  for (int i=0;i<n;i++) js.add(WiFi.SSID(i));
  String out;
  serializeJson(js,out);
  server.send(200,"application/json",out);
}

void hConnect() {
  String s = server.arg("s");
  String p = server.arg("p");

  WiFi.mode(WIFI_STA);
  WiFi.begin(s.c_str(), p.c_str());

  unsigned long t = millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t<8000) delay(200);

  if (WiFi.status()==WL_CONNECTED) {
    guardarCredenciales(s,p);
    server.send(200,"text/plain","OK conectado");
    restartFlag = true;
  } else {
    server.send(200,"text/plain","Fallo la conexión");
  }
}

void hForget() {
  borrarCredenciales();
  server.send(200,"text/plain","Credenciales borradas");
  delay(1500);
  ESP.restart();
}

void hMove() {
  JsonDocument doc;
  deserializeJson(doc, server.arg("plain"));

  String dir = doc["dir"];
  int speed  = doc["speed"] | 600;
  int dur    = doc["dur"]   | 500;

  if (dir=="adelante") adelante(speed);
  else if (dir=="atras") atras(speed);
  else if (dir=="izquierda") izquierda(speed);
  else if (dir=="derecha") derecha(speed);
  else { server.send(400,"text/plain","dir inválida"); return; }

  // MQTT
  String msg;
  serializeJson(doc,msg);
  mqtt.publish(mqttTopic,msg.c_str());

  delay(dur);
  detener();

  server.send(200,"application/json","{\"ok\":true}");
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LUCES, OUTPUT);
  pinMode(PIN_BOCINA, OUTPUT);

  pinMode(IN1_IZQ, OUTPUT);
  pinMode(IN2_IZQ, OUTPUT);
  pinMode(IN1_DER, OUTPUT);
  pinMode(IN2_DER, OUTPUT);

  ledcSetup(PWM_CH_I, PWM_FREQ, PWM_BITS);
  ledcAttachPin(ENA_IZQ, PWM_CH_I);
  ledcSetup(PWM_CH_D, PWM_FREQ, PWM_BITS);
  ledcAttachPin(ENA_DER, PWM_CH_D);

  cargarCredenciales();

  iniciarAP();

  mqtt.setServer(mqttServer,1883);

  server.on("/", hRoot);
  server.on("/scan", hScan);
  server.on("/connect", hConnect);
  server.on("/forget", hForget);
  server.on("/move", HTTP_POST, hMove);
  server.begin();
}

// -------------------------------------------------------
// Loop
// -------------------------------------------------------
void loop() {
  dns.processNextRequest();
  server.handleClient();

  if (WiFi.status()==WL_CONNECTED && !mqtt.connected()) {
    mqtt.connect("CarClientMini");
  }
  mqtt.loop();

  if (restartFlag) {
    restartFlag = false;
    delay(300);
    ESP.restart();
  }
}
