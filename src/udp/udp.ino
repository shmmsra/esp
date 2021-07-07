/* References:
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/udp-examples.html
 * https://www.esp8266.com/viewtopic.php?p=64084
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

char ssid[] = "********"; // network SSID (name)
char pass[] = "********"; // network password
unsigned int udpPort = 2311; // local port to listen for UDP packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void setup() {
    Serial.begin(38400);
    WiFi.begin(ssid, pass);

    while ( WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("[setup] WiFi connected");
    Serial.print("[setup] IP address: ");
    Serial.println(WiFi.localIP());

    udp.begin(udpPort);
}

void loop() {
    IPAddress broadcastIp = WiFi.localIP();
    broadcastIp[3] = 255;
    Serial.println(broadcastIp);
    udp.beginPacket(broadcastIp, udpPort);
    udp.print(F("ESP:SL:TV"));
    udp.endPacket();

    delay(2000);
}
