
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// ==========================================
// SPOTIFY DESK PARTNER
// Simulated Music Playback
// ==========================================

#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE
);

// ==========================================
// SIMULATED TRACK DATA
// ==========================================

struct Track {
    const char* title;
    const char* artist;
    uint16_t duration;
};

Track tracks[] = {
    {"Midnight City", "M83", 212},
    {"Instant Crush", "Daft Punk", 337},
    {"505", "Arctic Monkeys", 253},
    {"After Dark", "Mr.Kitty", 257}
};

const char* playlistName = "DAILY MIX";

const uint8_t TOTAL_TRACKS =
    sizeof(tracks) / sizeof(tracks[0]);

uint8_t currentTrack = 0;

uint16_t elapsedTime = 0;

unsigned long lastUpdate = 0;

// ==========================================
// TIME FORMATTING
// ==========================================

void formatTime(uint16_t seconds, char* output) {

    uint16_t minutes = seconds / 60;
    uint16_t remainingSeconds = seconds % 60;

    sprintf(
        output,
        "%d:%02d",
        minutes,
        remainingSeconds
    );
}

// ==========================================
// DRAW MUSIC INTERFACE
// ==========================================

void drawMusicInterface() {

    u8g2.clearBuffer();

    // Header
    u8g2.setFont(u8g2_font_5x8_tf);

    u8g2.drawStr(0, 8, "NOW PLAYING");

    int playlistWidth =
        u8g2.getStrWidth(playlistName);

    u8g2.drawStr(
        SCREEN_WIDTH - playlistWidth,
        8,
        playlistName
    );

    u8g2.drawHLine(0, 11, SCREEN_WIDTH);

    // Song title
    u8g2.setFont(u8g2_font_6x12_tf);

    u8g2.drawStr(
        0,
        25,
        tracks[currentTrack].title
    );

    // Artist
    u8g2.setFont(u8g2_font_5x8_tf);

    u8g2.drawStr(
        0,
        35,
        tracks[currentTrack].artist
    );

    // Progress bar
    const uint16_t duration =
        tracks[currentTrack].duration;

    uint8_t progressWidth =
        (uint32_t)elapsedTime * 126 / duration;

    u8g2.drawFrame(0, 41, 128, 5);

    if (progressWidth > 0) {
        u8g2.drawBox(
            1,
            42,
            progressWidth,
            3
        );
    }

    // Playback time
    char elapsed[8];
    char total[8];

    formatTime(elapsedTime, elapsed);
    formatTime(duration, total);

    u8g2.drawStr(0, 56, elapsed);

    int totalWidth = u8g2.getStrWidth(total);

    u8g2.drawStr(
        SCREEN_WIDTH - totalWidth,
        56,
        total
    );

    // Footer
    u8g2.drawHLine(0, 59, SCREEN_WIDTH);

    u8g2.drawStr(
        0,
        64,
        "SPOTIFY DESK PARTNER"
    );

    u8g2.sendBuffer();
}

// ==========================================
// INITIALIZATION
// ==========================================

void setup() {

    Serial.begin(115200);

    Wire.begin(OLED_SDA, OLED_SCL);

    u8g2.begin();
    u8g2.setContrast(255);

    // Start at 1:24 for demonstration
    elapsedTime = 84;

    lastUpdate = millis();

    drawMusicInterface();

    Serial.println("Spotify Desk Partner");
    Serial.println("Simulated playback started.");
}

// ==========================================
// PLAYBACK LOOP
// ==========================================

void loop() {

    unsigned long now = millis();

    if (now - lastUpdate >= 1000) {

        lastUpdate += 1000;

        elapsedTime++;

        // Move to next song when current song ends
        if (elapsedTime >= tracks[currentTrack].duration) {

            elapsedTime = 0;

            currentTrack++;

            if (currentTrack >= TOTAL_TRACKS) {
                currentTrack = 0;
            }

            Serial.print("Now playing: ");
            Serial.println(tracks[currentTrack].title);
        }

        drawMusicInterface();
    }
}
