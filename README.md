# 🚗 Control de Proyecto – Carro con Servidor HTTP y Publicación MQTT

Este proyecto implementa un **servidor HTTP en un ESP32** que permite recibir instrucciones de movimiento  
(adelante, atrás, izquierda, derecha) junto con **velocidad** y **duración**, sin activar motores reales.  
Las instrucciones:

- Se imprimen en el **Serial Monitor**.
- Se publican en un **broker MQTT**, incluyendo la **IP del cliente** que hizo la petición.

Su objetivo es demostrar comunicación **HTTP + MQTT**, validación de datos y construcción de un sistema IoT básico.

---

## 📡 Tecnologías y librerías utilizadas

El proyecto usa:

- **WiFi.h** → conexión del ESP32 a la red WiFi  
- **WebServer.h** → manejo de endpoints HTTP  
- **ArduinoJson** → procesar JSON entrante/saliente  
- **PubSubClient** → comunicación MQTT

---

## 🔁 Flujo general del sistema

1. El ESP32 se conecta a la red WiFi.
2. Inicia un servidor HTTP en el puerto 80.
3. Expone dos endpoints principales:
   - **GET /status**
   - **POST /move**
4. Cuando recibe una instrucción en `/move`:
   - Valida la información.
   - La imprime en Serial.
   - La publica en un broker MQTT.
   - Incluye siempre la IP del cliente.

---

# 🌐 Endpoints de la API

---

## ✔️ **GET /status**

Devuelve el estado del servidor HTTP y la conexión MQTT.

### **Ejemplo de respuesta**
```json
{
  "status": "ok",
  "mqtt_connected": true
}
```

## 🚀 POST /move
Recibe una instrucción de movimiento con los siguientes parámetros:

Campo	Tipo	Descripción
direction	string	“adelante”, “atras”, “izquierda”, “derecha”
speed	number	Velocidad del movimiento (0–1023 recomendado)
duration	number	Duración en ms (máx. 5000 ms)

Ejemplo de petición
```json
{
  "direction": "adelante",
  "speed": 600,
  "duration": 2000
}
```
Ejemplo de respuesta
```json
{
  "received": true,
  "client_ip": "192.168.0.25",
  "published_mqtt": true
}
```
## 📤 Publicación MQTT
Cada instrucción recibida en /move se publica automáticamente en un broker MQTT.

📌 Tema usado
```bash
carro/control
```
🔎 Ejemplo de payload publicado
```json
{
  "direction": "adelante",
  "speed": 600,
  "duration": 2000,
  "client_ip": "192.168.0.25",
  "timestamp": 1737165821
}
```
