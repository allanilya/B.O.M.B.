ESP32-L Alarm Clock - Complete Implementation Plan
Project Status
✅ Hardware Verified:

E-ink display (GDEY037T03) working - pins: CS=27, DC=14, RST=12, BUSY=13
I2S audio working - pins: DOUT=22, BCLK=25, LRC=26
Button working - pin: GPIO 4
No pin conflicts
✅ Existing Modules:

Button module (complete with debouncing, edge detection)
Audio module (basic tone generation only)
Config.h (all pins verified)
User Requirements
Alarm Configuration: iOS app only via BLE (no button-based alarm setting)
Button Behavior: Short press = snooze 5min, long press (2s) = dismiss alarm
Alarm Sounds: Support BOTH built-in tones AND MP3/WAV files from SPIFFS
Off-grid: No WiFi/NTP - time syncs from iOS via BLE
Architecture Overview
5 Core Modules:


TimeManager → Keeps time using ESP32 RTC + BLE sync
AlarmManager → Stores/schedules alarms in NVS
DisplayManager → E-ink UI with smart refresh
AudioPlayer → Tones + MP3/WAV playback
BLETimeSync → iOS app communication
Integration: Modules communicate via callbacks in main.cpp

Implementation Phases
Phase 1: Foundation - Display Clock (6-8 hours)
Goal: Working clock display before adding complexity

Tasks:

Create time_manager.h/cpp

ESP32 RTC timekeeping
Formatted string output (time/date)
Sync status tracking
Create display_manager.h/cpp

GxEPD2 initialization
showClock() method with large fonts
Partial vs full refresh logic
Status icons area (BLE, sync)
Update main.cpp

Initialize TimeManager + DisplayManager
Update display every second
Manually set time for testing
Milestone: Clock shows time/date on e-ink display

Phase 2: Time Synchronization (5-6 hours)
Goal: Sync time from iOS app via BLE

Tasks:

Create ble_time_sync.h/cpp (time service only)

BLE server with Current Time Service
Time sync callback to TimeManager
Connection status tracking
Integrate with TimeManager

Callback sets RTC time
Display shows BLE connection icon
Test with iOS BLE app (nRF Connect)
Milestone: Time syncs from iOS, BLE status visible on display

Phase 3: Alarm Management (8-10 hours)
Goal: Store and schedule alarms (no audio yet)

Tasks:

Create alarm_manager.h/cpp

AlarmData struct (hour, minute, days, sound, enabled)
NVS storage (Preferences library)
Alarm scheduling logic
Snooze/dismiss functionality
Check alarms every second
Extend BLE (alarm service)

Custom BLE service for alarms
Add/delete/list characteristics (JSON format)
Connect callbacks to AlarmManager
Update DisplayManager

showAlarmRinging() screen
Show next alarm on main screen
Test trigger detection (Serial logs)
Integrate button controls

Short press = snooze
Long press = dismiss
Test without audio first
Milestone: Alarms trigger on schedule, button controls work, visible on display

Phase 4: Audio - Tones (3-4 hours)
Goal: Basic alarm sounds working

Tasks:

Create audio_player.h/cpp

Refactor audio_test.cpp
Add 3-4 built-in tone patterns
playBuiltInAlarm(toneNumber) method
Integrate with AlarmManager

Play tone when alarm triggers
Stop on snooze/dismiss
Test end-to-end workflow
Milestone: Alarms play built-in tones, full workflow functional

Phase 5: Audio - File Playback (4-5 hours)
Goal: Add MP3/WAV support from SPIFFS

Tasks:

Add ESP32-audioI2S library

Update platformio.ini: schreibfaul1/ESP32-audioI2S@^3.0.0
Test basic MP3 playback
Extend AudioPlayer

playFile() with MP3/WAV support
Volume control
Looping until stopped
Test with sample files
Prepare SPIFFS

Add 3-5 MP3 files to /data/alarms/
Upload: pio run --target uploadfs
Test file playback
Update alarm configuration

Alarms specify "tone1" or "gentle_chimes.mp3"
Test mixed alarms
Milestone: Complete alarm clock with MP3/WAV support

Phase 6: Polish (6-8 hours)
Tasks:

Error handling

Missing MP3 files → fallback to tone
NVS corruption recovery
Display error messages
Power optimization

Reduce display refresh frequency
Optimize BLE advertising
Testing

All alarm types
Edge cases (midnight, multiple alarms)
Long-term stability
Milestone: Production-ready alarm clock

Phase 7: SD Card Support (OPTIONAL - 4-6 hours)
Goal: Add unlimited sound storage (only if 3-5 alarm sounds isn't enough)

Tasks:

Wire SD card module (uses SPI pins)
Replace SPIFFS.open() with SD.open()
Add SD card file browser to iOS app
Test with 20+ MP3 files
When to implement:

You want 10+ different alarm sounds
You want full songs (3+ minutes) as alarms
You want to swap sounds without reflashing
Critical Files
File	Purpose	Estimated Lines
src/time_manager.cpp	RTC timekeeping + sync	~150
src/alarm_manager.cpp	Alarm storage + scheduling	~300
src/ble_time_sync.cpp	BLE services (time + alarms)	~250
src/display_manager.cpp	E-ink UI with smart refresh	~200
src/audio_player.cpp	Tones + MP3/WAV playback	~200
src/main.cpp	Integration + event handling	~150
Total: ~1250 lines of new code

Key Design Decisions
1. RTC Instead of NTP
Off-grid requirement (no WiFi)
BLE sync from iOS (user's phone is accurate)
Power efficient
2. NVS for Alarms
Fast access vs SPIFFS
Automatic wear leveling
SPIFFS reserved for audio files
3. Callback Pattern for BLE
Decouples modules
Easy to test independently
Flexible for future features
4. Partial Display Updates
Full refresh: ~2 seconds (slow)
Partial refresh: instant (for time updates)
Full refresh only hourly (prevent ghosting)
5. ESP32-audioI2S Library
Supports MP3, WAV, AAC, FLAC
SPIFFS file playback
Streaming decoder (low RAM usage)
Required Libraries

# platformio.ini
lib_deps =
    zinggjm/GxEPD2@^1.5.9
    adafruit/Adafruit GFX Library@^1.12.4
    schreibfaul1/ESP32-audioI2S@^3.0.0    # Phase 5
Built-in (no install):

Preferences.h (NVS)
time.h (RTC)
SPIFFS.h
BLEDevice.h
Testing Strategy
Per-Phase Testing
Phase 1: Clock updates every second, manual time setting works
Phase 2: BLE connects, time sync updates display
Phase 3: Alarms trigger at correct time, button snooze/dismiss works
Phase 4: Tones play, stop on button press
Phase 5: MP3 files play, mixed alarms work
End-to-End Test
Set alarm via iOS app
Wait for trigger time
Verify audio plays (tone or MP3)
Test snooze → re-trigger after 5min
Test dismiss → alarm stops
Verify alarm persists after power cycle (NVS)
