# dhcp-depleter
Wireless DHCP IP Depletion Tester

Test whether your DHCP server or wireless access point configuration is resilient to DHCP address-exhaustion attacks.

## ⚠️ WARNING
This tool is designed for **TESTING PURPOSES ONLY** on networks you own or have explicit permission to test. Unauthorized use of this tool may be illegal in your jurisdiction.

## Description

This ESP32-C6 device creates a wireless access point that you can connect to and use to test DHCP exhaustion attacks against target networks. The device will repeatedly request IP addresses from the target DHCP server while constantly changing its MAC address, potentially depleting the available IP address pool.

## Features

- 📡 Creates a WiFi access point named "Sinkhole"
- 🌐 Web-based interface for easy configuration
- 🔍 Network scanner to detect available WiFi networks
- 🔐 Support for WPA2/WPA3 secured networks
- 🎯 Test connectivity before starting attack
- 🔄 Automatic MAC address randomization
- 📊 Real-time statistics and monitoring
- 🔥 Continuous DHCP address exhaustion

## Hardware Requirements

- ESP32-C6 Development Board (e.g., ESP32-C6-DevKitC-1)
- USB cable for programming and power

## Software Requirements

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- ESP32 board support

## Installation

### Using PlatformIO (Recommended)

1. Clone this repository:
   ```bash
   git clone https://github.com/keldnorman/dhcp-depleter.git
   cd dhcp-depleter
   ```

2. Open the project in PlatformIO (VS Code with PlatformIO extension)

3. Build and upload:
   ```bash
   pio run --target upload
   ```

4. Monitor serial output:
   ```bash
   pio device monitor
   ```

### Using Arduino IDE

1. Install ESP32 board support in Arduino IDE
2. Copy the contents of `src/main.cpp` to a new Arduino sketch
3. Install required libraries:
   - ArduinoJson (version 7.0.4 or higher)
4. Select board: "ESP32C6 Dev Module"
5. Upload the sketch

## Usage

### 1. Connect to the Device

1. Power on the ESP32-C6 device
2. Connect to the WiFi network named **"Sinkhole"**
   - Password: **12345678**
3. You can scan a QR code (check serial output) or manually enter the credentials

### 2. Access Web Interface

1. Open a web browser
2. Navigate to: **http://10.13.37.1**

### 3. Configure Target Network

1. Click "Scan WiFi Networks" to detect available networks
2. Select your target network from the dropdown
3. Enter the WPA2/WPA3 password if the network is secured

### 4. Test Connection

1. Click "Test Connection" to verify the device can connect to the target
2. If successful, the "Start Attack" button will be enabled

### 5. Run the Attack

1. Click "Start Attack" to begin the DHCP depletion test
2. Monitor the statistics in real-time:
   - Number of addresses obtained
   - Attack runtime
3. Click "Stop Attack" to end the test

## How It Works

The DHCP depletion attack works by:

1. **MAC Address Randomization**: Before each DHCP request, the device generates a random MAC address
2. **DHCP Discovery**: The device connects to the target network with the new MAC address
3. **IP Address Allocation**: The DHCP server assigns an IP address to the "new" device
4. **Repeat**: The process continues indefinitely, exhausting the DHCP pool

This simulates an attack where multiple devices rapidly join a network, potentially:
- Depleting available IP addresses
- Preventing legitimate devices from obtaining IP addresses
- Testing DHCP server rate limiting and security features

## Monitoring

The serial console provides detailed information:
- Device startup information
- Access point configuration
- Connection attempts and results
- MAC addresses being used
- IP addresses obtained
- Attack statistics

## Mitigation Strategies

If your network is vulnerable to this attack, consider:

1. **DHCP Snooping**: Enable on managed switches to limit DHCP requests per port
2. **MAC Address Limits**: Configure switches to limit MAC addresses per port
3. **Short Lease Times**: Use shorter DHCP lease times (with caution)
4. **Static IP Allocation**: Use static IPs for critical infrastructure
5. **802.1X Authentication**: Implement port-based network access control
6. **Rate Limiting**: Configure DHCP servers to rate-limit requests
7. **Monitoring**: Set up alerts for unusual DHCP activity

## Troubleshooting

### Device won't connect to target network
- Verify the SSID and password are correct
- Check if the network uses enterprise authentication (not supported)
- Ensure the ESP32-C6 is within range

### Web interface not accessible
- Verify you're connected to the "Sinkhole" WiFi network
- Try accessing http://10.13.37.1 directly
- Check serial output for any error messages

### Attack not obtaining addresses
- The DHCP server may be out of addresses (attack succeeded!)
- DHCP server may have rate limiting enabled
- Network may have security features blocking the attack

## Legal Disclaimer

This tool is provided for educational and authorized testing purposes only. Users must:

- Only test networks they own or have explicit written permission to test
- Comply with all applicable laws and regulations
- Understand that unauthorized network testing may be illegal
- Accept full responsibility for their use of this tool

The authors and contributors are not responsible for any misuse or damage caused by this tool.

## Technical Details

- **Platform**: ESP32-C6 (RISC-V based)
- **Framework**: Arduino
- **Language**: C++
- **Libraries**: ArduinoJson, ESP32 WiFi
- **Memory**: Uses partition scheme with large app space

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

This project is provided as-is for educational purposes.

## Support

For issues, questions, or contributions, please use the GitHub issue tracker.

## Acknowledgments

Created for network security testing and education.
