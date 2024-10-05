# HomeControl - ESP32 & ESP32CAM

This repository contains a project that enables an ESP32 device to receive infrared (IR) signals, which are then transmitted to an ESP32CAM. The ESP32CAM captures the data and sends it to a server for storage and further processing.

## :dart: About ##

**HomeControl** is designed to allow remote control of various home appliances using IR signals. The project utilizes ESP32 devices to receive, process, and transmit IR signals, providing a seamless way to control devices via a web interface or mobile app.

## :sparkles: Features ##

- **IR Signal Reception**: Capture IR signals from remote controls using the ESP32.
- **Data Transmission**: Send captured signals from the ESP32 to the ESP32CAM.
- **Server Communication**: The ESP32CAM sends the captured data to a designated server for storage.
- **Remote Control Capability**: Enables control of home appliances through the captured IR signals.

## :rocket: Technologies ##

The following technologies were used in this project:

- [ESP32](https://www.esp32.com/)
- [Arduino](https://www.arduino.cc/)
- [ESP32CAM](https://randomnerdtutorials.com/getting-started-with-esp32-cam/)
- [IRremote Library](https://github.com/Arduino-IRremote/Arduino-IRremote)

## :white_check_mark: Requirements ##

To run this project, you'll need:

- [Arduino IDE](https://www.arduino.cc/en/software) installed on your machine.
- An ESP32 and ESP32CAM module.
- Basic understanding of Arduino programming.

## :checkered_flag: Running the Project ##

1. Clone the repository to your local machine:

    ```bash
    $ git clone https://github.com/Piiii31/HomeControl-ESP32-ESP32CAM.git
    ```

2. Navigate to the project directory:

    ```bash
    $ cd HomeControl-ESP32-ESP32CAM
    ```

3. Open the project in the Arduino IDE.

4. Connect your ESP32 to your computer and select the appropriate board and port in the IDE.

5. Upload the code to the ESP32 and ESP32CAM modules.

6. Ensure both devices are connected to the same Wi-Fi network.

7. Monitor the serial output for debugging information and confirm the data is being sent to the server.



## :handshake: Contributions ##

Contributions are welcome! Feel free to open an issue or submit a pull request.

## :mailbox_with_mail: Contact ##

For any inquiries, feel free to reach out: [Piiii31](mailto:Piiii31)

---

<p align="center">
  Made with :heart: by <a href="https://github.com/Piiii31" target="_blank">Piiii31</a>
</p>
