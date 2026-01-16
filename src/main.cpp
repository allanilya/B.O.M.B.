#include <Arduino.h>
#include "config.h"
#include "time_manager.h"
#include "display_manager.h"
#include "ble_time_sync.h"
#include "alarm_manager.h"
#include "button.h"
#include "audio_test.h"

// ============================================
// Global Objects
// ============================================
TimeManager timeManager;
DisplayManager displayManager;
BLETimeSync bleSync;
AlarmManager alarmManager;
Button button(BUTTON_PIN);
AudioTest audioObj;

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
    Serial.println("Phase 2: BLE Time Sync Test");
    Serial.println("========================================\n");

    // Initialize TimeManager
    Serial.println("Initializing TimeManager...");
    if (timeManager.begin()) {
        Serial.println("TimeManager initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize TimeManager!");
    }

    // Initialize DisplayManager
    Serial.println("\nInitializing DisplayManager...");
    if (displayManager.begin()) {
        Serial.println("DisplayManager initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize DisplayManager!");
    }

    // Initialize BLE Time Sync
    Serial.println("\nInitializing BLE Time Sync...");
    if (bleSync.begin(BLE_DEVICE_NAME)) {
        Serial.println("BLE Time Sync initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize BLE Time Sync!");
    }

    // Set BLE callback to update time
    bleSync.setTimeSyncCallback([](time_t timestamp) {
        timeManager.setTimestamp(timestamp);
        Serial.println(">>> Time synchronized from BLE!");
    });

    // Initialize AlarmManager
    Serial.println("\nInitializing AlarmManager...");
    if (alarmManager.begin()) {
        Serial.println("AlarmManager initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize AlarmManager!");
    }

    // Set alarm callback
    alarmManager.setAlarmCallback([](uint8_t alarmId) {
        Serial.print(">>> ALARM CALLBACK: Alarm ");
        Serial.print(alarmId);
        Serial.println(" is ringing!");

        // Get alarm data to determine which sound to play
        AlarmData alarm;
        if (alarmManager.getAlarm(alarmId, alarm)) {
            // Map sound name to frequency
            uint16_t frequency;
            if (alarm.sound == "tone2") {
                frequency = 523;  // C5 note
            } else if (alarm.sound == "tone3") {
                frequency = 659;  // E5 note
            } else {
                frequency = 440;  // A4 note (default tone1)
            }

            // Play very short tone burst (non-blocking approach)
            audioObj.playTone(frequency, 50);  // 50ms burst only
            Serial.print(">>> AUDIO: Playing tone at ");
            Serial.print(frequency);
            Serial.println(" Hz (50ms burst)");
        }
    });

    // Initialize Button
    Serial.println("\nInitializing Button...");
    button.begin();
    Serial.println("Button initialized!");

    // Initialize Audio
    Serial.println("\nInitializing Audio...");
    if (audioObj.begin()) {
        Serial.println("Audio initialized!");
    } else {
        Serial.println("ERROR: Failed to initialize Audio!");
    }

    // Set initial status indicators
    displayManager.setBLEStatus(false);     // Will update when connected
    displayManager.setTimeSyncStatus(false); // Not synced yet

    // Display initial clock (will show default time)
    Serial.println("\nDisplaying initial clock...");
    uint8_t hour, minute, second;
    timeManager.getTime(hour, minute, second);
    displayManager.showClock(
        timeManager.getTimeString(true),  // 12-hour format with AM/PM
        timeManager.getDateString(),
        timeManager.getDayOfWeekString(),
        second
    );

    Serial.println("\n========================================");
    Serial.println("READY - Waiting for BLE time sync!");
    Serial.println("========================================");
    Serial.println("Instructions:");
    Serial.println("1. Open BLE app on your phone (LightBlue or nRF Connect)");
    Serial.println("2. Scan for 'ESP32-L Alarm'");
    Serial.println("3. Connect to the device");
    Serial.println("4. Find 'DateTime' characteristic");
    Serial.println("5. Write: YYYY-MM-DD HH:MM:SS");
    Serial.println("   Example: 2026-01-14 15:30:00");
    Serial.println("\nDisplay shows:");
    Serial.println("  - BLE: --- (not connected)");
    Serial.println("  - SYNC: ???? (not synced)");
    Serial.println("\nAfter sync, will show:");
    Serial.println("  - BLE: BLE (connected)");
    Serial.println("  - SYNC: SYNC (synced)");
    Serial.println("========================================\n");
}

// ============================================
// Loop Function
// ============================================
void loop() {
    static unsigned long lastUpdate = 0;
    static bool lastBLEStatus = false;
    static unsigned long lastToneStart = 0;  // Track when tone was started
    static bool wasRingingLastLoop = false;  // Track alarm state
    static bool displayUpdatedForAlarm = false;  // Track if alarm display shown
    unsigned long now = millis();

    // Update BLE
    bleSync.update();

    // Update button
    button.update();

    // Check if BLE connection status changed
    bool bleConnected = bleSync.isConnected();
    if (bleConnected != lastBLEStatus) {
        lastBLEStatus = bleConnected;
        displayManager.setBLEStatus(bleConnected);

        if (bleConnected) {
            Serial.println("\n>>> BLE STATUS: Connected");
        } else {
            Serial.println("\n>>> BLE STATUS: Disconnected");
        }
    }

    // Update time sync status
    displayManager.setTimeSyncStatus(timeManager.isSynced());

    // Handle button presses for alarm control
    bool buttonPressed = button.wasPressed();
    bool buttonDoubleClicked = button.wasDoubleClicked();

    // Debug: Log any button activity
    if (buttonPressed) {
        Serial.println("\n>>> BUTTON: Single press detected!");
    }
    if (buttonDoubleClicked) {
        Serial.println("\n>>> BUTTON: Double-click detected!");
    }

    if (alarmManager.isAlarmRinging()) {
        // Single click = snooze
        if (buttonPressed) {
            alarmManager.snoozeAlarm();
            audioObj.stop();
            lastToneStart = 0;  // Reset tone timer
            Serial.println("\n>>> BUTTON: Alarm snoozed (5 minutes)");
            Serial.println(">>> AUDIO: Stopped");
        }

        // Double-click = dismiss
        if (buttonDoubleClicked) {
            alarmManager.dismissAlarm();
            audioObj.stop();
            lastToneStart = 0;  // Reset tone timer
            Serial.println("\n>>> BUTTON: Alarm dismissed");
            Serial.println(">>> AUDIO: Stopped");
        }
    }

    // Handle alarm audio (runs every loop for responsiveness)
    if (alarmManager.isAlarmRinging()) {
        // If alarm just started, initialize timer and show alarm display
        if (!wasRingingLastLoop) {
            lastToneStart = 0;  // Force immediate play
            wasRingingLastLoop = true;
            displayUpdatedForAlarm = false;  // Need to show alarm screen

            // Show alarm screen immediately (only once)
            uint8_t hour, minute, second;
            timeManager.getTime(hour, minute, second);
            String timeStr = timeManager.getTimeString(true);
            displayManager.showAlarmRinging(timeStr);
            displayUpdatedForAlarm = true;
        }

        // Play tone bursts frequently for continuous sound
        if (now - lastToneStart >= 60) {  // Restart every 60ms
            uint8_t alarmId = alarmManager.getRingingAlarmId();
            AlarmData alarm;
            if (alarmManager.getAlarm(alarmId, alarm)) {
                uint16_t frequency = (alarm.sound == "tone2") ? 523 :
                                   (alarm.sound == "tone3") ? 659 : 440;
                audioObj.playTone(frequency, 50);  // 50ms burst
            }
            lastToneStart = now;
        }
    } else {
        // Reset state when alarm stops
        if (wasRingingLastLoop) {
            wasRingingLastLoop = false;
            lastToneStart = 0;
            displayUpdatedForAlarm = false;

            // Force display update to return to clock
            lastUpdate = 0;
        }
    }

    // Update display every second (only for normal clock, not alarm screen)
    if (now - lastUpdate >= 1000) {
        lastUpdate = now;

        // Get current time
        uint8_t hour, minute, second;
        timeManager.getTime(hour, minute, second);
        String timeStr = timeManager.getTimeString(true);  // 12-hour with AM/PM
        String dateStr = timeManager.getDateString();
        String dayStr = timeManager.getDayOfWeekString();

        // Check alarms (gets day of week from tm struct)
        struct tm timeinfo;
        time_t now_t = time(nullptr);
        localtime_r(&now_t, &timeinfo);
        alarmManager.checkAlarms(hour, minute, timeinfo.tm_wday);

        // Only update display if not showing alarm (alarm display updates once above)
        if (!alarmManager.isAlarmRinging()) {
            displayManager.showClock(timeStr, dateStr, dayStr, second);
        }

        // Print to serial (for debugging)
        Serial.print("Clock: ");
        Serial.print(timeStr);
        Serial.print(" | BLE: ");
        Serial.print(bleConnected ? "Connected" : "---");
        Serial.print(" | Sync: ");
        Serial.print(timeManager.isSynced() ? "YES" : "NO");
        Serial.print(" | Alarm: ");
        Serial.println(alarmManager.isAlarmRinging() ? "RINGING" : "---");
    }

    // Small delay to prevent overwhelming CPU
    delay(10);
}
