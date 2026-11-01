# System Architecture

## 1. Overview

Spotify Desk Partner is an embedded desktop companion that displays Spotify playback information on a dedicated OLED screen.

The system combines a Python application running on a computer with an ESP32 microcontroller. The computer communicates with Spotify through the Spotify Web API, processes the playback information, and exposes it through a local HTTP endpoint. The ESP32 retrieves this information over Wi-Fi and renders it on the OLED display.

The architecture separates API communication from hardware-level display operations.

## 2. Architecture Diagram

```text
+----------------------+
|      Spotify         |
|      Web API         |
+----------+-----------+
           |
           | Spotipy / OAuth
           v
+----------------------+
|   Spotify Client     |
|   spotify_client.py  |
+----------+-----------+
           |
           v
+----------------------+
|    Flask Bridge      |
|      server.py       |
|                      |
| Playback polling     |
| JSON response        |
+----------+-----------+
           |
           | HTTP over Wi-Fi
           | GET /playback
           v
+----------------------+
|     ESP32 NodeMCU    |
|                      |
| Wi-Fi connection     |
| HTTP client          |
| JSON parsing         |
+----------+-----------+
           |
           | I2C
           v
+----------------------+
|     SH1106 OLED      |
|       128 x 64       |
+----------------------+
```

## 3. Major Components

### 3.1 Spotify Web API

The Spotify Web API provides information about the user's currently playing track.

The application uses the `user-read-currently-playing` OAuth permission to retrieve playback information.

Relevant information includes:

- Track title
- Artist name
- Track duration
- Current playback position
- Playback state

### 3.2 Spotify Client

File: `bridge/spotify_client.py`

The Spotify client is responsible for authentication and communication with Spotify.

It uses Spotipy to handle OAuth authentication and API requests. Credentials are loaded from a local environment file rather than being embedded in source code.

### 3.3 Python Bridge

File: `bridge/server.py`

The Flask bridge acts as the communication layer between the Spotify client and ESP32.

Its responsibilities include:

- Polling Spotify playback information.
- Maintaining the latest playback state.
- Providing a local HTTP endpoint.
- Returning playback information in JSON format.
- Updating playback progress between Spotify API polls.

The bridge listens on port 5000 and exposes:

`GET /playback`

### 3.4 ESP32 Firmware

File: `firmware/src/main.cpp`

The ESP32 firmware handles the hardware side of the project.

Its responsibilities include:

- Connecting to the configured Wi-Fi network.
- Sending HTTP requests to the Python bridge.
- Parsing JSON responses using ArduinoJson.
- Maintaining the latest received playback information.
- Rendering the playback interface using U8g2.

The ESP32 does not independently extrapolate playback progress. The Python bridge provides the progress value.

### 3.5 OLED Display

The project uses a 1.3-inch SH1106 OLED display with a resolution of 128 × 64 pixels.

The display presents:

- Application header and connection indicator.
- Track title.
- Artist name.
- Playback progress bar.
- Elapsed and total playback time.
- Play or pause status.

## 4. Data Flow

1. The Python Spotify client authenticates with Spotify.
2. The bridge periodically requests currently playing information.
3. The latest playback information is stored by the Flask application.
4. The ESP32 sends an HTTP GET request to `/playback`.
5. The bridge returns a JSON response.
6. ArduinoJson parses the response.
7. The ESP32 updates its playback state.
8. U8g2 renders the updated information on the OLED.

## 5. Communication

| Connection | Protocol |
|---|---|
| Python client → Spotify | HTTPS / Spotify Web API |
| ESP32 → Python bridge | HTTP over local Wi-Fi |
| ESP32 → OLED | I²C |

The computer and ESP32 must be connected to the same local network.

## 6. Design Considerations

### Separation of Responsibilities

Spotify authentication and API communication remain on the computer. The ESP32 is responsible for network communication and display rendering.

This avoids implementing Spotify OAuth directly on the microcontroller.

### Centralized Playback Timing

The Python bridge is the single source of truth for playback progress. It updates progress using its most recent Spotify API sample and elapsed monotonic time.

This keeps the ESP32 firmware simpler and avoids maintaining a separate playback clock.

### Local Network Communication

The bridge is hosted locally, allowing the ESP32 to retrieve information without requiring a separate cloud backend.

## 7. Current Scope

The current implementation focuses on displaying Spotify playback information.

Playback control, playlist identification, and additional metadata are potential future improvements.