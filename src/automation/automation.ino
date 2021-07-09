/* References:
 * https://github.com/shmmsra/esp/blob/main/src/http/http.ino
 * https://github.com/shmmsra/esp/blob/main/src/relay/relay.ino
 * https://github.com/shmmsra/esp/blob/main/src/ntp/ntp.ino
 * https://github.com/shmmsra/esp/blob/main/src/udp/udp.ino
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define __PORT__ 4000
// Offset by +5:30 hour i.e., IST time zone
#define __UTC_OFFSET__ 19800
#define __RELAY__ 0
#define __MISS_COUNT_THRESHOLD__ 10
#define __SMART_RELAY__ 1
#define __ENABLE_LOGS__ 1

#ifdef __ENABLE_LOGS__
#define LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define LOG_PRINTF(...) {}
#endif

const char __SSID__[] = "********";
const char __PASS__[] = "********";

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpLocal;
// A UDP instance to get NTP data (trying to use one UDP instance for both local and NTP doesn't work)
WiFiUDP udpNtp;
// An array to hold the local UDP data received from other devices
char data[255];
unsigned short missCount = 0;

// Define NTP Client to get time
NTPClient ntp(udpNtp, "pool.ntp.org", __UTC_OFFSET__);

void setup() {
    Serial.begin(38400);
    WiFi.begin(__SSID__, __PASS__);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.println(".");
        delay(500);
    }

    LOG_PRINTF(
        "[setup] IP address: %s\n",
        WiFi.localIP().toString().c_str()
    );
    udpLocal.begin(__PORT__);
    pinMode(__RELAY__, OUTPUT);
}

void loop() {
#ifdef __SMART_RELAY__
    if (receivedMessage(F("ESP:SL:TV"))) {
        missCount = 1;
        LOG_PRINTF(
            "[loop] Status: receivedMessage: true, missCount: %hu\n",
            missCount
        );
    } else if (missCount > 0) {
        // Prevent number overflow
        missCount = (++missCount > __MISS_COUNT_THRESHOLD__) ? (__MISS_COUNT_THRESHOLD__ + 1) : missCount;
        LOG_PRINTF(
            "[loop] Status: receivedMessage: false, missCount: %hu\n",
            missCount
        );
    }

    unsigned short hour = getHour();
    LOG_PRINTF("[loop] Status: missCount: %hu, hour: %hu\n", missCount, hour);
    if (hour >= 6 && hour <= 18) {
        // Turn off the light
        digitalWrite(__RELAY__, HIGH);
    } else if (hour > 18 && hour <= 23) {
        // Turn on the light
        digitalWrite(__RELAY__, LOW);
    } else {
        if (missCount > __MISS_COUNT_THRESHOLD__) {
            // Turn off the light
            digitalWrite(__RELAY__, HIGH);
        } else if (missCount > 1) {
            // Turn on the light
            digitalWrite(__RELAY__, LOW);
        }
    }

    broadcastMessage(F("ESP:SL:BKWL"));
#else
    broadcastMessage(F("ESP:SL:TV"));
    // Keep the relay always on
    digitalWrite(__RELAY__, LOW);
    delay(1000);
#endif // __SMART_RELAY__
}

void broadcastMessage(const __FlashStringHelper* msg) {
    IPAddress broadcastIp = WiFi.localIP();
    broadcastIp[3] = 255;
    udpLocal.flush();
    udpLocal.beginPacket(broadcastIp, __PORT__);
    udpLocal.print(msg);
    udpLocal.endPacket();
    udpLocal.flush();
}

bool receivedMessage(const __FlashStringHelper* msg) {
    unsigned short i = 0;
    while (i++ < 5) {
        if (udpLocal.parsePacket()) {
            short len = udpLocal.read(data, 255);
            if (len > 0) {
                data[len] = 0;
                if (String(data) == msg) {
                    // receive incoming UDP packets
                    LOG_PRINTF(
                        "[receivedMessage] Received from %s, port %d, data: %s\n",
                        udpLocal.remoteIP().toString().c_str(),
                        udpLocal.remotePort(),
                        data
                    );
                    udpLocal.flush();
                    return true;
                }
            }
        }
        // TODO: Should optimize the time offset
        delay(200);
    }
    return false;
}

unsigned short getHour() {
    unsigned long ms = millis();
    static unsigned long elapsedTime = 0;

    // By default return mid-night hour
    static unsigned short hour = 0;

    // Handle memory overflow, first invocation and backoff logic
    if (ms < 60000 || elapsedTime == 0 || (ms - elapsedTime) >= 60000) {
        ntp.update();
        elapsedTime = ms;
        hour = ntp.getHours();
    }

    return hour;
}
