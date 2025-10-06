# Quick Start Guide

Get started with DHCP Depleter in under 5 minutes!

## Prerequisites

- ESP32-C6 development board
- USB cable
- Computer with PlatformIO installed
- Target WiFi network (that you own or have permission to test)

## Step 1: Install PlatformIO (2 minutes)

```bash
pip install platformio
```

Or install the PlatformIO extension in VS Code.

## Step 2: Build and Upload (3 minutes)

```bash
# Clone the repository
git clone https://github.com/keldnorman/dhcp-depleter.git
cd dhcp-depleter

# Connect your ESP32-C6 via USB

# Build and upload
pio run --target upload

# Monitor output (optional)
pio device monitor
```

## Step 3: Connect to Device (1 minute)

### Option A: Scan QR Code
1. Look at the serial monitor output
2. Use your phone's camera to scan the WiFi QR code

### Option B: Manual Connection
1. Open WiFi settings on your phone/laptop
2. Connect to network: **Sinkhole**
3. Password: **12345678**

## Step 4: Access Web Interface (1 minute)

1. Open a web browser
2. Navigate to: **http://10.13.37.1**
3. You should see the DHCP Depleter interface

## Step 5: Run Your First Test (2 minutes)

1. **Scan Networks**
   - Click "Scan WiFi Networks" button
   - Wait for the list to populate

2. **Select Target**
   - Choose your target network from the dropdown
   - Enter the WiFi password (if secured)

3. **Test Connection**
   - Click "Test Connection"
   - Wait for confirmation (shows assigned IP)

4. **Start Attack**
   - Click "Start Attack"
   - Watch the statistics update in real-time

5. **Monitor Results**
   - See addresses obtained count
   - View attack runtime
   - Try connecting another device to see the impact

6. **Stop Attack**
   - Click "Stop Attack" when done
   - Your network should recover after DHCP leases expire

## What to Expect

### First Run
- Device creates "Sinkhole" WiFi network
- Web interface loads immediately
- Network scan finds nearby WiFi

### During Attack
- New IP address every ~2 seconds
- Serial monitor shows each successful connection
- Target network's DHCP pool gradually depletes

### Success Indicators
- Counter shows increasing number of addresses
- New devices can't connect to target network
- DHCP pool becomes exhausted

## Troubleshooting

### Can't Connect to "Sinkhole"
- **Problem**: WiFi not showing up
- **Solution**: Check serial monitor for errors, try rebooting device

### Web Interface Won't Load
- **Problem**: Page not loading at 10.13.37.1
- **Solution**: Verify you're connected to "Sinkhole" WiFi

### Test Connection Fails
- **Problem**: Can't connect to target network
- **Solution**: Double-check WiFi password, ensure network is in range

### Attack Gets No Addresses
- **Problem**: Counter stays at 0
- **Solution**: Run "Test Connection" first, check serial monitor for errors

### Device Keeps Restarting
- **Problem**: Power brownouts
- **Solution**: Use better USB power supply or powered USB hub

## Next Steps

### Learn More
- Read the full [README.md](README.md) for detailed information
- Check [EXAMPLES.md](EXAMPLES.md) for usage scenarios
- Review [SECURITY.md](SECURITY.md) for legal guidelines

### Customize
- Modify AP name/password in `src/main.cpp`
- Adjust attack rate (default: 1 request/second)
- Change web interface styling

### Contribute
- Report bugs on GitHub
- Submit feature requests
- Contribute code improvements

## Safety Tips

⚠️ **Always remember:**

1. **Get Permission**: Only test networks you own or have authorization for
2. **Start Small**: Test on home network first to understand impact
3. **Monitor Impact**: Watch for unexpected consequences
4. **Have a Plan**: Know how to stop and restore normal operation
5. **Document Everything**: Keep records of your testing

## Common Scenarios

### Home Network Test (5 minutes)
```
1. Connect to Sinkhole
2. Scan and select your home WiFi
3. Test connection
4. Start attack
5. Try connecting a new device (should fail)
6. Stop attack
7. Wait for lease expiry or reboot router
```

### Lab Environment (10 minutes)
```
1. Set up isolated test network
2. Configure small DHCP pool (e.g., 10 addresses)
3. Run attack until pool exhausts
4. Document time to exhaustion
5. Test mitigation strategies
6. Verify attacks are blocked
```

## Performance Expectations

| Network Type | Expected Result |
|--------------|----------------|
| **Home Router** | Pool exhausted in 2-10 minutes |
| **Small Business** | Pool exhausted in 10-30 minutes |
| **Enterprise (unprotected)** | Pool exhausted in 30+ minutes |
| **Enterprise (protected)** | Attack blocked/rate limited |

## Serial Monitor Output

Expected output during operation:

```
=================================
DHCP Depleter - Starting up
=================================

Setting up Access Point...
Access Point started
AP IP address: 10.13.37.1
Setting up Web Server...
Web Server started

=================================
System Ready!
Connect to WiFi: Sinkhole
Password: 12345678
Web Interface: http://10.13.37.1
=================================

QR Code for WiFi Connection:
Scan this with your phone to connect:
WiFi QR Content: WIFI:T:WPA;S:Sinkhole;P:12345678;;

=================================
STARTING DHCP ATTACK
Target: MyHomeWiFi
=================================

New MAC: 02:A3:4F:12:B8:9C
Attempting to obtain new DHCP address...
✓ Success! Obtained IP: 192.168.1.100 (Total: 1)

New MAC: 02:71:E5:8D:3A:F2
Attempting to obtain new DHCP address...
✓ Success! Obtained IP: 192.168.1.101 (Total: 2)

[continues...]
```

## Web Interface Features

### Main Screen
- Clean, modern design
- Color-coded status indicators
- Real-time statistics
- Responsive layout (works on phone/tablet)

### Network List
- Shows SSID names
- Signal strength (RSSI in dBm)
- Security type (Open/Secured)
- Easy selection

### Statistics Dashboard
- **Addresses Obtained**: Total IPs allocated
- **Runtime**: How long attack has been running
- Updates every second during attack

## Advanced Usage

### Change Attack Rate
Edit in `src/main.cpp`:
```cpp
// In performDHCPAttack()
if (millis() - last_attack < 1000) {  // Change 1000 to desired ms
    return;
}
```

### Custom AP Configuration
Edit in `src/main.cpp`:
```cpp
const char* ap_ssid = "YourAPName";
const char* ap_password = "YourPassword";
const IPAddress ap_ip(10, 13, 37, 1);  // Change IP if needed
```

### Enable Debug Logging
Already enabled! Check serial monitor at 115200 baud.

## Resources

- **Documentation**: See README.md, BUILDING.md, EXAMPLES.md
- **Source Code**: src/main.cpp (well-commented)
- **Issue Tracker**: GitHub issues
- **Community**: GitHub discussions

## Support

### Getting Help
1. Check the documentation first
2. Review common issues in this guide
3. Check serial monitor for error messages
4. Open a GitHub issue with details

### Reporting Bugs
Include:
- ESP32-C6 board model
- PlatformIO version
- Serial monitor output
- Steps to reproduce
- Expected vs actual behavior

## Legal Reminder

🚨 **This tool is for authorized testing only!**

Using this tool without permission is likely illegal. Always:
- Get written authorization
- Test only networks you own
- Understand the legal implications
- Use responsibly

---

**You're now ready to test DHCP resilience! Happy (authorized) testing! 🎯**

For detailed information, continue to the main [README.md](README.md).
