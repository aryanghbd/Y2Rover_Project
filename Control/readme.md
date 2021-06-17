# How to set up the Control subsystem.


1) Configure ESP32 for use with your system:
	-Before doing anything, ensure that you have the UART to USB bridge driver installed on your system.
	the driver can be downloaded from this link: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
	-Next, go into File/Preferences -> Additional Boards Managers URLs, and paste the link in the text field:
	https://dl.espressif.com/dl/package_esp32_index.json

2) The following libraries are required in Arduino IDE in order for this code to execute:
	-PubSubClient (Acquired in Sketch/Include Library/Manage Libraries + Search for 'PubSubClient')
	-ESP32 (Acquired in Tools/Boards/Boards Manager + Search for 'esp32')

3) Ensure that the ESP32 Dev Board is selected in Tools/Board/ESP32->ESP32 Dev Kit

4) Ensure the UART port is selected in Tools/Port.

5) In the serial monitor, ensure the Baud Rate is set to 115200, replace the SSID, Password, and MQTT based variables at the top with your local network parameters and MQTT Command terminal or other Broker parameters respectively

6) Compile and run the code, it should run on its own in the serial monitor, issue commands from the broker remotely.

