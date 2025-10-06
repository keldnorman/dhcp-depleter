# dhcp-depleter
Wireless DHCP IP Depletion Tester

This little C++program + an esp32-c6 hardware will setup a Wireless accesspoint that you connect to and select what SSID (wireless accesspoint name) you want to test a DHCP starvation attack against. 

You can setup and start the test by connected to the WiFi that the device will create called Sinkhole.
The fastest way to connect to it is to scan the QR code or you could alternatively connect to it manually using the password 12345678.

When connected to the setup WiFi and visiting the webpage http://10.13.37.1 you can scan for surrounding WiFi's and select a target. If the SSID requires a WPA2/WPA3 passphrase you can supply it at the webpage.

Then just press "Test" to see if the esp32-c6 can connect and pull an IP address from the DHCP server.
If it succeed you can start the test that will continue forever (or until it runs out of battery) to retrieve new IP addresses. 
