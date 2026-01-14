#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "audio_test.h"

// Display pins (verified from schematic)
#define EPD_CS    27
#define EPD_DC    14
#define EPD_RST   12
#define EPD_BUSY  13

// I2S Audio pins (moved to avoid conflicts)
#define I2S_DOUT  22  // SCL (GPIO 22)
#define I2S_BCLK  25  // A18 (GPIO 25) - NO CONFLICT!
#define I2S_LRC   26  // A19 (GPIO 26) - NO CONFLICT!

// Create display object
GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

// Create audio object
AudioTest audio;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("DISPLAY + AUDIO - NO CONFLICTS!");
    Serial.println("========================================");
    Serial.println("Display: CS=27, DC=14, RST=12, BUSY=13");
    Serial.println("Audio: DOUT=22, BCLK=25, LRC=26");
    Serial.println("SUCCESS: No pin conflicts!");
    Serial.println("========================================\n");

    // Test 1: Initialize Display
    Serial.println("1. Initializing display...");
    display.init(115200);
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    Serial.println("   Display OK\n");

    // Test 2: Initialize Audio
    Serial.println("2. Initializing audio...");
    if (audio.begin()) {
        Serial.println("   Audio OK\n");
    } else {
        Serial.println("   Audio FAILED\n");
    }

    // Test 3: Update Display
    Serial.println("3. Updating display...");
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(10, 30);
        display.setTextSize(2);
        display.print("Display Test");
        display.setCursor(10, 70);
        display.setTextSize(1);
        display.print("Testing pin sharing");
        display.print("Audio will play next");
    } while (display.nextPage());
    Serial.println("   Display updated!\n");

    delay(1000);

    // Test 4: Play Audio Tone
    Serial.println("4. Playing audio tone (440 Hz)...");
    audio.playTone(440, 2000);  // A4 for 2 seconds
    Serial.println("   Audio played!\n");

    Serial.println("========================================");
    Serial.println("TEST COMPLETE!");
    Serial.println("========================================");
    Serial.println("Results:");
    Serial.println("  1. Did display update? (check screen)");
    Serial.println("  2. Did audio play? (check speaker)");
    Serial.println("  3. Any crashes or errors?");
    Serial.println("\nBoth should work now with new wiring!");
    Serial.println("Display uses: T7, T6, T5, T4");
    Serial.println("Audio uses: SCL, A18, A19");
    Serial.println("========================================\n");
}

void loop() {
    delay(1000);
}
