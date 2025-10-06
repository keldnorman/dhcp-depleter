# Building the Project

## Prerequisites

- [PlatformIO](https://platformio.org/install) installed
- ESP32-C6 development board
- USB cable for programming

## Build Steps

### 1. Install PlatformIO

#### Using pip (Recommended)
```bash
pip install platformio
```

#### Using VS Code
Install the PlatformIO IDE extension for Visual Studio Code.

### 2. Clone the Repository

```bash
git clone https://github.com/keldnorman/dhcp-depleter.git
cd dhcp-depleter
```

### 3. Install Dependencies

PlatformIO will automatically download and install all dependencies when you build the project.

### 4. Build the Project

```bash
pio run
```

### 5. Upload to Device

Connect your ESP32-C6 board via USB and run:

```bash
pio run --target upload
```

### 6. Monitor Serial Output

```bash
pio device monitor
```

Or use the combined command:

```bash
pio run --target upload && pio device monitor
```

## Build Targets

- `pio run` - Build the project
- `pio run --target upload` - Upload to device
- `pio run --target clean` - Clean build files
- `pio device monitor` - Monitor serial output
- `pio run --target erase` - Erase flash memory

## Configuration

The project is configured via `platformio.ini`:

- **Platform**: espressif32
- **Board**: esp32-c6-devkitc-1
- **Framework**: Arduino
- **Monitor Speed**: 115200 baud

### Libraries Used

- **ArduinoJson**: JSON serialization/deserialization
- **AsyncElegantOTA**: Over-the-air updates (optional)

## Customization

You can modify the following in `src/main.cpp`:

```cpp
// Access Point Configuration
const char* ap_ssid = "Sinkhole";          // Change AP name
const char* ap_password = "12345678";      // Change AP password
const IPAddress ap_ip(10, 13, 37, 1);      // Change AP IP
```

## Troubleshooting

### PlatformIO not found
Ensure PlatformIO is in your PATH:
```bash
export PATH=$PATH:~/.platformio/penv/bin
```

### Upload failed
- Check USB cable connection
- Verify correct port in PlatformIO
- Try holding BOOT button during upload

### Build errors
- Ensure you have the latest PlatformIO Core
- Try cleaning the project: `pio run --target clean`
- Delete `.pio` directory and rebuild

## Development

For development, you can use:

```bash
# Start a serial monitor
pio device monitor

# Check for code issues
pio check

# Run tests (if available)
pio test
```

## Platform-Specific Notes

### ESP32-C6

The ESP32-C6 is a RISC-V based microcontroller with:
- WiFi 6 (802.11ax) support
- Bluetooth 5 (LE)
- Lower power consumption
- USB Serial/JTAG controller

Make sure you have the latest espressif32 platform installed:

```bash
pio platform update espressif32
```

## Alternative: Arduino IDE

If you prefer Arduino IDE:

1. Install ESP32 board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   
2. Install ESP32 boards:
   - Tools → Board → Boards Manager
   - Search for "esp32" and install

3. Install libraries:
   - Sketch → Include Library → Manage Libraries
   - Install "ArduinoJson" (version 7.x)

4. Select board:
   - Tools → Board → ESP32 Arduino → ESP32C6 Dev Module

5. Configure board:
   - Tools → USB CDC On Boot → Enabled
   - Tools → Flash Size → 4MB

6. Upload the sketch
