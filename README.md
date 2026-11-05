# Spotify Desk Partner

A compact ESP32-powered desktop display that brings your Spotify playback information onto a dedicated OLED screen.

Spotify Desk Partner uses a Python bridge to communicate with the Spotify Web API and an ESP32 microcontroller to display the currently playing track, artist, playback progress, and playback state on a 128×64 OLED display.

## Features

- Displays the currently playing track and artist.
- Shows playback progress with a graphical progress bar.
- Displays elapsed and total playback time.
- Indicates whether playback is active or paused.
- Uses a Python-based Spotify API bridge.
- Communicates wirelessly between the computer and ESP32 over Wi-Fi.
- Real time clock display 

## System Architecture

```text
Spotify Web API
      |
      v
Python Bridge (Spotipy)
      |
      v
Flask HTTP Server
      |
      | Wi-Fi / HTTP JSON
      v
ESP32 NodeMCU
      |
      v
SH1106 OLED Display
```

The Python bridge handles Spotify authentication and retrieves playback information. It exposes the latest playback data through a local HTTP endpoint. The ESP32 periodically requests this data and renders it on the OLED display.

## Hardware

| Component | Purpose |
|---|---|
| ESP32 NodeMCU | Main microcontroller |
| 1.3-inch SH1106 OLED | Playback information display |
| Breadboard and jumper wires | Circuit connections |
| USB cable | Programming and power |

### OLED Wiring

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

The display communicates with the ESP32 using I²C.

## Software Stack

**Firmware**
- C++
- Arduino framework
- PlatformIO
- U8g2 graphics library
- ArduinoJson

**Backend**
- Python
- Flask
- Spotipy
- python-dotenv

## Project Structure

```text
SpotifyDeskPartner/
├── bridge/
│   ├── spotify_client.py
│   ├── server.py
│   └── requirements.txt
│
├── firmware/
│   ├── include/
│   │   └── secrets.h
│   ├── src/
│   │   └── main.cpp
│   └── platformio.ini
│
├── docs/
├── .gitignore
└── README.md
```

Local credentials, authentication caches, virtual environments, and generated build files are excluded from version control.

## Setup and Installation

### 1. Requirements

- Python 3.x
- PlatformIO IDE or PlatformIO Core
- ESP32 development board
- Spotify account
- Spotify Developer application
- Computer and ESP32 connected to the same 2.4 GHz Wi-Fi network

### 2. Configure Spotify API Access

1. Create an application through the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard).
2. Obtain its Client ID and Client Secret.
3. Configure this redirect URI in the application settings:

   `http://127.0.0.1:8888/callback`

4. Create a `bridge/.env` file:

```env
SPOTIPY_CLIENT_ID=your_client_id
SPOTIPY_CLIENT_SECRET=your_client_secret
SPOTIPY_REDIRECT_URI=http://127.0.0.1:8888/callback
```

The bridge uses the `user-read-currently-playing` permission to retrieve playback information.

### 3. Set Up the Python Bridge

From the repository root:

```powershell
python -m venv .venv
```

Activate the environment:

```powershell
.venv\Scripts\Activate.ps1
```

Install dependencies:

```powershell
pip install -r bridge/requirements.txt
```

Start the bridge:

```powershell
python bridge/server.py
```

Complete Spotify authorization in the browser when prompted.

### 4. Configure ESP32 Credentials

Create `firmware/include/secrets.h`:

```cpp
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"
#define SERVER_URL "http://YOUR_COMPUTER_IP:5000/playback"
```

Replace `YOUR_COMPUTER_IP` with the local IP address of the computer running the Python bridge.

**Do not commit `secrets.h` or `.env` to GitHub.**

### 5. Build and Upload Firmware

Open the `firmware` directory in PlatformIO.

Build the firmware, connect the ESP32, and upload it to the board.

The serial monitor runs at 115200 baud.

## Playback API

The Python bridge exposes:

`GET /playback`

Example response:

```json
{
  "connected": true,
  "track": {
    "title": "Example Track",
    "artist": "Example Artist",
    "duration_ms": 193000,
    "progress_ms": 45000,
    "is_playing": true
  }
}
```

The ESP32 uses these fields to render the playback interface.

## Current Limitations

- The project currently displays playback information; it does not control Spotify playback.
- Spotify playback information requires an active Spotify session and valid API authorization.
- The computer running the bridge and the ESP32 must be connected to the same network.
- Displayed duration and progress are based on Spotify Web API metadata.

## Future Improvements

- Display playlist or album information.
- Improve network reconnection and error handling.
- Add richer playback controls.
- Improve text scrolling for longer track titles and artist names.
- Explore a more polished enclosure and desktop form factor.

## Project Status

The core Spotify-to-ESP32 playback display integration is implemented. Documentation, hardware refinement, and further reliability improvements remain part of the development roadmap.

---

Built as an exploration of embedded systems, API integration, wireless communication, and hardware-software interaction.
