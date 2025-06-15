# ESP32_MQTT433MHzBridge: Environment Setup

This guide will help you set up your development environment for the ESP32_MQTT433MHzBridge project using PlatformIO and Visual Studio Code on Linux.

---

## 1. Prerequisites

- **Hardware:** ESP32 development board
- **Operating System:** Linux (tested on Ubuntu/Debian)
- **Software:**
  - [Visual Studio Code](https://code.visualstudio.com/)
  - [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
  - Python 3.x (for PlatformIO and OTA tools)
  - `git` (for cloning the repository)

---

## 2. Install Visual Studio Code

```sh
sudo snap install --classic code
```
Or download from [here](https://code.visualstudio.com/).

---

## 3. Install PlatformIO Extension

1. Open Visual Studio Code.
2. Go to the Extensions view (`Ctrl+Shift+X`).
3. Search for **PlatformIO IDE** and click **Install**.

---

## 4. Clone the Repository

```sh
git clone https://github.com/yourusername/ESP32_MQTT433MHzBridge.git
cd ESP32_MQTT433MHzBridge
```

---

## 5. Install Python 3 (if not already installed)

```sh
sudo apt update
sudo apt install python3 python3-pip
```

---

## 6. Open the Project in VS Code

1. Launch Visual Studio Code.
2. Open the project folder (`File` > `Open Folder...`).

---

## 7. Build and Upload Firmware

1. Connect your ESP32 board via USB.
2. In VS Code, open the PlatformIO sidebar (alien icon).
3. Click **Build** (checkmark icon) to compile the project.
4. Click **Upload** (right arrow icon) to flash the firmware.

**Note:** If you encounter permission errors, add your user to the `dialout` group:
```sh
sudo usermod -a -G dialout $USER
# Log out and log back in for changes to take effect.
```

---

## 8. OTA (Over-the-Air) Updates

- Edit `platformio.ini` to set your network parameters if needed.
- Use the `espota.py` script in the `tools/` directory for OTA updates:
  ```sh
  python3 tools/espota.py -i <ESP32_IP> -p 3232 --auth=<password> -f .pio/build/esp32dev/firmware.bin
  ```
- Replace `<ESP32_IP>` and `<password>` with your device's IP and OTA password.

---

## 9. Troubleshooting

- **Serial Port Not Found:**  
  Ensure your user is in the `dialout` group and the device is connected.
- **Build Fails:**  
  Make sure all dependencies are installed. Run `pio update` and `pio upgrade` in the terminal.
- **OTA Fails:**  
  Check that the ESP32 is on the same network and OTA is enabled in the firmware.
- **Permission Denied:**  
  Use `sudo` only if necessary. Prefer fixing group permissions.

---

## 10. Additional Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Project README.md](../README.md)

---

**If you encounter issues not covered here, please open an issue on the project GitHub