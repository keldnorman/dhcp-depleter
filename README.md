# dhcp-depleter
Wireless DHCP IP Depletion Tester

![Parts](https://github.com/keldnorman/dhcp-depleter/blob/main/images/parts.jpg?raw=true)

This C++ program, paired with an ESP32-C6, sets up a wireless access point you can join to run a DHCP-starvation test against a chosen SSID.
Connect to the device’s Wi-Fi named “Sinkhole” with the passphrase 12345678 ( or scan the QR code with your phone).

![Parts](https://github.com/keldnorman/dhcp-depleter/blob/main/images/qr.png?raw=true)

After connecting, the config page should open automatically but if not then open http://10.13.37.1
Press the button "Scan Wi-Fi" for nearby accesspoints/SSID's/wireless networks and select a target.

![Scan](https://github.com/keldnorman/dhcp-depleter/blob/main/images/scan.png?raw=true)

If the wireless accesspoint requires a WPA2/WPA3 passphrase you can enter it on the webpage.
Then press “Test connection” to verify that the ESP32-C6 can connect and obtain an IP address via DHCP.

![Select](https://github.com/keldnorman/dhcp-depleter/blob/main/images/select.png?raw=true)

If it succeeds, you can start the test by pressing "Start".

![Start](https://github.com/keldnorman/dhcp-depleter/blob/main/images/start.png?raw=true)

The starvation attack will keep running forever requesting new IP addresses ( until turned off or the battery is depleted).

How to setup Arduino ?

Download Arduino IDE here: https://www.arduino.cc/en/software/ [arduino](https://www.arduino.cc/en/software/)

Watch the video for how to ensure the esp32c6 can be selected as board to work with: 

[![Setup Arduino](https://raw.githubusercontent.com/keldnorman/dhcp-depleter/main/images/setup-arduino.png)](https://raw.githubusercontent.com/keldnorman/dhcp-depleter/main/film/setup-arduino.mp4)
