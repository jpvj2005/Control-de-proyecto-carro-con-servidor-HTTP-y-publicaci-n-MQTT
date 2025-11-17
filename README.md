# Control-de-proyecto-carro-con-servidor-HTTP-y-publicaci-n-MQTT
Este proyecto implementa un servidor HTTP en un ESP32 que permite enviar instrucciones de movimiento (adelante, atrás, izquierda, derecha) junto con velocidad y duración.
Las instrucciones no activan motores reales, sino que únicamente se imprimen en el Serial Monitor y se publican en un servidor MQTT.

El propósito principal es mostrar cómo:

Exponer endpoints HTTP desde el ESP32.

#Validar datos recibidos (velocidad/dirección/duración).

Limitar duración a un máximo de 5 segundos.

Publicar las instrucciones en un broker MQTT.

Enviar también la IP del cliente que hizo la petición.

📝 Explicación general del código

El sistema está construido usando:

WiFi.h para la conexión a la red WiFi.

WebServer.h para manejar peticiones HTTP.

ArduinoJson para procesar JSON.

PubSubClient para la conexión MQTT.

El flujo del dispositivo es:

Conectar el ESP32 a WiFi.

Levantar un servidor HTTP en el puerto 80.

Manejar dos endpoints principales:

/status

/move

Cada instrucción enviada al endpoint /move:

Se valida.

Se imprime en el Serial Monitor.

Se envía al broker MQTT.

El payload publicado incluye la IP del cliente.

🌐 Endpoints de la API
GET /status

Devuelve el estado del servidor HTTP y MQTT.

Ejemplo de respuesta:
{
  "status": "ok",
  "mqtt_connected": true
}

POST /move

Recibe instrucciones para controlar el carro.
Parámetros esperados:

{
  "direction": "forward | back | left | right",
  "speed": 0–255,
  "duration": 1–5
}

✔️ Reglas

duration no puede exceder 5 segundos.

speed debe ser un número válido.

La instrucción NO mueve motores reales: solo se imprime en Serial.

Ejemplo de solicitud:
{
  "direction": "forward",
  "speed": 200,
  "duration": 3
}

Ejemplo de respuesta:
{
  "received": true,
  "mqtt_published": true
}

📡 Publicación MQTT

Cada instrucción enviada al endpoint HTTP /move se publica en un tópico definido en el código.

Ejemplo de mensaje publicado:

{
  "direction": "forward",
  "speed": 200,
  "duration": 3,
  "client_ip": "192.168.1.45"
}

🖥️ Ejemplo del mensaje en Serial

Cuando llega una instrucción, el ESP32 imprime algo como:

--- INSTRUCCIÓN RECIBIDA ---
Dirección: forward
Velocidad: 200
Duración: 3
IP del cliente: 192.168.1.45
-----------------------------
