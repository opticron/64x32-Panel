#include <PxMatrix.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

// Pins for LED MATRIX

uint8_t display_draw_time=0;

PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16_t myCOLORS[8]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA,myBLACK};

// ISR for display refresh
void display_updater()
{
  display.display(display_draw_time);
}

void display_update_enable(bool is_enable)
{
  if (is_enable)
    display_ticker.attach(0.002, display_updater);
  else
    display_ticker.detach();
}

ESP8266WebServer *HTTP;

void indexPageFn() {
  String response = "<html>"
    "  <form action=\"/setText\" method=\"post\" name=\"myform\" id=\"myform\">"
    "    Text: <input name=\"text\" value=\"\" /><br/>"
    "    Red: <input name=\"red\" value=\"255\" /><br/>"
    "    Green: <input name=\"green\" value=\"255\" /><br/>"
    "    Blue: <input name=\"blue\" value=\"255\" /><br/>"
    "    <input type=\"submit\" value=\"Set Text\" /><br/>"
    "  </form>"
    "  <script type=\"text/javascript\">"
    "var form = document.getElementById('myform');"
    "function processForm (e) {"
    "  e.preventDefault();"

  // collect the form data while iterating over the inputs
    "  var data = {};"
    "  for (var i = 0, ii = form.length; i < ii; ++i) {"
    "    var input = form[i];"
    "    if (input.name) {"
    "      data[input.name] = input.value;"
    "    }"
    "  }"

  // construct an HTTP request
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(form.method, form.action, true);"
    "  xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');"

  // send the collected data as JSON
    "  xhr.send(JSON.stringify(data));"

    "  xhr.onloadend = function () {"
    // done
    "  };"
    "};"
    "if (form.attachEvent) {"
    "  form.attachEvent(\"submit\", processForm);"
    "} else {"
    "  form.addEventListener(\"submit\", processForm);"
    "}"
    "  </script>"
    "</body></html>";
  HTTP->send(200, "text/html", response);
}

String screen_text = "";
uint8_t current_red = 0;
uint8_t current_green = 255;
uint8_t current_blue = 255;
void textFn() {
  String screen_json = HTTP->arg("plain");
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(screen_json);
  if (root.containsKey("text")) {
    const char *screen_cstr = root["text"];
    screen_text = screen_cstr;
  }
  if (root.containsKey("red")) {
    current_red = root["red"];
  }
  if (root.containsKey("green")) {
    current_green = root["green"];
  }
  if (root.containsKey("blue")) {
    current_blue = root["blue"];
  }
  HTTP->send(204, "text/html");
  display.clearDisplay();
  display.setTextColor(display.color565(current_red, current_green, current_blue));
  display.setCursor(0,6);
  display.print(screen_text);
}

#include "test_font.h"
void setup() {
  // Define your display layout here, e.g. 1/8 step
  display.begin(8);
  display.setPanelsWidth(2);
  display.setTextColor(myRED);
  display.setCursor(0,6);
  display.setFastUpdate(true);
  Serial.begin(115200);
  display_update_enable(true);
  display.setFont(&practical8pt7b);
  WiFi.mode(WIFI_STA);
  WiFi.begin("Makers Local 256", "makerslocal");
  Serial.println("connecting");
  display.print("Connecting");

  int dots = 0;
  String cstr[4];
  cstr[0] = "Connecting";
  cstr[1] = "Connecting.";
  cstr[2] = "Connecting..";
  cstr[3] = "Connecting...";
  while (WiFi.status() != WL_CONNECTED) {
    display.setCursor(0,6);
    display.clearDisplay();
    display.print(cstr[dots]);
    dots++;
    dots %= 4;
    delay(500);
    Serial.print(".");
  }
  HTTP = new ESP8266WebServer(80);
  HTTP->on("/index.html", HTTP_GET, indexPageFn);
  HTTP->on("/", HTTP_GET, indexPageFn);
  HTTP->on("/setText", HTTP_POST, textFn);
  HTTP->begin();
  display.setCursor(0,6);
  display.clearDisplay();
  display.print("Connected!");
  delay(500);
  display.setCursor(0,6);
  display.clearDisplay();
  display.setTextColor(myGREEN);
  display.print(WiFi.localIP());
  Serial.print("Starting HTTP at ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(80);
  delay(2000);
}

/*int8_t current_x = 0;
int8_t max_x = 64;
bool x_dir_pos = true;
int8_t current_y = 0;
int8_t max_y = 32;
bool y_dir_pos = true;
// returns true if overflow happened
bool update_x() {
  current_x += (!x_dir_pos)*(-1)+x_dir_pos;
  if (current_x > max_x - 1) {
    current_x = max_x - 1;
    return true;
  }
  if (current_x < 0) {
    current_x = 0;
    return true;
  }
  return false;
}

bool update_y() {
  current_y += (!y_dir_pos)*(-1)+y_dir_pos;
  if (current_y > max_y - 1) {
    current_y = max_y - 1;
    return true;
  }
  if (current_y < 0) {
    current_y = 0;
    return true;
  }
  return false;
}*/

void loop() {
  HTTP->handleClient();
  //display.clearDisplay();

  //display.setTextColor(display.color565(current_red, current_green, current_blue));
  //display.setCursor(0,6);
  //display.print(screen_text);
  //display.drawPixelRGB888(current_x, current_y, 255, 255, 255);
  //display.flushDisplay();
  
  /*if (update_x()) {
    x_dir_pos = !x_dir_pos;
    if (update_y()) {
      y_dir_pos = !y_dir_pos;
    }
  }*/
  delay(20);
}
