# esp32_ota_mqtt

Over-the-Air (OTA) updates with ESP32 and a local MQTT server provide a seamless way to wirelessly upgrade ESP32 firmware.

## To start the Mosquitto Broker in Windows, you can follow these steps

1. **Install Mosquitto:**
If you haven't already installed Mosquitto, you can download the installer for Windows from the official Mosquitto website: [Mosquitto Downloads](https://mosquitto.org/download/).
   - Download the Windows installer that matches your system architecture (32-bit or 64-bit).
   - Run the installer and follow the installation wizard's instructions.

2. **Start the Mosquitto Broker:**
Once the installation is complete, you can start the Mosquitto Broker. You can do this way:
   - Using the Command Prompt:
      - Open the Command Prompt as an administrator (right-click the Command Prompt and choose `Run as administrator`).
      - Navigate to the Mosquitto installation directory (by default, it's installed in **C:\Program Files\Mosquitto**).
      - To enable remote access to the Mosquitto MQTT broker, we must modify the`mosquitto.conf` file.
      - Open `mosquitto.conf` file in text editor(by default, it's located in **C:\Program Files\Mosquitto\mosquitto.conf**)
      - To allow remote access, you need to specify the network interface to listen on. For example, to listen on all network interfaces (0.0.0.0), use the following:

      ```
      listener 1883 0.0.0.0
      ```

      - The `max_packet_size` setting should be 10000 or higher.
      - The `message_size_limit` should be set to 10000 or higher.
      - After adding the necessary configuration settings, save the file and exit the text editor.
      - Run the following command to start the Mosquitto broker in varbose mode:

      ```
        mosquitto -c mosquitto.conf -v
      ```

    The Mosquitto Broker should now be running and listening on the default MQTT port (**1883**).

3. **Verify the Mosquitto Broker is Running:**
You can verify that the Mosquitto Broker is running by opening a Command Prompt and using the mosquitto_sub command-line tool to subscribe to a test topic. Open a new Command Prompt window and run the following command:

```
mosquitto_sub -h localhost -t test/topic
```
If the Mosquitto Broker is running, you should see this command waiting for incoming messages on the `test/topic` MQTT topic.

You have now successfully started the Mosquitto Broker on your Windows machine. You can use it for MQTT-based messaging and IoT applications. Remember that you can install and use MQTT client libraries like Paho MQTT in Python to create MQTT clients that interact with the broker.


## Building and Uploading `esp32_ota_mqtt` Project with PlatformIO in Visual Studio Code

### Prerequisites

Before you begin, make sure you have the following:

- **Visual Studio Code**: If you haven't already, [download and install Visual Studio Code](https://code.visualstudio.com/).
- **PlatformIO Extension**: Open Visual Studio Code, go to the Extensions view by clicking on the square icon in the sidebar, search for `PlatformIO IDE`, and install it.

### Getting Started

1. **Open Your Project**: Open `esp32_ota_mqtt` project folder in Visual Studio Code. Open `platformio.ini` file and modify `upload_port` and `monitor_port` according to your board port, you can check your port number from `device manager`
2. **Change credentials** : 
   - Open `defines.h` file (it's located in **esp32_ota_mqtt/include/defines.h**)
   - Change `WIFI_SSID` with your WiFi name
   - Change `WIFI_PASS` with your WiFi password
   - You can find the IP address of your Windows computer where your Mosquitto Broker is running by using the cmd tool and the command `ipconfig`. Replace `MQTT_SERVER` with that address.
   - Change `MQTT_PORT` to your `mosquitto` server port (By default i will be `1883`)
   - Please make sure your your Windows computer connected with same WiFi
3. **Build**: Click on the `Build` icon to compile your code.
4. **Upload**: Click on the `Upload` icon to flash the compiled code onto your target board.
5. **Serial Monitor (Optional)**: Click on the `Serial Monitor` icon to open the serial monitor and interact with your board.
6. **Run Your Code**: Your code is now running on your embedded device!
