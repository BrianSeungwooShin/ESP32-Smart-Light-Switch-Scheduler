#include <ESP32Servo.h>
#include <WiFi.h>
#include <time.h>

//WIFI INFO:
const char *ssid = ""; //WIFI ID
const char *password = ""; //WIFI PASS

WiFiServer server(80);

//==================================================================================================================================================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>ESP32 Light Switch</title>
  <style>
    * { box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background: #111318;
      color: #f2f2f2;
      margin: 0;
      padding: 28px 16px 60px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    h1 {
      font-size: 1.4rem;
      margin-bottom: 4px;
      text-align: center;
    }
    .subtitle {
      font-size: 0.85rem;
      color: #797f8a;
      margin-bottom: 24px;
      text-align: center;
    }
    h2 {
      font-size: 0.85rem;
      text-transform: uppercase;
      letter-spacing: 0.06em;
      color: #9aa0aa;
      margin: 0 0 14px;
    }
    .card {
      background: #1c1f26;
      border-radius: 16px;
      padding: 20px;
      width: 100%;
      max-width: 380px;
      margin-bottom: 18px;
      box-shadow: 0 4px 14px rgba(0,0,0,0.35);
    }
    .btn-row {
      display: flex;
      gap: 12px;
    }
    a {
      flex: 1;
      text-decoration: none;
    }
    button {
      width: 100%;
      padding: 16px 0;
      font-size: 1rem;
      font-weight: 600;
      border: none;
      border-radius: 12px;
      cursor: pointer;
      -webkit-tap-highlight-color: transparent;
      transition: transform 0.08s ease, opacity 0.15s ease;
    }
    button:active { transform: scale(0.96); opacity: 0.9; }
    .on-btn { background: #33d17a; color: #06210f; }
    .off-btn { background: #e6544a; color: #2a0704; }
    form {
      margin-bottom: 18px;
    }
    form:last-child { margin-bottom: 0; }
    label {
      display: block;
      font-size: 0.9rem;
      color: #c7cad1;
      margin-bottom: 8px;
    }
    input[type="time"] {
      width: 100%;
      padding: 12px;
      font-size: 1rem;
      border-radius: 10px;
      border: 1px solid #333844;
      background: #0f1116;
      color: #f2f2f2;
      margin-bottom: 10px;
    }
    .set-on-btn { background: #2e7d5b; color: #f2f2f2; }
    .set-off-btn { background: #7d3a2e; color: #f2f2f2; }
    .status-line {
      font-size: 1.1rem;
      font-weight: 600;
      text-align: center;
      margin: 0;
    }
    .status-line span { color: #33d17a; }
    .notice-row {
      display: flex;
      justify-content: space-between;
      padding: 8px 0;
      border-bottom: 1px solid #262a33;
      font-size: 0.95rem;
    }
    .notice-row:last-child { border-bottom: none; }
    .notice-row span { color: #9aa0aa; }
  </style>
</head>
<body>

  <h1>ESP32 Light Switch</h1>
  <div class="subtitle">Manual control &amp; scheduled alarms</div>

  <div class="card">
    <h2>Status</h2>
    <p class="status-line">Light is currently <span>%LIGHT_STATE%</span></p>
  </div>

  <div class="card">
    <h2>Manual Control</h2>
    <div class="btn-row">
      <a href="/on"><button class="on-btn">Turn ON</button></a>
      <a href="/off"><button class="off-btn">Turn OFF</button></a>
    </div>
  </div>

  <div class="card">
    <h2>Set Alarms</h2>
    <form action="/scheduleOn" method="POST">
      <label>Turn ON at</label>
      <input type="time" name="time" required>
      <button type="submit" class="set-on-btn">Set ON Alarm</button>
    </form>

    <form action="/scheduleOff" method="POST">
      <label>Turn OFF at</label>
      <input type="time" name="time" required>
      <button type="submit" class="set-off-btn">Set OFF Alarm</button>
    </form>
  </div>

  <div class="card">
    <h2>Current Alarms</h2>
    <div class="notice-row"><span>ON Alarm</span> %ON_ALARM%</div>
    <div class="notice-row"><span>OFF Alarm</span> %OFF_ALARM%</div>
  </div>

</body>
</html>
)rawliteral";

//==================================================================================================================================================================================

//AlarmVariables:
int onHour = -1;
int onMin = -1;

int offHour = -1;
int offMin = -1;

//For ensuring servo motor only moves once per min.
bool lightOn = false;
bool lightOff= false;

//Tracks the light's current ON/OFF state persistently, for display on the page.
bool currentLightState = false;

//PORT NUMBERS FOR LEDs
const int RED_LED = 15;
const int GREEN_LED = 4;

//INITIALIZING SERVO MOTOR
Servo myServo;
const int servoPin = 13;

//FUNCTION FOR WHEN LIGHT IS ON/OFF:
void LightState(bool currState){
  currentLightState = currState; //Keep the tracked state in sync every time this runs.

  if(currState){ //ON
    Serial.println("Light ON");
    myServo.write(90);

  } else {
    Serial.println("Light OFF");
    myServo.write(0);

  }
}

//HELP FROM CLAUDE:
//Formats an hour/minute pair as "HH:MM" with leading zeros, or "Not set" if unset (-1).
String formatAlarm(int h, int m){
  if(h == -1){
    return "Not set";
  }
  char buf[6];
  sprintf(buf, "%02d:%02d", h, m);
  return String(buf);
}

//Builds a fresh copy of the page with the live values substituted in.
String buildPage(){
  String page = String(index_html);
  page.replace("%LIGHT_STATE%", currentLightState ? "ON" : "OFF");
  page.replace("%ON_ALARM%", formatAlarm(onHour, onMin));
  page.replace("%OFF_ALARM%", formatAlarm(offHour, offMin));
  return page;
}

// Time Server Configuration (KST UTC+9)
const char* ntpServer = "pool.ntp.org"; //Tells the ESP32 which network server to contact over Wi-Fi to get the atomic clock time.
const long  gmtOffset_sec = 9 * 3600; //Converts UTC/GMT time to Korea Standard Time (KST). 9 hours×3600 seconds=32,400 seconds. Without this, my ESP32 clock will be 9 hours behind.
const int   daylightOffset_sec = 0;

unsigned long lastAlarmCheck = 0;

void checkAlarm(){
  if (millis() - lastAlarmCheck < 1000) {
    return; // Skip — checked too recently, no need to hit getLocalTime() again yet.
  }
  lastAlarmCheck = millis();

  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){ //If NTP not in sync (Incorrect Time)
    return;
  }

  int currHour = timeinfo.tm_hour;
  int currMin = timeinfo.tm_min;

  if(onHour != -1 && currHour == onHour && currMin == onMin){
    if(!lightOn){
      Serial.printf("[ON ALARM] Triggering Auto-ON at %02d:%02d\n", currHour, currMin);
      LightState(true);
      lightOn = true; //Allows only once.
    }
  } else if (currMin != onMin || currHour != onHour) {
    lightOn = false;
  }
  

  if(offHour != -1 && currHour == offHour && currMin == offMin){
    if(!lightOff){
      Serial.printf("[OFF ALARM] Triggering Auto-OFF at %02d:%02d\n", currHour, currMin);
      LightState(false);
      lightOff = true; //Allows only once.
    }
  } else if (currMin != offMin || currHour != offHour) {
    lightOff = false;
  }
  
}

void wifiStatus(){
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(GREEN_LED, HIGH); 
    digitalWrite(RED_LED, LOW);
  }
  else{
    digitalWrite(GREEN_LED, LOW); 
    digitalWrite(RED_LED, HIGH);
    WiFi.reconnect();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  //Connecting to Local WIFI

  WiFi.begin(ssid, password);

  Serial.print("Connecting to... ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Start NTP time sync (REQUIRED before calling getLocalTime)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  Serial.println("");
  Serial.println("WIFI CONNECTED!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("WEB SERVER INITIATED. Please open the IP in your browser.");

  //Initializing the Servo Motor

  myServo.attach(servoPin);
  LightState(false);
}

void loop() {

  checkAlarm();
  wifiStatus();
  WiFiClient client = server.available();

  if (client){
    Serial.println("DEVICE CONNECTED!");

    // PARSING THE REQUEST FROM HTML WEB SERVER.

    String currLine = "";
    String requestLine = "";
    int lenContent = 0;
    bool sendResponse = false;

    while(client.connected()){
      checkAlarm();
      if(client.available()){
        char c = client.read();

        if(c != '\n'){
          currLine = currLine + c;
        }
        else { // IF FULL LINE COMPLETE
          currLine.trim(); //REMOVES WHITE SPACES (\r);

          if(requestLine == ""){ 
            Serial.print("REQUEST RECIEVED FROM WEB BROWSER: ");
            requestLine = currLine; //("GET /on HTTP/1.1" or "POST /schedule HTTP/1.1")
            Serial.println(requestLine);
          }
      
          //THE USER PRESSES TURNING ON/OFF
          if(requestLine.startsWith("GET")){
            if(requestLine.startsWith("GET /on")){
            LightState(true);

            } else if (requestLine.startsWith("GET /off")){
            LightState(false);

            }
            sendResponse = true;

          //ALARM ON/OFF
          } else if(requestLine.startsWith("POST")){
            if(currLine.startsWith("Content-Length: ")){ //Content-Length: 13 FROM REQUEST
              lenContent = currLine.substring(16).toInt(); 
            }

            if(currLine.length() == 0){
              String timeSet = ""; 

              for (int i = 0; i < lenContent; i++) {
                unsigned long startMs = millis(); //Records the exact time in milliseconds right before trying to read the next byte.
                // Safe timeout guard (max 2 seconds per byte)
                while (!client.available() && (millis() - startMs < 2000)) {  //Pauses execution as long as both conditions are true:
                                                                              //!client.available() — No new byte is available in the ESP32’s network buffer yet.
                                                                              //(millis() - startMs < 2000) — Less than 2 seconds (2000 ms) have elapsed since we started waiting for this specific character.
                  delay(1); 
                }
                if (client.available()) {
                  timeSet += (char)client.read();
                }
              }

              // EX) "time=XX%3AXX"
              //PARSING 

              int equalsIndex = timeSet.indexOf("=");
              int colonIndex = timeSet.indexOf("%3A");

              if (equalsIndex != -1 && colonIndex != -1) {
                int hourParsed = timeSet.substring(equalsIndex+1, colonIndex).toInt();
                int minParsed = timeSet.substring(colonIndex+3).toInt();

                if(requestLine.startsWith("POST /scheduleOn")){
                  onHour = hourParsed;
                  onMin = minParsed;

                  lightOn = false;

                  Serial.printf("Auto-ON set for: %02d:%02d\n", onHour, onMin);
                }

                else if(requestLine.startsWith("POST /scheduleOff")){
                  offHour = hourParsed;
                  offMin = minParsed;

                  lightOff = false;

                  Serial.printf("Auto-OFF set for: %02d:%02d\n", offHour, offMin);
                }
              }

              sendResponse = true;
            }
          }

          //SENDING BACK THE HTTP RESPONSE
          if(sendResponse) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.print(buildPage());

            client.stop(); // Close TCP connection
            break;        // Exit client reading loop
          }

          currLine = "";
        }
      }
    }
  }
}
