#include <NeoPixelBus.h>
#include <RgbColor.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266WiFi.h>

//////////////////////
// WiFi Definitions //
//////////////////////
const char WiFiAPPSK[] = "sparkfun";

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the Thing
const int ROTARY_1_PIN = 12; // Digital pin to be read
const int ROTARY_2_PIN = 13; // Digital pin to be read

const int ONE_WIRE_BUS = 2;  // DS18B20 pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


#define pixelCount 3
#define pixelPin 0  // make sure to set this to the correct pin
#define colorSaturation 128

NeoPixelBus strip = NeoPixelBus(pixelCount, pixelPin);
RgbColor red = RgbColor(colorSaturation, 0, 0);
RgbColor green = RgbColor(0, colorSaturation, 0);
RgbColor blue = RgbColor(0, 0, colorSaturation);
RgbColor white = RgbColor(colorSaturation);
RgbColor black = RgbColor(0);

const RgbColor colors[4] = {red, green, blue, black};

int counter = 0;

bool neutral = true;
int rotation = 0;

int pulses[2] = {0, 0};
int lasttime[2] = {0, 0};

WiFiServer server(80);

void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void ioChanged(int index) {
  int time = micros();
  /*if (time - lasttime[index] < 5) {
    return;
  }*/
  int pin1 = digitalRead(ROTARY_1_PIN);
  int pin2 = digitalRead(ROTARY_2_PIN);

  if (pin1 == 1 && pin2 == 1) {
    digitalWrite(LED_PIN, HIGH);
    neutral = true;
  } else {
    if (neutral && pin1 == 1) {
      rotation = 1;
    } else if (neutral && pin2 == 1)  {
      rotation = 2;
    }
    neutral = false;

    digitalWrite(LED_PIN, LOW);
  }

  pulses[index]++;
  lasttime[index] = time;
}

void io1Changed() { ioChanged(0); }
void io2Changed() { ioChanged(1); }

void initHardware()
{
  Serial.begin(115200);
  pinMode(ROTARY_1_PIN, INPUT_PULLUP);
  pinMode(ROTARY_2_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
 
  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();

  attachInterrupt(ROTARY_1_PIN, io1Changed, CHANGE);
  attachInterrupt(ROTARY_2_PIN, io2Changed, CHANGE);
  
  // Don't need to set ANALOG_PIN as input, 
  // that's all it can be.
}


void setup() 
{
  counter = 0;
  initHardware();
  setupWiFi();
  server.begin();
}

int pos = 0;
void loop() 
{
  float temp;

  if (counter >= 1000) {
    for (int i = 0; i < pixelCount; i++) {
      strip.SetPixelColor(i, colors[pos++]);
      if (pos > 3)
        pos = 0;
    }
    
    strip.Show();
    counter = 0;

    Serial.print("\nRotation 1 ");
    Serial.print(pulses[0]);
    Serial.print("\nRotation 2 ");
    Serial.print(pulses[1]);
    Serial.print("\nDirection ");
    Serial.print(rotation);
    Serial.println();
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    delay(1);
    counter++;
    return;
  }

  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.println(temp);
  } while (temp == 85.0 || temp == (-127.0));

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/led/0") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/led/1") != -1)
    val = 1; // Will write LED high
  else if (req.indexOf("/read") != -1)
    val = -2; // Will print pin reads
  // Otherwise request will be invalid. We'll say as much in HTML

  // Set GPIO5 according to the request
  if (val >= 0)
    digitalWrite(LED_PIN, val);

  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the LED, print out a message saying we did
  if (val >= 0)
  {
    s += "LED is now ";
    s += (val)?"on":"off";
  }
  else if (val == -2)
  { // If we're reading pins, print out those values:
    s += "Analog Pin = ";
    s += String(analogRead(ANALOG_PIN));
    s += "<br>"; // Go to the next line.
    s += "Digital Pin 12 = ";
    s += String(digitalRead(ROTARY_1_PIN));
    s += "<br>"; // Go to the next line.
    s += "Digital Pin 13 = ";
    s += String(digitalRead(ROTARY_2_PIN));
    s += "<br>"; // Go to the next line.
    s += "Temperature on Pin 2 = ";
    s += String(temp);

    s += "<br>";
    s += String(pulses[0]);
    s += "<br>";
    s += String(pulses[1]);
    s += "<br> Direction";
    s += String(rotation);
    pulses[0] = 0;
    pulses[1] = 0;
  }
  else
  {
    s += "Invalid Request.<br> Try /led/1, /led/0, or /read.";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  counter++;
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

