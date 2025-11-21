#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>

bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;

Preferences prefs;

struct ServerConfig {
  String type = "coreiot";          // coreiot hoặc custom
  String token_url = "sclh3zy0qywsr4ruznjq";
  uint32_t cycle_ms = 10000;
} serverConfig;

WebServer server(80);


unsigned long connect_start_ms = 0;
bool connecting = false;

String mainPage()
{
  float temperature = glob_temperature;
  float humidity = glob_humidity;
  float light = glob_light;
  String led1 = led1_state ? "ON" : "OFF";
  String led2 = led2_state ? "ON" : "OFF";

  return R"rawliteral(
  <!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>ESP32 Dashboard</title>
  <style>
    body {
      font-family: "Segoe UI", Arial, sans-serif;
      background: linear-gradient(135deg, #58a6ff, #8ec5ff);
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      color: #333;
    }

    .glass {
      background: rgba(255, 255, 255, 0.25);
      backdrop-filter: blur(14px) saturate(180%);
      -webkit-backdrop-filter: blur(14px) saturate(180%);
      border-radius: 22px;
      border: 1px solid rgba(255, 255, 255, 0.35);
      padding: 35px;
      width: 90%;
      max-width: 430px;
      text-align: center;
      box-shadow: 0 8px 32px rgba(0,0,0,0.2);
      animation: fadeIn 0.6s ease-in-out;
    }

    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(15px); }
      to   { opacity: 1; transform: translateY(0); }
    }

    h1 {
      color: #ffffff;
      font-size: 2.1em;
      font-weight: 700;
      margin-bottom: 25px;
      text-shadow: 0 0 12px rgba(255,255,255,0.8);
    }

    .sensorRow {
      display: flex;
      justify-content: space-between;
      background: rgba(255,255,255,0.35);
      padding: 14px 18px;
      margin: 10px 0;
      border-radius: 14px;
      font-size: 1.15em;
      font-weight: 600;
      color: #003d80;
      box-shadow: inset 0 0 8px rgba(0,0,0,0.1);
      animation: pop 0.4s ease;
    }

    @keyframes pop {
      from { transform: scale(0.95); opacity: 0.5; }
      to { transform: scale(1); opacity: 1; }
    }

    .sensor span {
      color: #005eff;
      font-weight: 700;
    }

    button {
      margin: 12px 8px;
      background: linear-gradient(135deg, #007bff, #00c6ff);
      color: #fff;
      border: none;
      border-radius: 25px;
      padding: 12px 28px;
      font-size: 1.05em;
      cursor: pointer;
      transition: 0.25s;
      font-weight: 600;
      box-shadow: 0 4px 12px rgba(0,0,0,0.2);
    }

    button:hover {
      transform: scale(1.09);
      box-shadow: 0 6px 18px rgba(0,0,0,0.25);
    }

    #settings {
      background: rgba(255,255,255,0.75);
      color: #005eff;
    }

    #settings:hover {
      background: rgba(255,255,255,0.9);
    }
  </style>
</head>
<body>

  <div class="glass">
    <h1>📡 ESP32 Realtime Dashboard</h1>

    <div class="sensorRow">🌡️ Nhiệt độ: <span id="temp">--</span> °C</div>
    <div class="sensorRow">💧 Độ ẩm: <span id="hum">--</span> %</div>
    <div class="sensorRow">🌞 Ánh sáng: <span id="light">--</span> lx</div>

    <button onclick="toggleLED(1)">💡 LED 1: <span id="l1">OFF</span></button>
    <button onclick="toggleLED(2)">💡 LED 2: <span id="l2">OFF</span></button>

    <button id="settings" onclick="window.location='/settings'">⚙️ Cài đặt</button>
  </div>

  <script>
    function toggleLED(id) {
      fetch(`/toggle?led=${id}`)
        .then(res => res.json())
        .then(data => {
          document.getElementById('l1').innerText = data.led1;
          document.getElementById('l2').innerText = data.led2;
        });
    }

    setInterval(() => {
      fetch('/sensors')
        .then(res => res.json())
        .then(data => {
          document.getElementById('temp').innerText = data.temp;
          document.getElementById('hum').innerText = data.hum;
          document.getElementById('light').innerText = data.light;
        });
    }, 1500);
  </script>

</body>
</html>
)rawliteral";
}

// ==================== TRANG CÀI ĐẶT (HOÀN CHỈNH) ====================
String settingsPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Cài đặt Wi-Fi & Chu kỳ</title>
  <style>
    body {font-family: "Segoe UI",Arial,sans-serif;background: linear-gradient(135deg,#667eea,#764ba2);margin:0;padding:20px;min-height:100vh;display:flex;align-items:center;justify-content:center;color:#fff;}
    .container {width:100%;max-width:520px;background:rgba(255,255,255,0.25);border-radius:24px;padding:32px;box-shadow:0 12px 40px rgba(0,0,0,0.3);backdrop-filter:blur(16px);}
    h1 {font-size:2.3em;margin-bottom:30px;text-align:center;}
    .columns {display:flex;gap:20px;flex-wrap:wrap;margin-bottom:25px;}
    .column {flex:1;min-width:280px;background:rgba(255,255,255,0.22);padding:22px;border-radius:18px;box-shadow:0 4px 15px rgba(0,0,0,0.2);}
    h2 {margin:0 0 20px 0;font-size:1.45em;text-align:center;}
    input, select {width:100%;padding:14px;margin:11px 0;border:none;border-radius:12px;font-size:1.1em;box-sizing:border-box;box-shadow:inset 0 2px 8px rgba(0,0,0,0.15);}
    .btn {width:100%;padding:15px;margin-top:15px;border:none;border-radius:30px;font-size:1.15em;font-weight:600;cursor:pointer;background:linear-gradient(135deg,#00f2c3,#009c7d);color:#002d22;transition:all 0.3s;box-shadow:0 6px 20px rgba(0,0,0,0.25);}
    .btn:hover {transform:translateY(-3px);box-shadow:0 10px 28px rgba(0,0,0,0.35);}
    .back-btn {background:rgba(255,255,255,0.85);color:#5533cc;margin-top:15px;}
    #msg {margin-top:20px;font-weight:600;min-height:28px;text-align:center;}
  </style>
</head>
<body>
  <div class="container">
    <h1>⚙️ Cài đặt Wi-Fi & CoreIOT</h1>

    <div class="columns">
      <!-- Cột Wi-Fi -->
      <div class="column">
        <h2>Wi-Fi</h2>
        <input type="text" id="ssid" placeholder="Tên Wi-Fi (SSID)" required />
        <input type="password" id="pass" placeholder="Mật khẩu Wi-Fi" />
        <button class="btn" onclick="connectWiFi()">Kết nối Wi-Fi</button>
      </div>

      <!-- Cột Chu kỳ gửi -->
      <div class="column">
        <h2>Chu kỳ gửi lên CoreIOT</h2>
        <select id="cycle">
          <option value="5000">5 giây</option>
          <option value="10000">10 giây (mặc định)</option>
          <option value="15000">15 giây</option>
          <option value="30000">30 giây</option>
          <option value="60000">60 giây</option>
          <option value="120000">120 giây</option>
          <option value="300000">300 giây</option>
        </select>

        <button class="btn" onclick="saveCycle()">Cập nhật chu kỳ</button>

        <div style="margin-top:15px;font-size:1.1em;">
          Đang gửi mỗi: <b id="currentCycle">10</b> giây
        </div>
      </div>
    </div>

    <button class="btn back-btn" onclick="window.location='/'">← Quay lại Dashboard</button>
    <div id="msg"></div>
  </div>

  <script>
    // Tải chu kỳ hiện tại khi mở trang
    fetch('/getcycle')
      .then(r => r.text())
      .then(ms => {
        const sec = Math.round(ms / 1000);
        document.getElementById('cycle').value = ms;
        document.getElementById('currentCycle').innerText = sec;
      });

    function connectWiFi() {
      const ssid = document.getElementById('ssid').value.trim();
      const pass = document.getElementById('pass').value;
      if (!ssid) return alert("Vui lòng nhập SSID!");

      fetch(`/connect?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`)
        .then(r => r.text())
        .then(msg => {
          document.getElementById('msg').innerHTML = `<span style="color:#0f0">✓ ${msg}</span>`;
        });
    }

    function saveCycle() {
      const ms = document.getElementById('cycle').value;
      fetch(`/setcycle?ms=${ms}`)
        .then(r => r.text())
        .then(msg => {
          document.getElementById('msg').innerHTML = `<span style="color:#0f0">✓ ${msg}</span>`;
          document.getElementById('currentCycle').innerText = Math.round(ms/1000);
          setTimeout(() => document.getElementById('msg').innerHTML = '', 5000);
        });
    }
  </script>
</body>
</html>
  )rawliteral";
}
// Lấy chu kỳ hiện tại
void handleGetCycle() {
  server.send(200, "text/plain", String(publish_interval_ms));
}

// Cập nhật chu kỳ
void handleSetCycle() {
  if (server.hasArg("ms")) {
    uint32_t new_ms = server.arg("ms").toInt();
    if (new_ms >= 5000 && new_ms <= 300000) {
      publish_interval_ms = new_ms;
      server.send(200, "text/plain", "Đã cập nhật chu kỳ thành " + String(new_ms/1000) + " giây!");
    } else {
      server.send(400, "text/plain", "Chu kỳ phải từ 5 đến 300 giây!");
    }
  }
}
// Hiển thị chu kỳ hiện tại trên trang settings
void handleSettings() {
  String html = settingsPage();  // hàm cũ của bạn
  html.replace("10000", String(publish_interval_ms));  // thay giá trị mặc định
  server.send(200, "text/html", html);
}
// ========== Handlers ==========
void handleRoot() { server.send(200, "text/html", mainPage()); }

void handleToggle() {
  int led = server.arg("led").toInt();
  if (led == 1) {
    led1_state = !led1_state;
    Serial.println("YOUR CODE TO CONTROL LED1");
    pinMode(48, OUTPUT);
    digitalWrite(48, led1_state ? HIGH : LOW);
  }
  else if (led == 2){
    led2_state = !led2_state;
    Serial.println("YOUR CODE TO CONTROL LED2");
  }
  server.send(200, "application/json",
    "{\"led1\":\"" + String(led1_state ? "ON":"OFF") +
    "\",\"led2\":\"" + String(led2_state ? "ON":"OFF") + "\"}");
}

void handleSensors() {
  float t = glob_temperature;
  float h = glob_humidity;
  float l = glob_light;
  String json = "{\"temp\":"+String(t)+",\"hum\":"+String(h)+ ",\"light\":"+String(l)+"}";
  server.send(200, "application/json", json);
}

//void handleSettings() { server.send(200, "text/html", settingsPage()); }

void handleConnect() {
  Serial.print("OK");
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}

// ========== WiFi ==========
void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/connect", HTTP_GET, handleConnect);
  server.on("/getcycle", HTTP_GET, handleGetCycle);
  server.on("/setcycle", HTTP_GET, handleSetCycle);
  server.begin();
}

void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());

  Serial.print(" Password: ");
  Serial.print(wifi_password.c_str());
}

// ========== Main task ==========
void main_server_task(void *pvParameters){
  pinMode(BOOT_PIN, INPUT_PULLUP);

  startAP();
  setupServer();

  while(1){
    server.handleClient();

    // BOOT Button to switch to AP Mode
    if (digitalRead(BOOT_PIN) == LOW) {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW) {
        if (!isAPMode) {
          startAP();
          setupServer();
        }
      }
    }

    // STA Mode
    if (connecting) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; //Internet access

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
         
      } else if (millis() - connect_start_ms > 10000) { // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        startAP();
        setupServer();
        connecting = false;
        isWifiConnected = false;
      }
    }

    vTaskDelay(20); // avoid watchdog reset
  }
}