# Screenshots and UI Guide

This document provides a visual guide to the DHCP Depleter web interface.

## Main Interface

The web interface is accessible at **http://10.13.37.1** when connected to the "Sinkhole" WiFi network.

### Interface Sections

#### 1. Header
```
┌─────────────────────────────────────────┐
│  🔥 DHCP Depleter                       │
│  Wireless DHCP IP Depletion Tester     │
└─────────────────────────────────────────┘
```
- **Title**: Shows the application name
- **Subtitle**: Brief description of purpose

#### 2. Network Scanner Section
```
┌─────────────────────────────────────────┐
│  1. Scan for Networks                   │
│  ┌─────────────────────────────────┐   │
│  │  Scan WiFi Networks              │   │
│  └─────────────────────────────────┘   │
│                                         │
│  Found networks:                        │
│  • HomeWiFi (-45 dBm) Secured          │
│  • Office_Guest (-67 dBm) Open         │
│  • MyNetwork (-52 dBm) Secured         │
└─────────────────────────────────────────┘
```

**Features:**
- Blue "Scan" button
- Results show in real-time
- Displays SSID, signal strength (RSSI), and security type

#### 3. Target Configuration Section
```
┌─────────────────────────────────────────┐
│  2. Target Configuration                │
│                                         │
│  Target SSID:                           │
│  ┌─────────────────────────────────┐   │
│  │ Select a network...        ▼   │   │
│  └─────────────────────────────────┘   │
│                                         │
│  Password (if required):                │
│  ┌─────────────────────────────────┐   │
│  │ ••••••••••••••••               │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

**Features:**
- Dropdown populated from scan results
- Password field (masked input)
- Auto-populated with scanned networks

#### 4. Test & Attack Section
```
┌─────────────────────────────────────────┐
│  3. Test & Attack                       │
│  ┌─────────────────────────────────┐   │
│  │  Test Connection                 │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  Start Attack                    │   │
│  └─────────────────────────────────┘   │
│                                         │
│  ✓ Connection successful!               │
│  IP: 192.168.1.100                      │
└─────────────────────────────────────────┘
```

**Features:**
- Blue "Test Connection" button
- Green "Start Attack" button (enabled after successful test)
- Red "Stop Attack" button (shown during attack)
- Status messages with color coding

#### 5. Statistics Section (During Attack)
```
┌─────────────────────────────────────────┐
│  Attack Statistics                      │
│  ┌─────────────┐    ┌─────────────┐    │
│  │     42      │    │   2m 15s    │    │
│  │  Addresses  │    │   Runtime   │    │
│  │  Obtained   │    │             │    │
│  └─────────────┘    └─────────────┘    │
└─────────────────────────────────────────┘
```

**Features:**
- Real-time counter updates
- Runtime display (auto-formatted)
- Only visible during active attack

## Color Scheme

### Status Messages
- **Green** (Success): Connection successful, test passed
- **Orange** (Warning): Testing in progress, attack stopped
- **Red** (Error): Connection failed, errors occurred

### Buttons
- **Blue** (Primary): Default action buttons
- **Green** (Success): Start attack (when ready)
- **Red** (Danger): Stop attack

## Responsive Design

The interface adapts to different screen sizes:

### Desktop/Tablet (600px+)
- Wide layout
- Full statistics display
- All features visible

### Mobile (< 600px)
- Stacked layout
- Touch-friendly buttons
- Readable text sizes

## User Flow

### Step-by-Step Visual Flow

```
    ┌─────────────────┐
    │  Connect to     │
    │  "Sinkhole"     │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Open Browser   │
    │  10.13.37.1     │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Click "Scan"   │
    │  Wait for list  │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Select Target  │
    │  Enter Password │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Click "Test"   │
    │  Verify Success │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Click "Start"  │
    │  Monitor Stats  │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Click "Stop"   │
    │  When Done      │
    └─────────────────┘
```

## Status Indicators

### Before Test
- Start button: **Disabled** (gray)
- Status: Empty

### During Test
- Test button: Shows "Testing..."
- Status: Orange box "Testing connection..."

### After Successful Test
- Start button: **Enabled** (green)
- Status: Green box "✓ Connection successful! IP: x.x.x.x"

### After Failed Test
- Start button: **Disabled** (gray)
- Status: Red box "✗ Connection failed: [reason]"

### During Attack
- Start button: Hidden
- Stop button: Visible (red)
- Statistics: Updating every second
- Status: "Attack is running..."

### After Stopping
- Start button: Visible (green)
- Stop button: Hidden
- Status: Orange box "Attack stopped"

## Browser Compatibility

Tested and working on:
- ✅ Chrome/Edge (Desktop & Mobile)
- ✅ Firefox (Desktop & Mobile)
- ✅ Safari (Desktop & Mobile)
- ✅ Opera
- ✅ Samsung Internet

Minimum requirements:
- JavaScript enabled
- CSS3 support
- Modern browser (last 2 years)

## Accessibility Features

- **Semantic HTML**: Proper heading hierarchy
- **Color Contrast**: WCAG AA compliant
- **Button Labels**: Clear, descriptive text
- **Responsive Text**: Scales appropriately
- **Touch Targets**: 44px minimum for mobile

## Performance

- **Load Time**: < 1 second
- **API Calls**: Async, non-blocking
- **Updates**: 1 second interval during attack
- **Size**: ~15KB HTML (uncompressed)

## Tips for Best Experience

### Desktop
- Use Chrome or Firefox for best performance
- Keep window open during attack
- Monitor serial console for detailed logs

### Mobile
- Portrait mode recommended
- Use WiFi instead of cellular data
- Keep screen on during monitoring

### General
- Don't refresh page during attack
- Wait for test to complete before starting attack
- Allow network scan to finish before selecting

## Common Visual Issues

### Interface Not Loading
**Symptoms**: Blank page, loading forever
**Solution**: 
- Verify connected to "Sinkhole"
- Try http://10.13.37.1 (not https)
- Clear browser cache

### Buttons Not Working
**Symptoms**: Clicking does nothing
**Solution**:
- Check JavaScript is enabled
- Try different browser
- Check serial console for errors

### Statistics Not Updating
**Symptoms**: Counter stays at same value
**Solution**:
- Attack might have failed
- Check "Stop" then "Start" again
- Monitor serial console

## Developer Notes

### Customization

To customize the interface, edit `src/main.cpp`:

```cpp
// Find the handleRoot() function
// Modify HTML in the R"rawliteral(...)rawliteral" block

// Change colors
.container { background: white; }  // Main background
button { background: #667eea; }     // Button color
```

### Adding New Features

To add interface elements:
1. Add HTML in `handleRoot()`
2. Add JavaScript handlers
3. Create new API endpoint
4. Add backend function

### Debugging

Enable browser developer tools:
- **Console**: Check for JavaScript errors
- **Network**: Monitor API calls
- **Elements**: Inspect HTML/CSS

## Future UI Enhancements

Planned improvements:
- [ ] Dark mode toggle
- [ ] Advanced settings panel
- [ ] Export statistics as CSV
- [ ] Connection history log
- [ ] Visual signal strength meter
- [ ] Attack pattern selection
- [ ] Multi-language support
- [ ] Custom themes

---

For more information, see:
- [README.md](README.md) - Project overview
- [QUICKSTART.md](QUICKSTART.md) - Getting started
- [EXAMPLES.md](EXAMPLES.md) - Usage examples
