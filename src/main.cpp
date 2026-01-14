#include <Arduino.h>
#include "config.h"
#include "time_manager.h"
#include "display_manager.h"

// ============================================
// Global Objects
// ============================================
TimeManager timeManager;
DisplayManager displayManager;

// ============================================
// Setup Function
// ============================================
void setup() {
    // Initialize Serial communication
    Serial.begin(SERIAL_BAUD);
    delay(1000);

    // Print startup banner
    Serial.println("\n\n========================================");
    Serial.println(PROJECT_NAME);
    Serial.print("Version: ");
    Serial.println(VERSION);
    Serial.println("========================================");
    Serial.println("Phase 1: Display Clock Test");
    Serial.println("========================================\n");

    // Initialize TimeManager
    Serial.println("Initializing TimeManager...");
    if (timeManager.begin()) {
        Serial.println("TimeManager initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize TimeManager!");
    }

    // Set initial time manually (for testing)
    // TODO: In Phase 2, this will be replaced with BLE sync
    Serial.println("\nSetting initial time...");
    timeManager.setDate(14, 1, 2026);      // Jan 14, 2026 (Wednesday)
    timeManager.setTime(12, 34, 0);        // 12:34:00
    Serial.println("Time set to: 2026-01-14 12:34:00");

    // Initialize DisplayManager
    Serial.println("\nInitializing DisplayManager...");
    if (displayManager.begin()) {
        Serial.println("DisplayManager initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize DisplayManager!");
    }

    // Set status indicators
    displayManager.setBLEStatus(false);    // No BLE yet
    displayManager.setTimeSyncStatus(true); // Manually synced

    // Display initial clock
    Serial.println("\nDisplaying initial clock...");
    displayManager.showClock(
        timeManager.getTimeString(false),  // 24-hour format
        timeManager.getDateString(),
        timeManager.getDayOfWeekString()
    );

    Serial.println("\n========================================");
    Serial.println("READY - Clock should be displayed!");
    Serial.println("========================================");
    Serial.println("Expected on display:");
    Serial.println("  - Day of week: Wednesday");
    Serial.println("  - Time: 12:34");
    Serial.println("  - Date: Jan 14, 2026");
    Serial.println("  - BLE status: ---");
    Serial.println("  - Sync status: SYNC");
    Serial.println("\nClock will update every second...\n");
}

// ============================================
// Loop Function
// ============================================
void loop() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    // Update display every second
    if (now - lastUpdate >= 1000) {
        lastUpdate = now;

        // Get current time
        String timeStr = timeManager.getTimeString(false);  // 24-hour
        String dateStr = timeManager.getDateString();
        String dayStr = timeManager.getDayOfWeekString();

        // Update display
        displayManager.showClock(timeStr, dateStr, dayStr);

        // Print to serial (for debugging)
        Serial.print("Clock updated: ");
        Serial.print(dayStr);
        Serial.print(", ");
        Serial.print(timeStr);
        Serial.print(", ");
        Serial.println(dateStr);
    }

    // Small delay to prevent overwhelming CPU
    delay(10);
}
