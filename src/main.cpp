#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Налаштування вашої Wi-Fi мережі
const char *ssid = "";
const char *password = "";

// Створюємо вебсервер на 80 порту
ESP8266WebServer server(80);

// Конфігурація пінів для 3 каналів реле
const int relay1 = 4;  // GPIO 4  | D2
const int relay2 = 14; // GPIO 14 | D5
const int relay3 = 12; // GPIO 12 | D6

// Стани реле
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;

// Стани та таймери для неблокуючого підключення Wi-Fi
enum WifiState
{
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_FAILED
};
WifiState wifiState = WIFI_CONNECTING;

unsigned long wifiStartAttempt = 0;
unsigned long lastWifiCheck = 0;
const unsigned long wifiTimeout = 30000; // Таймаут підключення 30 секунд

// Функція генерації HTML-сторінки
void handleRoot()
{
  String html = "<!DOCTYPE html><html lang='uk'>";
  html += "<head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Smart Home Gateway | Non-Blocking</title>";
  html += "<style>";
  html += "*{box-sizing:border-box;margin:0;padding:0;}";
  html += "body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#eaf2ff,#dce5f2);color:#172033;min-height:100vh;padding:20px;}";
  html += ".container{max-width:600px;margin:0 auto;background:white;border-radius:16px;padding:25px;box-shadow:0 6px 18px rgba(20,45,85,0.09);}";
  html += "h2{text-align:center;color:#183b70;margin-bottom:20px;}";
  html += ".card{background:#f4f7fb;border-radius:12px;padding:15px;margin-bottom:15px;display:flex;justify-content:space-between;align-items:center;}";
  html += ".info h3{font-size:16px;color:#183b70;margin-bottom:4px;}";
  html += ".info p{font-size:12px;color:#7b8798;}";
  html += ".badge{padding:5px 12px;border-radius:20px;font-size:11px;font-weight:bold;margin-right:15px;}";
  html += ".on{background:#d9f8e6;color:#08783c;}";
  html += ".off{background:#ffe1e4;color:#c6283d;}";
  html += ".btn{padding:10px 20px;border-radius:8px;text-decoration:none;color:white;font-size:13px;font-weight:bold;text-align:center;min-width:100px;}";
  html += ".btn-on{background:#1769d2;}";
  html += ".btn-off{background:#c5223d;}";
  html += ".footer{text-align:center;margin-top:20px;font-size:11px;color:#8892a0;}";
  html += "</style></head><body>";

  html += "<div class='container'>";
  html += "<h2>Smart Home Gateway</h2>";

  // Канал 1
  html += "<div class='card'><div class='info'><h3>Канал 01 (Мотор/Насос)</h3><p>GPIO 4 | D2</p></div>";
  if (relay1State)
  {
    html += "<div><span class='badge on'>ON</span><a class='btn btn-off' href='/r1/off'>Вимкнути</a></div>";
  }
  else
  {
    html += "<div><span class='badge off'>OFF</span><a class='btn btn-on' href='/r1/on'>Увімкнути</a></div>";
  }
  html += "</div>";

  // Канал 2
  html += "<div class='card'><div class='info'><h3>Канал 02 (Вентилятор)</h3><p>GPIO 14 | D5</p></div>";
  if (relay2State)
  {
    html += "<div><span class='badge on'>ON</span><a class='btn btn-off' href='/r2/off'>Вимкнути</a></div>";
  }
  else
  {
    html += "<div><span class='badge off'>OFF</span><a class='btn btn-on' href='/r2/on'>Увімкнути</a></div>";
  }
  html += "</div>";

  // Канал 3
  html += "<div class='card'><div class='info'><h3>Канал 03 (Світло)</h3><p>GPIO 12 | D6</p></div>";
  if (relay3State)
  {
    html += "<div><span class='badge on'>ON</span><a class='btn btn-off' href='/r3/off'>Вимкнути</a></div>";
  }
  else
  {
    html += "<div><span class='badge off'>OFF</span><a class='btn btn-on' href='/r3/on'>Увімкнути</a></div>";
  }
  html += "</div>";

  html += "<div class='footer'>ESP8266 Non-Blocking Web Server</div>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// Обробники маршрутів (не блокуючі)
void handleR1On()
{
  relay1State = true;
  digitalWrite(relay1, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleR1Off()
{
  relay1State = false;
  digitalWrite(relay1, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleR2On()
{
  relay2State = true;
  digitalWrite(relay2, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleR2Off()
{
  relay2State = false;
  digitalWrite(relay2, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleR3On()
{
  relay3State = true;
  digitalWrite(relay3, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleR3Off()
{
  relay3State = false;
  digitalWrite(relay3, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup()
{
  Serial.begin(115200);

  // Налаштування пінів реле
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);

  // Початковий стан реле (HIGH - вимкнено для активного рівня LOW)
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);

  // Ініціалізація Wi-Fi без блокування у циклах
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to WiFi");
  wifiStartAttempt = millis();
}

void loop()
{
  unsigned long currentMillis = millis();

  // Неблокуюча машина станів для Wi-Fi підключення
  switch (wifiState)
  {
  case WIFI_CONNECTING:
    if (WiFi.status() == WL_CONNECTED)
    {
      wifiState = WIFI_CONNECTED;
      Serial.println("");
      Serial.println("WiFi connected successfully!");
      Serial.print("IP address: http://");
      Serial.println(WiFi.localIP());

      // Реєстрація маршрутів та запуск вебсервера лише після успішного підключення
      server.on("/", handleRoot);
      server.on("/r1/on", handleR1On);
      server.on("/r1/off", handleR1Off);
      server.on("/r2/on", handleR2On);
      server.on("/r2/off", handleR2Off);
      server.on("/r3/on", handleR3On);
      server.on("/r3/off", handleR3Off);

      server.begin();
      Serial.println("HTTP server started");
    }
    else
    {
      // Виводимо крапку кожні 500 мс без зависання процесора
      if (currentMillis - lastWifiCheck >= 500)
      {
        lastWifiCheck = currentMillis;
        Serial.print(".");
      }

      // Перевірка на таймаут (якщо за 30 секунд не підключилось — перезапуск)
      if (currentMillis - wifiStartAttempt >= wifiTimeout)
      {
        wifiState = WIFI_FAILED;
        Serial.println("\nWiFi connection failed! Restarting...");
        ESP.restart();
      }
    }
    break;

  case WIFI_CONNECTED:
    // Обробка запитів клієнтів вебсервера працює в асинхронному режимі
    server.handleClient();
    break;

  case WIFI_FAILED:
    // Нічого не робимо, чекаємо на перезапуск
    break;
  }

  // Тут у loop() можна вільно додавати інші фонові задачі (опитування датчиків MQ, DHT тощо),
  // оскільки тепер тут немає жодного delay().
  yield();
}