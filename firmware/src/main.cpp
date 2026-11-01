
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

#include "secrets.h"

// OLED: SH1106 128x64, I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE
);

// Timing
const unsigned long POLL_INTERVAL = 1000;
const unsigned long DISPLAY_INTERVAL = 250;

unsigned long lastPollTime = 0;
unsigned long lastDisplayTime = 0;

// Playback state
String trackTitle = "Waiting for Spotify...";
String artistName = "";
bool isPlaying = false;
bool serverConnected = false;

long durationMs = 0;
long progressMs = 0;

// Format milliseconds as M:SS
String formatTime(long milliseconds) {
    if (milliseconds < 0) {
        milliseconds = 0;
    }

    long totalSeconds = milliseconds / 1000;
    long minutes = totalSeconds / 60;
    long seconds = totalSeconds % 60;

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%ld:%02ld", minutes, seconds);

    return String(buffer);
}

// Connect to Wi-Fi
void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi connected");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

// Fetch playback information from Python bridge
void fetchPlaybackData() {
    if (WiFi.status() != WL_CONNECTED) {
        serverConnected = false;
        return;
    }

    HTTPClient http;
    http.setTimeout(3000);

    http.begin(SERVER_URL);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
            serverConnected = doc["connected"] | false;

            JsonObject track = doc["track"];

            if (track.isNull()) {
                trackTitle = "Nothing playing";
                artistName = "";
                isPlaying = false;
                durationMs = 0;
                progressMs = 0;
            } else {
                trackTitle = track["title"] | "Unknown Track";
                artistName = track["artist"] | "Unknown Artist";

                isPlaying = track["is_playing"] | false;
                durationMs = track["duration_ms"] | 0L;
                progressMs = track["progress_ms"] | 0L;

                if (progressMs < 0) {
                    progressMs = 0;
                }

                if (durationMs > 0 && progressMs > durationMs) {
                    progressMs = durationMs;
                }
            }
        } else {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            serverConnected = false;
        }
    } else {
        Serial.print("HTTP request failed: ");
        Serial.println(httpCode);
        serverConnected = false;
    }

    http.end();
}

// Draw playback interface
void drawDisplay() {
    u8g2.clearBuffer();

    // Header
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 9, "SPOTIFY");

    // Connection status
    if (serverConnected) {
        u8g2.drawDisc(120, 5, 3);
    } else {
        u8g2.drawCircle(120, 5, 3);
    }

    u8g2.drawHLine(0, 13, 128);

    if (!serverConnected) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, 34, "Bridge disconnected");
        u8g2.drawStr(0, 48, "Check laptop server");
    } else {
        // Track title
        u8g2.setFont(u8g2_font_7x14B_tf);

        String title = trackTitle;
        const int titleWidth = 128;

        if (u8g2.getStrWidth(title.c_str()) > titleWidth) {
            while (title.length() > 0 &&
                   u8g2.getStrWidth((title + "...").c_str()) > titleWidth) {
                title.remove(title.length() - 1);
            }
            title += "...";
        }

        u8g2.drawStr(0, 31, title.c_str());

        // Artist
        u8g2.setFont(u8g2_font_6x10_tf);

        String artist = artistName;

        if (u8g2.getStrWidth(artist.c_str()) > 128) {
            while (artist.length() > 0 &&
                   u8g2.getStrWidth((artist + "...").c_str()) > 128) {
                artist.remove(artist.length() - 1);
            }
            artist += "...";
        }

        u8g2.drawStr(0, 43, artist.c_str());

        // Progress bar
        const int barX = 0;
        const int barY = 49;
        const int barWidth = 128;
        const int barHeight = 5;

        u8g2.drawFrame(barX, barY, barWidth, barHeight);

        if (durationMs > 0) {
            int filledWidth = (int)(
                ((float)progressMs / (float)durationMs) * (barWidth - 2)
            );

            filledWidth = constrain(filledWidth, 0, barWidth - 2);

            if (filledWidth > 0) {
                u8g2.drawBox(
                    barX + 1,
                    barY + 1,
                    filledWidth,
                    barHeight - 2
                );
            }
        }

        // Playback time
        u8g2.setFont(u8g2_font_5x8_tf);

        String elapsed = formatTime(progressMs);
        String total = formatTime(durationMs);

        u8g2.drawStr(0, 63, elapsed.c_str());

        int totalWidth = u8g2.getStrWidth(total.c_str());
        u8g2.drawStr(128 - totalWidth, 63, total.c_str());

        // Playback state indicator
        if (isPlaying) {
            u8g2.drawStr(58, 63, "PLAY");
        } else {
            u8g2.drawStr(58, 63, "PAUSE");
        }
    }

    u8g2.sendBuffer();
}

void setup() {
    Serial.begin(115200);

    u8g2.begin();

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 32, "Spotify Desk Partner");
    u8g2.drawStr(42, 48, "Starting...");
    u8g2.sendBuffer();

    connectWiFi();

    fetchPlaybackData();
    drawDisplay();
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastPollTime >= POLL_INTERVAL) {
        lastPollTime = currentTime;
        fetchPlaybackData();
    }

    if (currentTime - lastDisplayTime >= DISPLAY_INTERVAL) {
        lastDisplayTime = currentTime;
        drawDisplay();
    }
}
