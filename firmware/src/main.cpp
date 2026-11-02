
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <time.h>

#include "secrets.h"

// OLED: SH1106 128x64, I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE
);

// Timing
const unsigned long POLL_INTERVAL = 1000;
const unsigned long DISPLAY_INTERVAL = 50;

// Marquee settings
const unsigned long SCROLL_INTERVAL = 30;
const int SCROLL_GAP = 28;

unsigned long lastPollTime = 0;
unsigned long lastDisplayTime = 0;
unsigned long lastScrollTime = 0;
unsigned long lastClockUpdate = 0;

// Playback state
String trackTitle = "Waiting for Spotify...";
String artistName = "";

bool isPlaying = false;
bool serverConnected = false;

long durationMs = 0;
long progressMs = 0;

// Marquee state
int titleScrollX = 0;

// Clock and greeting
String greeting = "Hello, music lover";
char clockText[6] = "--:--";

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

// Update greeting and clock using Indian Standard Time
void updateClock() {
    struct tm timeInfo;

    if (getLocalTime(&timeInfo, 10)) {
        int hour = timeInfo.tm_hour;

        if (hour >= 5 && hour < 12) {
            greeting = "Good morning";
        }
        else if (hour >= 12 && hour < 17) {
            greeting = "Good afternoon";
        }
        else if (hour >= 17 && hour < 22) {
            greeting = "Good evening";
        }
        else {
            greeting = "Late night jamming";
        }

        snprintf(
            clockText,
            sizeof(clockText),
            "%02d:%02d",
            timeInfo.tm_hour,
            timeInfo.tm_min
        );
    }
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

                titleScrollX = 0;
            }
            else {
                String newTitle = track["title"] | "Unknown Track";

                // Reset marquee when the track changes
                if (newTitle != trackTitle) {
                    titleScrollX = 0;
                    lastScrollTime = millis();
                }

                trackTitle = newTitle;
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
        }
        else {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());

            serverConnected = false;
        }
    }
    else {
        Serial.print("HTTP request failed: ");
        Serial.println(httpCode);

        serverConnected = false;
    }

    http.end();
}

// Update marquee position
void updateTitleScroll(int titleWidth) {
    const int screenWidth = 128;

    // Short titles do not need scrolling
    if (titleWidth <= screenWidth) {
        titleScrollX = 0;
        return;
    }

    unsigned long currentTime = millis();

    if (currentTime - lastScrollTime >= SCROLL_INTERVAL) {

        // Account for elapsed time so scrolling stays consistent
        unsigned long steps =
            (currentTime - lastScrollTime) / SCROLL_INTERVAL;

        int cycleWidth = titleWidth + SCROLL_GAP;

        titleScrollX =
            (titleScrollX + steps) % cycleWidth;

        lastScrollTime += steps * SCROLL_INTERVAL;
    }
}

// Draw playback interface
void drawDisplay() {
    u8g2.clearBuffer();

    // Greeting and clock header
    u8g2.setFont(u8g2_font_5x8_tf);

    int clockWidth = u8g2.getStrWidth(clockText);
    u8g2.drawStr(128 - clockWidth, 9, clockText);

    u8g2.drawStr(0, 9, greeting.c_str());

    u8g2.drawHLine(0, 13, 128);

    if (!serverConnected) {
        u8g2.setFont(u8g2_font_6x10_tf);

        u8g2.drawStr(0, 34, "Bridge disconnected");
        u8g2.drawStr(0, 48, "Check laptop server");
    }
    else {
        // Track title with continuous marquee
        u8g2.setFont(u8g2_font_7x14B_tf);

        int titleWidth = u8g2.getStrWidth(trackTitle.c_str());

        updateTitleScroll(titleWidth);

        if (titleWidth <= 128) {
            // Keep short titles stationary
            u8g2.drawStr(0, 31, trackTitle.c_str());
        }
        else {
            // First copy
            u8g2.drawStr(
                -titleScrollX,
                31,
                trackTitle.c_str()
            );

            // Second copy with a gap for continuous repetition
            u8g2.drawStr(
                -titleScrollX + titleWidth + SCROLL_GAP,
                31,
                trackTitle.c_str()
            );
        }

        // Artist
        u8g2.setFont(u8g2_font_6x10_tf);

        String artist = artistName;

        if (u8g2.getStrWidth(artist.c_str()) > 128) {
            while (
                artist.length() > 0 &&
                u8g2.getStrWidth((artist + "...").c_str()) > 128
            ) {
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

        u8g2.drawFrame(
            barX,
            barY,
            barWidth,
            barHeight
        );

        if (durationMs > 0) {
            int filledWidth = (int)(
                ((float)progressMs / (float)durationMs)
                * (barWidth - 2)
            );

            filledWidth = constrain(
                filledWidth,
                0,
                barWidth - 2
            );

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

        u8g2.drawStr(
            128 - totalWidth,
            63,
            total.c_str()
        );

        // Playback status
        String status = isPlaying ? "PLAYING" : "PAUSED";

        int statusWidth = u8g2.getStrWidth(status.c_str());

        u8g2.drawStr(
            (128 - statusWidth) / 2,
            63,
            status.c_str()
        );
    }

    u8g2.sendBuffer();
}

void setup() {
    Serial.begin(115200);

    u8g2.begin();

    // Initial loading screen
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tf);

    const char* loadingText = "Spotify Loading...";

    int loadingWidth = u8g2.getStrWidth(loadingText);

    u8g2.drawStr(
        (128 - loadingWidth) / 2,
        36,
        loadingText
    );

    u8g2.sendBuffer();

    connectWiFi();

    // Configure NTP for Indian Standard Time (UTC +5:30)
    configTime(
        19800,
        0,
        "pool.ntp.org",
        "time.nist.gov"
    );

    updateClock();

    fetchPlaybackData();

    drawDisplay();
}

void loop() {
    unsigned long currentTime = millis();

    // Poll Spotify bridge
    if (currentTime - lastPollTime >= POLL_INTERVAL) {
        lastPollTime = currentTime;
        fetchPlaybackData();
    }

    // Update clock once per second
    if (currentTime - lastClockUpdate >= 1000) {
        lastClockUpdate = currentTime;
        updateClock();
    }

    // Refresh OLED display
    if (currentTime - lastDisplayTime >= DISPLAY_INTERVAL) {
        lastDisplayTime = currentTime;
        drawDisplay();
    }
}
