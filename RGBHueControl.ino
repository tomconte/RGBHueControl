#include <Ethernet.h>
#include <SPI.h>

/*
** Network setup
*/

// MAC address of the Ethernet shield (on the shield's sticker)
byte mac[]    = {  0x90, 0xA2, 0xDA, 0x0F, 0x24, 0xA8 };
// Address of the Hue bridge (connect to http://www.meethue.com/api/nupnp to find it)
byte server[] = { 192, 168, 0, 141 };
// Fallback IP if DHCP fails (not used)
byte ip[]     = { 192, 168, 0, 10 };
// Ethernet client
EthernetClient client;

/*
** Hue
*/

// The username who has access to the API (see http://developers.meethue.com/gettingstarted.html to create a user)
const String username = "3e395757167e1dbfb5235472713dcf";

/*
** Analog lines
*/

const int redSensorPin = A0;
const int greenSensorPin = A1;
const int blueSensorPin = A2;

/*
** Values from the photoresistors
*/

int redSensorValue = 0;
int greenSensorValue = 0;
int blueSensorValue = 0;
float *hsb = new float[3];

/*
** Read an HTTP response
*/

void readResponse()
{
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  client.stop();
}

/*
** Test request -- could be used to find out how many lights we have
*/

void sendTestRequest()
{
  Serial.println("connecting...");
  if (client.connect(server, 80)) {
    Serial.println("connected");
    String req = "GET /api/";
    req += username;
    req += "/lights HTTP/1.1";
    client.println(req);
    delay(500);
    readResponse();
    client.stop();
  } else {
    Serial.println("connection failed");
  }
}

/*
** Send a command to all lights.
** Hardcoded for lights 1..3 (the default set)
*/

void sendToAllLights(String body) {
  String req;
  for (int i=1; i<4; i++) {
    Serial.print("send to light ");
    Serial.println(i);
    Serial.println("connecting...");
    if (client.connect(server, 80)) {
      Serial.println("connected");
      req = "";
      req = "PUT /api/";
      req += username;
      req += "/lights/";
      req += i;
      req += "/state HTTP/1.1";
      client.println(req);
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(body.length());
      client.println();
      client.println(body);
      delay(500);
      readResponse();
      client.stop();
    } else {
      Serial.println("connection failed");
    }
  }
}

/*
** Convert RGB values to HSB
** Stolen from http://www.docjar.com/html/api/java/awt/Color.java.html
** More on the Hue color system:
** - http://developers.meethue.com/coreconcepts.html
** - http://developers.meethue.com/1_lightsapi.html#16_set_light_state 
*/

float *RGBtoHSB(int r, int g, int b, float *hsbvals) {
  float hue, saturation, brightness;
  if (hsbvals == NULL) {
    hsbvals = new float[3];
  }
  int cmax = (r > g) ? r : g;
  if (b > cmax) cmax = b;
  int cmin = (r < g) ? r : g;
  if (b < cmin) cmin = b;

  brightness = ((float) cmax) / 255.0f;
  if (cmax != 0)
    saturation = ((float) (cmax - cmin)) / ((float) cmax);
  else
    saturation = 0;
  if (saturation == 0)
    hue = 0;
  else {
    float redc = ((float) (cmax - r)) / ((float) (cmax - cmin));
    float greenc = ((float) (cmax - g)) / ((float) (cmax - cmin));
    float bluec = ((float) (cmax - b)) / ((float) (cmax - cmin));
    if (r == cmax)
      hue = bluec - greenc;
    else if (g == cmax)
      hue = 2.0f + redc - bluec;
    else
      hue = 4.0f + greenc - redc;
    hue = hue / 6.0f;
    if (hue < 0)
      hue = hue + 1.0f;
  }
  hsbvals[0] = hue;
  hsbvals[1] = saturation;
  hsbvals[2] = brightness;
  return hsbvals;
}

/*
** Setup
*/

void setup() {
  Serial.begin(9600);
  Serial.println("setup...");
  Ethernet.begin(mac);
  delay(1000);
  //sendTestRequest();
  sendToAllLights("{\"on\":true}");
}

/*
** Loop
*/

void loop() {
  // 1024-range to 256 for RGB values
  redSensorValue = analogRead(redSensorPin) / 4;
  greenSensorValue = analogRead(greenSensorPin) / 4;
  blueSensorValue = analogRead(blueSensorPin) / 4;

//  Serial.print("R:");
//  Serial.print(redSensorValue);
//  Serial.print(":G:");
//  Serial.print(greenSensorValue);
//  Serial.print(":B:");
//  Serial.println(blueSensorValue);
  
  RGBtoHSB(redSensorValue, greenSensorValue, blueSensorValue, hsb);

//  Serial.print("H:");
//  Serial.print(hsb[0]);
//  Serial.print(":S:");
//  Serial.print(hsb[1]);
//  Serial.print(":B:");
//  Serial.println(hsb[2]);

  // Build the Hue command for H/S/B color
  String req = "{";
  req += "\"hue\":";
  req += (int)(hsb[0] * 65535);
  req += ",\"sat\":";
  req += (int)(hsb[1] * 255);
  req += ",\"bri\":";
  req += (int)(hsb[2] * 255);
  req += "}";
  
  // Print the command for debugging
  Serial.println(req);
  
  // Send the command to all lights
  sendToAllLights(req);

  delay(30*1000);
}

