# Hardware and Wiring

## 1. Hardware Overview

Spotify Desk Partner uses an ESP32 development board connected to a monochrome OLED display. The ESP32 handles wireless communication and display rendering, while the OLED provides a compact interface for Spotify playback information.

## 2. Components

| Component | Specification | Purpose |
|---|---|---|
| ESP32 NodeMCU | ESP32 development board | Main controller and Wi-Fi communication |
| OLED Display | 1.3-inch SH1106, 128 × 64 pixels | Displays playback information |
| Breadboard | 840-point breadboard | Circuit prototyping |
| Jumper Wires | Male-to-male jumper wires | Electrical connections |
| USB Cable | Compatible with ESP32 board | Power and firmware upload |

## 3. OLED Display Specifications

The display used in this project is a 1.3-inch monochrome OLED based on the SH1106 display controller.

Key specifications:

- Resolution: 128 × 64 pixels.
- Display controller: SH1106.
- Communication interface: I²C.
- Supply voltage: 3.3V.
- Display color: Blue monochrome.
- Four-pin interface: VCC, GND, SCL and SDA.

The firmware uses the U8g2 graphics library with the following display constructor:

```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE
);
```

This configures a full-frame-buffer SH1106 display using hardware I²C.

## 4. Wiring Connections

The OLED connects directly to the ESP32 using its I²C interface.

| OLED Pin | ESP32 Pin | Description |
|---|---|---|
| VCC | 3V3 | Power supply |
| GND | GND | Ground |
| SCL | GPIO 22 | I²C clock |
| SDA | GPIO 21 | I²C data |

### Connection Diagram

```text
       ESP32 NodeMCU             SH1106 OLED
      +-------------+           +-----------+
      |             |           |           |
      | 3V3         |---------->| VCC       |
      | GND         |---------->| GND       |
      | GPIO 22     |---------->| SCL       |
      | GPIO 21     |---------->| SDA       |
      |             |           |           |
      +-------------+           +-----------+
```

### Wiring Notes

- Connect VCC to the ESP32's 3.3V output.
- Ensure the ESP32 and OLED share a common ground.
- Connect SCL to GPIO 22 and SDA to GPIO 21.
- Verify the pin labels on the actual OLED module before powering the circuit.
- Avoid loose breadboard connections, particularly on the I²C lines.

## 5. Hardware Assembly

1. Place the ESP32 development board on the breadboard.
2. Connect the OLED VCC and GND pins.
3. Connect the I²C clock and data lines.
4. Connect the ESP32 to the computer through USB.
5. Upload the firmware using PlatformIO.
6. Verify that the OLED initializes and displays the startup screen.

## 6. Firmware and Hardware Integration

The firmware uses three main libraries:

| Library | Responsibility |
|---|---|
| U8g2 | OLED initialization and graphics rendering |
| ArduinoJson | Parsing playback data received from the bridge |
| HTTPClient | Sending HTTP requests to the Python bridge |

The ESP32 uses Wi-Fi to retrieve playback information and I²C to communicate with the OLED.

## 7. Troubleshooting

### Display remains blank

- Verify VCC and GND connections.
- Check the SDA and SCL wiring.
- Confirm that the firmware uses the SH1106 display constructor.
- Check the serial monitor for initialization or runtime issues.

### Display shows incorrect characters

- Verify the selected display controller.
- Check I²C connections.
- Ensure the firmware is using the correct U8g2 configuration.

### ESP32 fails to connect to Wi-Fi

- Verify the SSID and password in the local secrets file.
- Ensure the network supports 2.4 GHz Wi-Fi.
- Confirm that the ESP32 is within network range.

## 8. Future Hardware Improvements

Potential improvements include:

- Designing a compact enclosure.
- Creating a more permanent PCB-based circuit.
- Improving cable management.
- Exploring a standalone desktop form factor.