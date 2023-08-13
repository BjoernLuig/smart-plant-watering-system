#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

#include "ESP32_MailClient.h"
#include "secrets.h"  // you need to make a file called "secrets.h" with the following secrets:
// #define ssid                  "ssid"
// #define password              "password"
// #define emailSenderAccount    "emailSenderAccount"
// #define emailSenderPassword   "emailSenderPassword"
// #define emailRecipient        "emailRecipient"
// #define smtpServer            "smtpServer"
// #define smtpServerPort        465 //might need to be changed

// time settings
#define ntpServer "pool.ntp.org"

// pins
#define MOISTURE1_SENSOR_PIN = 12
#define MOISTURE2_SENSOR_PIN = 13
#define MOISTURE3_SENSOR_PIN = 14
#define MOISTURE4_SENSOR_PIN = 27
#define MOISTURE5_SENSOR_PIN = 32
#define PUMP1_PIN
#define PUMP2_PIN
#define PUMP3_PIN
#define PUMP4_PIN
#define RED_PIN = 26
#define GREEN_PIN = 25
#define BLUE_PIN = 33

// datetime settings
#define TIME_ZONE \
  "CET-1CEST,M3.5.0/02,M10.5.0/03"  // from
                                    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TIME_SERVER "pool.ntp.org"  // address of timeserver
struct tm timeInfo;
char timeString[50];
int curruntHour;
int curruntWeekDay;
int curruntMonth;
int lastHour;
int lastWeekDay;
int lastMonth;

// moisture sensors
MoistureSensor {
 public:
  int pin;
  int value;
  float percent;
  MoistureSensor(int pin) : pin(pin), value(-1), percent(-1) {}
  int measure() {
    value = analogRead(pin);
    percent = 100 * (4095. - value) / (4095.);
    return percent;
  }
}
MoistureSensor moistureSensor1(MOISTURE1_SENSOR_PIN);
MoistureSensor moistureSensor2(MOISTURE2_SENSOR_PIN);
MoistureSensor moistureSensor1(MOISTURE3_SENSOR_PIN);
MoistureSensor moistureSensor1(MOISTURE4_SENSOR_PIN);
MoistureSensor moistureSensor2(MOISTURE5_SENSOR_PIN);

// motors
class Motor {
 public:
  int pin;
  bool state;
  Motor(int pin) pin(pin), state(LOW) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  void on() {
    digitalWrite(pin, HIGH);
    state = HIGH;
  }
  void off() {
    digitalWrite(pin, LOW);
    state = LOW;
  }
  void onForTime(int time) {
    on();
    delay(time);
    off();
  }
};
MOTOR pump1(16);
MOTOR pump2(4);
MOTOR pump3(16);
MOTOR pump4(4);

// RGB led
class RGBLed {
 public:
  int redPin, greenPin, bluePin;
  RGBLed(int redPin, int greenPin, int bluePin)
      : redPin(redPin), greenPin(greenPin), bluePin(bluePin) {}
  void red() {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }
  void green() {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  }
  void blue() {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
  }
  void white() {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }
  void off() {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }
}

// objects
AsyncWebServer server(80);
Preferences preferences;
SMTPData smtpData;

// html code
String html1 =
    "<!DOCTYPE html> <html>"
    "<head>"
    "<meta charset='utf-8'/>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0, "
    "user-scalable=no'>"
    "<title>";
String html2 =
    "</title>"
    "<style>"
    "body{font-family: Helvetica; font-size: 16px; text-align: center; "
    "background-color: black; margin-top: 50px;}"
    "div{ display: inline-block; border: none; padding: 16px; margin: 4px; "
    "width: 300px; color: #eeeeee; background-color: #242424;}"
    ".box{display: inline-block; border: none; padding: 16px; margin: 4px; "
    "font-family: Helvetica; font-size: 16px;}"
    ".textfield{     color: #242424; background-color: #d9d9d9; width: 260px; }"
    ".button-neutral{color: #eeeeee; background-color: #3564b5; width: 292px;}"
    ".button-on{     color: #eeeeee; background-color: #229942; width: 292px;}"
    ".button-off{    color: #eeeeee; background-color: #9c2424; width: 292px;}"
    "</style>"
    "</head>"
    "<body>";
String html3 =
    "</body>"
    "</html>";

// handlers
void handle_home(AsyncWebServerRequest *request) {
  String html =
      html1 + "Home" + html2 + "<div><h2>Home</h2><br>" + timeText() +
      "</div><br><div>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/sensors\"'>Sensors</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/actuators\"'>Actuators</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/settings\"'>Settings</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/data\"'>Data</button><br>"
      "</div><br><div>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/\"'>refresh</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/update\"'>update</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/reboot\"'>reboot</button>"
      "</div>" +
      html3;
  request->send(200, "text/html", html);
}
void handle_sensors(AsyncWebServerRequest *request) {
  measure();
  String html =
      html1 + "Sensors" + html2 +
      "<div><h2>Sensors</h2></div><br>"
      "<div>moisture sensor 1: " +
      String(moistureSensor1.percent) + " % (" + String(moistureSensor1.value) +
      ")</div><br>"
      "<div>moisture sensor 2: " +
      String(moistureSensor2.percent) + " % (" + String(moistureSensor2.value) +
      ")</div><br>"
      "<div>light sensor: " +
      String(lightSensor.percent) + " % (" + String(lightSensor.value) +
      ")</div><br>"
      "<div>ultrasonic sensor: " +
      String(ultrasonicSensor.distance) +
      " cm</div><br>"
      "<div>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/sensors\"'>refresh</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/\"'>back</button>"
      "</div>" +
      html3;
  request->send(200, "text/html", html);
}
void handle_actuators(AsyncWebServerRequest *request) {
  if (request->hasParam("actuator")) {
    String actuator = request->getParam("actuator")->value();
    if (actuator == "redLED") {
      redLED.change();
    } else if (actuator == "yellowLED") {
      yellowLED.change();
    } else if (actuator == "pump1") {
      pump1.change();
    } else if (actuator == "pump2") {
      pump2.change();
    }
  }
  String html =
      html1 + "Actuators" + html2 +
      "<div><h2>Actuators</h2></div><br>"
      "<div>"
      "<button class='box " +
      (redLED.state ? "button-on" : "button-off") +
      "' onclick='window.location.href=\"/actuators?actuator=redLED\"'>red "
      "LED</button><br>"
      "<button class='box " +
      (yellowLED.state ? "button-on" : "button-off") +
      "' "
      "onclick='window.location.href=\"/actuators?actuator=yellowLED\"'>yellow "
      "LED</button><br>"
      "<button class='box " +
      (pump1.state ? "button-on" : "button-off") +
      "' onclick='window.location.href=\"/actuators?actuator=pump1\"'>pump "
      "1</button><br>"
      "<button class='box " +
      (pump2.state ? "button-on" : "button-off") +
      "' onclick='window.location.href=\"/actuators?actuator=pump2\"'>pump "
      "2</button><br>"
      "</div><br><div>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/actuators\"'>refresh</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/\"'>back</button>"
      "</div>" +
      html3;
  request->send(200, "text/html", html);
}
void handle_settings(AsyncWebServerRequest *request) {
  if (request->hasParam("setting") && request->hasParam("value")) {
    String setting = request->getParam("setting")->value();
    int value = request->getParam("value")->value().toInt();
    if (setting == "reset") {
      preferences.clear();
    } else if (setting == "resetWaterLevelSettings") {
      preferences.putInt("doubleReflected", 0);
      preferences.putInt("singleReflected", 0);
      preferences.putInt("waterEmpty", 0);
      preferences.putInt("sentMail", 0);
    } else if (setting == "timeOffset") {
      preferences.putInt("timeOffset", value);
      configTime(preferences.getInt("timeOffset", 1) * 3600, 0, ntpServer);
    } else if (setting == "dataInterval") {
      preferences.putInt("dataInterval", value);
    } else if (setting == "doubleRefDist") {
      preferences.putInt("doubleRefDist", value);
    } else if (setting == "singleRefDist") {
      preferences.putInt("singleRefDist", value);
    } else if (setting == "waterEmptyDist") {
      preferences.putInt("waterEmptyDist", value);
    } else if (setting == "doubleReflected") {
      preferences.putInt("doubleReflected", value);
    } else if (setting == "singleReflected") {
      preferences.putInt("singleReflected", value);
    } else if (setting == "waterEmpty") {
      preferences.putInt("waterEmpty", value);
    } else if (setting == "sentMail") {
      preferences.putInt("sentMail", value);
    } else if (setting == "watering1") {
      preferences.putInt("watering1", value);
    } else if (setting == "pumpHour1") {
      preferences.putInt("pumpHour1", value);
    } else if (setting == "percentMin1") {
      preferences.putInt("percentMin1", value);
    } else if (setting == "pumpTime1") {
      preferences.putInt("pumpTime1", value);
    } else if (setting == "watering2") {
      preferences.putInt("watering2", value);
    } else if (setting == "pumpHour2") {
      preferences.putInt("pumpHour2", value);
    } else if (setting == "percentMin2") {
      preferences.putInt("percentMin2", value);
    } else if (setting == "pumpTime2") {
      preferences.putInt("pumpTime2", value);
    }
  }
  String html =
      html1 + "Settings" + html2 +
      "<div><h2>Settings</h2></div><br>"
      "<div>time:<br>"
      "timeOffset = " +
      String(preferences.getInt("timeOffset", 3600)) +
      " h<br>"
      "dataInterval = " +
      String(preferences.getInt("dataInterval", 1)) +
      " h"
      "</div><br><div>water level:<br>"
      "doubleRefDist = " +
      String(preferences.getInt("doubleRefDist", 25)) +
      " cm<br>"
      "singleRefDist = " +
      String(preferences.getInt("singleRefDist", 20)) +
      " cm<br>"
      "waterEmptyDist = " +
      String(preferences.getInt("waterEmptyDist", 20)) +
      " cm<br>"
      "doubleReflected = " +
      String(preferences.getInt("doubleReflected", 0)) +
      " (1 = true)<br>"
      "singleReflected = " +
      String(preferences.getInt("singleReflected", 0)) +
      " (1 = true)<br>"
      "waterEmpty = " +
      String(preferences.getInt("waterEmpty", 0)) +
      " (1 = true)<br>"
      "sentMail = " +
      String(preferences.getInt("sentMail", 0)) +
      " (1 = true)"
      "</div><br><div>pump 1:<br>"
      "watering1 = " +
      String(preferences.getInt("watering1", 0)) +
      " (1 = true)<br>"
      "pumpHour1 = " +
      String(preferences.getInt("pumpHour1", 12)) +
      " h<br>"
      "percentMin1 = " +
      String(preferences.getInt("percentMin1", 50)) +
      "%<br>"
      "pumpTime1 = " +
      String(preferences.getInt("pumpTime1", 1000)) +
      " ms"
      "</div><br><div>pump 2:<br>"
      "watering2 = " +
      String(preferences.getInt("watering2", 0)) +
      " (1 = true)<br>"
      "pumpHour2 = " +
      String(preferences.getInt("pumpHour2", 12)) +
      " h<br>"
      "percentMin2 = " +
      String(preferences.getInt("percentMin2", 50)) +
      "%<br>"
      "pumpTime2 = " +
      String(preferences.getInt("pumpTime2", 1000)) +
      " ms"
      "</div><br><div><form action='/settings'>"
      "<label for='setting'>name of variable:</label><br><input class='box "
      "textfield' type='text' id='setting' name='setting'><br>"
      "<label for='value'>value (integer):</label><br><input class='box "
      "textfield' type='text' id='value' name='value'><br>"
      "<input class='box button-neutral' type='submit' value='set'>"
      "</form>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/"
      "settings?setting=resetWaterLevelSettings&value=1\"'>reset water level "
      "settings</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/"
      "settings?setting=reset&value=1\"'>reset all settings</button>"
      "</div><br><div>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/settings\"'>refresh</button><br>"
      "<button class='box button-neutral' "
      "onclick='window.location.href=\"/\"'>back</button>"
      "</div>" +
      html3;
  request->send(200, "text/html", html);
}
void handle_reboot(AsyncWebServerRequest *request) {
  String html = html1 + "Home" + html2 +
                "<div><h2>Reboot started</h2></div><br><div>"
                "<button class='box button-neutral' "
                "onclick='window.location.href=\"/\"'>back</button>"
                "</div>" +
                html3;
  request->send(200, "text/html", html);
  delay(2000);
  ESP.restart();
}
void handle_data(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", dataString);
}

// functions
String towDigits(int i) { return ((i < 10) ? ("0" + String(i)) : String(i)); }
String timeText() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return ("Failed to obtain time");
  }
  String text = String(timeinfo.tm_year + 1900) + "-" +
                towDigits(timeinfo.tm_mon + 1) + "-" +
                towDigits(timeinfo.tm_mday) + "T" +
                towDigits(timeinfo.tm_hour) + ":" + towDigits(timeinfo.tm_min);
  return (text);
}
int getHour() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return (-1);
  }
  return (timeinfo.tm_hour);
}
void measure() {
  moistureSensor1.measure();
  moistureSensor2.measure();
  lightSensor.measure();
  ultrasonicSensor.measure();
}

// setup
void setup(void) {
  // serial
  Serial.begin(115200);
  delay(100);

  // connect wifi
  WiFi.begin(SSID, PASSWORD);
  Serial.print("connecting to wifi");
  display.print("connecting to wifi");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    display.print(".");
    display.display();
    delay(1000);
  }
  Serial.println();
  Serial.println("wifi connected");
  display.println();
  display.println("wifi connected");
  display.display();
  delay(2000);  // wail for readability

  // connect to npt server and set timezone
  configTime(0, 0,
             TIME_SERVER);     // 0, 0 because we will use TZ in the next line
  setenv("TZ", TIME_ZONE, 1);  // Set environment variable with your time zone
  tzset();
  while (!getLocalTime(&timeInfo)) {
    Serial.println("failed to get time info");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("failed to get time info");
    display.display();
    delay(500);
    display.clearDisplay();
    display.display();
    delay(500);
  }
  strftime(timeString, sizeof(timeString), "%d.%m %H:%M", &timeInfo);
  Serial.print("new time: ");
  Serial.println(timeString);
  display.print("new time: ");
  display.println(timeString);
  display.display();

  // disconnect wifi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("wifi disconnected");
  display.println("wifi disconnected");
  display.display();
  delay(2000);  // wail for readability

  // wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // preferences
  preferences.begin("settings", false);

  // time
  configTime(preferences.getInt("timeOffset", 1) * 3600, 0, ntpServer);

  // server
  server.on("/", HTTP_GET, handle_home);
  server.on("/sensors", HTTP_GET, handle_sensors);
  server.on("/actuators", HTTP_GET, handle_actuators);
  server.on("/settings", HTTP_GET, handle_settings);
  server.on("/data", HTTP_GET, handle_data);
  server.on("/reboot", HTTP_GET, handle_reboot);

  // start ElegantOTA and server
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");

  // email
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount,
                    emailSenderPassword);

  // led
  yellowLED.on();
}

// loop and settings
int lastHour = -1;
int hour = 0;
void loop() {
  AsyncElegantOTA.loop();
  if (rebootButton.state()) ESP.restart();
  hour = getHour();
  if (lastHour < hour or (lastHour == 23 and hour == 0)) {
    if (hour % preferences.getInt("dataInterval", 1) == 0) {
      measure();
      dataString += timeText() + " " + String(moistureSensor1.percent) + " " +
                    String(moistureSensor2.percent) + " " +
                    String(lightSensor.percent) + " " +
                    String(ultrasonicSensor.distance) + "\n";
    }
    if ((hour == preferences.getInt("pumpHour1", 12)) and
        (moistureSensor1.percent < preferences.getInt("percentMin1", 50))) {
      if (preferences.getInt("watering1", 0))
        pump1.onForTime(preferences.getInt("pumpTime1", 1000));
      else
        redLED.onForTime(preferences.getInt("pumpTime1", 1000));
    }
    if ((hour == preferences.getInt("pumpHour2", 12)) and
        (moistureSensor2.percent < preferences.getInt("percentMin2", 50))) {
      if (preferences.getInt("watering2", 0))
        pump2.onForTime(preferences.getInt("pumpTime2", 1000));
      else
        redLED.onForTime(preferences.getInt("pumpTime2", 1000));
    }
    if (ultrasonicSensor.distance > preferences.getInt("doubleRefDist", 25)) {
      preferences.putInt("doubleReflected", 1);
      preferences.putInt("singleReflected", 0);
      preferences.putInt("waterEmpty", 0);
      preferences.putInt("sentMail", 0);
    }
    if (preferences.getInt("doubleReflected", 0) and
        (ultrasonicSensor.distance < preferences.getInt("singleRefDist", 20))) {
      preferences.putInt("singleReflected", 1);
    }
    if (preferences.getInt("singleReflected", 0) and
        (ultrasonicSensor.distance >
         preferences.getInt("waterEmptyDist", 20))) {
      preferences.putInt("doubleReflected", 0);
      preferences.putInt("waterEmpty", 1);
    }
    if (preferences.getInt("waterEmpty") and
        not(preferences.getInt("sentMail", 0))) {
      smtpData.setSender("ESP32", emailSenderAccount);
      smtpData.setPriority("High");
      smtpData.setSubject("ESP32 wateringsystem");
      smtpData.setMessage("Wasserstand kritisch!<br>Ultrasonic Sensor: " +
                              String(ultrasonicSensor.distance) +
                              " cm<br><br>Sent from <a href='http://" +
                              WiFi.localIP().toString() +
                              "'>ESP32 wateringsystem</a>",
                          true);
      smtpData.addRecipient(emailRecipient);
      if (!MailClient.sendMail(smtpData))
        Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
      smtpData.empty();
      preferences.putInt("sentMail", 1);
    }
    lastHour = hour;
  }
}
