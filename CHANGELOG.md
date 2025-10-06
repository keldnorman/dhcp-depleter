# Changelog

All notable changes to the DHCP Depleter project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01-06

### Added
- Initial release of DHCP Depleter for ESP32-C6
- WiFi Access Point mode with "Sinkhole" SSID
- Web-based interface for device configuration
- Network scanning functionality to discover target WiFi networks
- Connection testing before starting attack
- DHCP address exhaustion attack with MAC address randomization
- Real-time statistics and monitoring
- QR code information for easy WiFi connection
- Comprehensive documentation:
  - README.md - Main project documentation
  - BUILDING.md - Build instructions
  - QUICKSTART.md - Quick start guide
  - EXAMPLES.md - Usage scenarios and examples
  - SECURITY.md - Legal and ethical guidelines
  - ARCHITECTURE.md - System architecture documentation
  - LICENSE - MIT license with legal disclaimer
- PlatformIO project structure
- Support for WPA2/WPA3 secured networks
- JSON API for web interface communication
- Serial console logging for debugging
- Dual WiFi mode support (AP + STA simultaneously)

### Features
- Automatic MAC address randomization for each DHCP request
- Rate-limited attack (1 request per second) to prevent device overload
- Clean, modern web interface with responsive design
- Real-time attack statistics (addresses obtained, runtime)
- Start/Stop controls for attack management
- Target network validation before attack
- Comprehensive error handling and user feedback

### Technical Details
- Platform: ESP32-C6 (RISC-V based)
- Framework: Arduino
- Libraries: ArduinoJson 7.x, ESP32 WiFi
- Memory: Optimized for ESP32-C6 resources
- Network: Dual-mode WiFi (AP + STA)
- Security: Locally administered MAC addresses

### Documentation
- Quick start guide for beginners
- Detailed usage examples for various scenarios
- Legal and ethical guidelines
- Security best practices
- Build instructions for multiple platforms
- Architecture documentation with diagrams

## [Unreleased]

### Planned Features
- Over-The-Air (OTA) firmware updates
- Statistics logging to SD card
- Multiple target support
- Scheduled attacks
- OLED display for status information
- Battery level monitoring
- Custom attack patterns configuration
- Advanced rate limiting controls via UI
- Export attack logs
- Network discovery improvements

### Potential Improvements
- Faster attack rate options
- Web interface themes
- Mobile app companion
- Enhanced MAC randomization algorithms
- Captive portal for easier setup
- Bluetooth configuration option
- WPA Enterprise support investigation

---

## Version History

### Version Numbering
- **Major (X.0.0)**: Breaking changes, major feature additions
- **Minor (0.X.0)**: New features, backward compatible
- **Patch (0.0.X)**: Bug fixes, minor improvements

### Release Notes Format
- **Added**: New features
- **Changed**: Changes to existing functionality
- **Deprecated**: Features marked for removal
- **Removed**: Removed features
- **Fixed**: Bug fixes
- **Security**: Security improvements

---

## Contributing

When submitting changes:
1. Update this CHANGELOG.md with your changes
2. Follow the format above
3. Add entries under [Unreleased] section
4. Entries will be moved to a version section on release

## Support

For issues or questions:
- Open an issue on GitHub
- Check existing documentation
- Review closed issues for solutions

## License

See [LICENSE](LICENSE) file for details.

---

**Note**: This is the initial release. Future versions will build upon this foundation with additional features and improvements based on community feedback and use cases.
