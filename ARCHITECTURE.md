# System Architecture

## Overview

This document describes the architecture of the DHCP Depleter system.

## Component Diagram

```
                         ┌─────────────────────────────────┐
                         │    User Device (Phone/Laptop)   │
                         │                                 │
                         │  1. Connects to "Sinkhole" WiFi │
                         │  2. Opens http://10.13.37.1     │
                         └────────────┬────────────────────┘
                                      │
                                      │ WiFi Connection
                                      │
                         ┌────────────▼────────────────────┐
                         │      ESP32-C6 Device            │
                         │  ┌───────────────────────────┐  │
                         │  │  Access Point Mode        │  │
                         │  │  SSID: Sinkhole           │  │
                         │  │  IP: 10.13.37.1           │  │
                         │  │  Password: 12345678       │  │
                         │  └───────────────────────────┘  │
                         │                                 │
                         │  ┌───────────────────────────┐  │
                         │  │   Web Server              │  │
                         │  │   - Serves web interface  │  │
                         │  │   - Handles API requests  │  │
                         │  │   - Returns JSON data     │  │
                         │  └───────────────────────────┘  │
                         │                                 │
                         │  ┌───────────────────────────┐  │
                         │  │   Station Mode            │  │
                         │  │   - Connects to target    │  │
                         │  │   - Randomizes MAC        │  │
                         │  │   - Requests DHCP         │  │
                         │  └───────────────────────────┘  │
                         └────────────┬────────────────────┘
                                      │
                                      │ WiFi Connection
                                      │ (with random MAC)
                                      │
                         ┌────────────▼────────────────────┐
                         │    Target WiFi Network          │
                         │                                 │
                         │  ┌───────────────────────────┐  │
                         │  │   DHCP Server             │  │
                         │  │   - Receives requests     │  │
                         │  │   - Assigns IP addresses  │  │
                         │  │   - Pool gets depleted    │  │
                         │  └───────────────────────────┘  │
                         └─────────────────────────────────┘
```

## Data Flow

### 1. Initial Setup
```
┌─────┐       ┌──────────┐       ┌────────────┐
│User │──────>│ ESP32-C6 │──────>│  Creates   │
│     │ Power │          │       │ "Sinkhole" │
│     │  On   │          │       │  AP        │
└─────┘       └──────────┘       └────────────┘
```

### 2. Network Scanning
```
┌──────┐      ┌──────────┐      ┌────────────┐      ┌────────┐
│User  │─────>│Web UI    │─────>│ ESP32-C6   │─────>│ Target │
│      │ Scan │ Request  │ /scan│ WiFi Scan  │      │Networks│
│      │<─────│ Response │<─────│ JSON Data  │<─────│        │
└──────┘ List └──────────┘      └────────────┘      └────────┘
```

### 3. Connection Test
```
┌──────┐     ┌──────────┐     ┌──────────┐     ┌────────┐
│User  │────>│ Web UI   │────>│ ESP32-C6 │────>│Target  │
│      │Test │  /test   │     │ Connect  │WiFi │Network │
│      │     │          │     │          │     │        │
│      │<────│ Response │<────│DHCP OK?  │<────│DHCP    │
└──────┘     └──────────┘     └──────────┘     │Server  │
  │                                             └────────┘
  └─> Shows IP address if successful
```

### 4. Attack Sequence
```
┌──────┐     ┌──────────┐     ┌──────────┐     ┌────────┐
│User  │────>│ Web UI   │────>│ ESP32-C6 │     │Target  │
│      │Start│ /start   │     │          │     │Network │
└──────┘     └──────────┘     └────┬─────┘     └───┬────┘
                                    │               │
                         Loop:      │               │
                         1. Generate new MAC        │
                         2. Disconnect              │
                         3. Connect with new MAC ──>│
                         4. Request DHCP         ──>│
                         5. Get IP address      <───│
                         6. Increment counter       │
                         7. Repeat                  │
```

### 5. Status Updates
```
┌──────┐     ┌──────────┐     ┌──────────┐
│User  │────>│ Web UI   │────>│ ESP32-C6 │
│      │Poll │ /status  │     │          │
│      │<────│ JSON     │<────│ Stats    │
└──────┘     └──────────┘     └──────────┘
  │
  └─> Shows: Addresses obtained, Runtime
```

## State Machine

```
┌─────────┐
│  IDLE   │<──────────────────────────────┐
└────┬────┘                                │
     │ User clicks "Scan"                  │
     ▼                                     │
┌─────────┐                                │
│SCANNING │                                │
└────┬────┘                                │
     │ Networks found                      │
     ▼                                     │
┌─────────┐                                │
│  READY  │                                │
└────┬────┘                                │
     │ User clicks "Test"                  │
     ▼                                     │
┌─────────┐                                │
│TESTING  │───> Failed ────────────────────┤
└────┬────┘                                │
     │ Success                             │
     ▼                                     │
┌─────────┐                                │
│VALIDATED│                                │
└────┬────┘                                │
     │ User clicks "Start"                 │
     ▼                                     │
┌─────────┐                                │
│ATTACKING│───> User clicks "Stop" ────────┤
└─────────┘                                │
     │                                     │
     └─> Loop: Get DHCP addresses          │
                                           │
                                           │
```

## Network Architecture

### Dual WiFi Mode

The ESP32-C6 operates in dual mode:

1. **Access Point (AP) Mode**
   - Always active
   - Serves web interface
   - IP: 10.13.37.0/24
   - Independent of attack

2. **Station (STA) Mode**
   - Used for attack
   - Connects to target
   - MAC randomized each connection
   - Gets DHCP from target

```
             ┌──────────────────────────────┐
             │        ESP32-C6              │
             │                              │
             │  ┌────────┐    ┌──────────┐  │
             │  │   AP   │    │   STA    │  │
             │  │  Mode  │    │   Mode   │  │
             │  │        │    │          │  │
User Device ─┼──┤10.13.  │    │ Target   ├──┼─ Target Network
             │  │37.1    │    │ Network  │  │
             │  │        │    │          │  │
             │  │ Static │    │ Random   │  │
             │  │  MAC   │    │  MAC     │  │
             │  └────────┘    └──────────┘  │
             │                              │
             └──────────────────────────────┘
```

## API Endpoints

### GET /
- **Purpose**: Serve web interface
- **Response**: HTML page
- **Authentication**: None

### GET /scan
- **Purpose**: Scan for WiFi networks
- **Response**: JSON array of networks
- **Example**:
  ```json
  {
    "networks": [
      {
        "ssid": "MyWiFi",
        "rssi": -45,
        "encryption": "Secured"
      }
    ]
  }
  ```

### GET /test
- **Purpose**: Test connection to target
- **Parameters**: `ssid`, `password`
- **Response**: Success/failure with IP
- **Example**:
  ```json
  {
    "success": true,
    "ip": "192.168.1.123"
  }
  ```

### GET /start
- **Purpose**: Start DHCP attack
- **Parameters**: `ssid`, `password`
- **Response**: Confirmation

### GET /stop
- **Purpose**: Stop attack
- **Response**: Confirmation

### GET /status
- **Purpose**: Get current statistics
- **Response**: Attack status and stats
- **Example**:
  ```json
  {
    "running": true,
    "addresses": 42,
    "runtime": "2m 15s"
  }
  ```

## Security Considerations

### MAC Address Randomization

```
┌──────────────────────────────────────┐
│ MAC Address Format                   │
├──────────────────────────────────────┤
│ Byte 0: 0x02 (locally administered)  │
│ Bytes 1-5: Random                    │
└──────────────────────────────────────┘

Example MACs generated:
- 02:A3:4F:12:B8:9C
- 02:71:E5:8D:3A:F2
- 02:99:2D:4B:C7:61
```

### Isolation

- AP and STA modes are independent
- Control interface (AP) always available
- Attack doesn't affect user's connection to device

## Performance

### Resource Usage
- **Memory**: ~100KB RAM for WiFi stack
- **CPU**: Minimal (most time spent waiting)
- **Power**: ~150mA during attack

### Attack Rate
- **Throttled**: 1 request per second
- **Reason**: Prevent device overload
- **Tunable**: Can be adjusted in code

### Typical Results
| Network Type | Pool Size | Time to Exhaust |
|--------------|-----------|-----------------|
| Home Router  | 50-254    | 2-10 minutes    |
| Small Office | 100-500   | 10-30 minutes   |
| Enterprise   | 500-5000  | 30+ minutes     |

## Code Structure

```
src/main.cpp
├── Global Variables
│   ├── AP configuration
│   ├── State variables
│   └── Server instance
│
├── setup()
│   ├── Initialize serial
│   ├── Setup AP
│   ├── Setup web server
│   └── Print QR code
│
├── loop()
│   ├── Handle web requests
│   └── Perform attack (if running)
│
├── Web Handlers
│   ├── handleRoot() - Serve HTML
│   ├── handleScan() - WiFi scan
│   ├── handleTest() - Test connection
│   ├── handleStart() - Start attack
│   ├── handleStop() - Stop attack
│   └── handleStatus() - Get stats
│
├── Attack Functions
│   ├── performDHCPAttack() - Main attack loop
│   ├── randomizeMAC() - Generate random MAC
│   └── generateQRCode() - Create WiFi QR
│
└── Helper Functions
    └── Various utilities
```

## Dependencies

```
platformio.ini
├── espressif32 platform
├── ESP32-C6 board definition
├── Arduino framework
└── Libraries
    ├── ArduinoJson (JSON handling)
    └── Built-in ESP32 WiFi libraries
```

## Deployment

```
Development Machine
       │
       │ USB Cable
       ▼
   ESP32-C6 Device
       │
       │ Flash firmware
       ▼
   Deployed Device
       │
       │ Creates WiFi AP
       ▼
   Ready for Use
```

## Future Enhancements

Potential improvements:
1. OTA (Over-The-Air) updates
2. Statistics logging to SD card
3. Multiple target support
4. Scheduled attacks
5. OLED display for status
6. Battery level monitoring
7. Custom attack patterns
8. Rate limiting configuration via UI

---

For implementation details, see the source code in `src/main.cpp`.
