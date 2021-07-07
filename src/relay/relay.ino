/* References:
 * https://ncd.io/relay-logic/
 * https://medium.com/@ttrolololll/lamp-control-using-esp-8266-relay-module-199c4a9a6286
 * https://www.geekering.com/categories/embedded-sytems/esp8266/ricardocarreira/esp-01s-wi-fi-relay-module-for-light-remote-control/
 * https://dzone.com/articles/programming-the-esp8266-with-the-arduino-ide-in-3
 * https://create.arduino.cc/projecthub/pratikdesai/how-to-program-esp8266-esp-01-module-with-arduino-uno-598166
 * https://www.instructables.com/ESP0101S-RELAY-MODULE-TUTORIAL/
 * https://iot-guider.com/esp8266-nodemcu/hardware-basics-of-esp8266-esp-01-wifi-module/
 */

#include <ESP8266WiFi.h>

const char* ssid = "********"; // fill in here your router or wifi SSID
const char* password = "********"; // fill in here your router or wifi password
#define RELAY 0 // relay connected to    GPIO0
WiFiServer server(80);

void setup() {
    Serial.begin(115200); // must be same baudrate with the Serial Monitor

    pinMode(RELAY,OUTPUT);
    digitalWrite(RELAY, LOW);

    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.printf("Web server started, open http://%s/ in a web browser\n", WiFi.localIP().toString().c_str());
}

void loop() {
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    // Wait until the client sends some data
    Serial.println("new client");
    while(!client.available()) {
        delay(1);
    }

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Match the request
    int value = LOW;
    if (request.indexOf("/RELAY=ON") != -1) {
        Serial.println("RELAY=ON");
        digitalWrite(RELAY,LOW);
        value = LOW;
    }
    if (request.indexOf("/RELAY=OFF") != -1) {
        Serial.println("RELAY=OFF");
        digitalWrite(RELAY,HIGH);
        value = HIGH;
    }

    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //    this is a must
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head><title>ESP8266 RELAY Control</title></head>");
    client.print("Relay is now: ");

    if(value == HIGH) {
        client.print("OFF");
    } else {
        client.print("ON");
    }
    client.println("<br><br>");
    client.println("Turn <a href=\"/RELAY=OFF\">OFF</a> RELAY<br>");
    client.println("Turn <a href=\"/RELAY=ON\">ON</a> RELAY<br>");
    client.println("</html>");

    delay(1);
    Serial.println("Client disonnected");
    Serial.println("");
}
