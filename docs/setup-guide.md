# Setup and Installation Guide

This guide explains how to configure, build, and run Spotify Desk Partner on a new machine.

## 1. Prerequisites

### Hardware
- ESP32 NodeMCU development board.
- 1.3-inch SH1106 OLED display (128 × 64).
- Breadboard and jumper wires.
- USB cable compatible with the ESP32.

### Software
- Windows, Linux, or macOS computer.
- Python 3.x.
- Git.
- Visual Studio Code.
- PlatformIO IDE extension.
- A Spotify account.
- A Spotify Developer application.

The computer and ESP32 must be connected to the same 2.4 GHz Wi-Fi network.

## 2. Clone the Repository

Clone the project:

```bash
git clone https://github.com/Shaurya714/spotify-desk-partner.git
```

Navigate into the repository:

```bash
cd spotify-desk-partner
```

## 3. Configure Spotify Developer Access

The Python bridge uses Spotipy to communicate with Spotify.

### Create a Spotify Application

1. Open the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard).
2. Create a new application.
3. Copy the Client ID and Client Secret.
4. Configure the following redirect URI in the application settings:

   `http://127.0.0.1:8888/callback`

### Configure Environment Variables

Create a `.env` file inside the `bridge` directory.

```env
SPOTIPY_CLIENT_ID=your_client_id
SPOTIPY_CLIENT_SECRET=your_client_secret
SPOTIPY_REDIRECT_URI=http://127.0.0.1:8888/callback
```

The application uses the `user-read-currently-playing` permission to retrieve playback information.

Keep this file private. Never commit Spotify credentials to GitHub.

## 4. Set Up the Python Environment

From the repository root, create a virtual environment:

```bash
python -m venv .venv
```

Activate it on Windows PowerShell:

```powershell
.venv\Scripts\Activate.ps1
```

On macOS or Linux:

```bash
source .venv/bin/activate
```

Install the project dependencies:

```bash
pip install -r bridge/requirements.txt
```

## 5. Configure the ESP32

Create the following file:

`firmware/include/secrets.h`

Add your Wi-Fi credentials and the local address of the computer running the bridge:

```cpp
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"
#define SERVER_URL "http://YOUR_COMPUTER_IP:5000/playback"
```

Replace `YOUR_COMPUTER_IP` with the computer's local IPv4 address.

On Windows, find the address using:

```powershell
ipconfig
```

Look for the IPv4 address of the active Wi-Fi adapter.

For example:

`http://192.168.1.100:5000/playback`

The ESP32 and computer must be on the same local network.

**Security:** `secrets.h` contains private credentials and must never be committed to version control.

## 6. Start the Python Bridge

From the repository root, activate the virtual environment and run:

```bash
python bridge/server.py
```

The Flask application listens on port 5000.

If Spotify authorization is required, complete the OAuth process in the browser.

Keep the bridge running while using the OLED display.

### Verify the API

Open the following address in a browser on the same computer:

`http://127.0.0.1:5000/playback`

The endpoint should return a JSON response containing the connection state and playback information.

If the ESP32 cannot reach the bridge, check the computer's firewall settings and allow local network access to port 5000.

## 7. Build and Upload the ESP32 Firmware

1. Open the repository in Visual Studio Code.
2. Open the `firmware` directory as a PlatformIO project.
3. Connect the ESP32 through USB.
4. Build the firmware.
5. Upload it to the board.
6. Open the serial monitor at 115200 baud.

PlatformIO Core commands:

```bash
pio run
pio run --target upload
pio device monitor --baud 115200
```

If PlatformIO is installed through the VS Code extension, use its Build, Upload, and Serial Monitor controls.

## 8. Running the Complete System

Follow this sequence:

1. Connect the OLED to the ESP32 using the documented I²C wiring.
2. Connect the ESP32 to the computer using USB.
3. Start the Python virtual environment.
4. Run the Flask bridge.
5. Ensure Spotify is authenticated and playback information is available.
6. Upload and run the ESP32 firmware.
7. Observe the playback information on the OLED display.

The ESP32 periodically requests the latest playback information from the Python bridge.

## 9. API Reference

### Endpoint

`GET /playback`

The endpoint returns the current bridge connection state and the latest available track information.

The response includes:

| Field | Description |
|---|---|
| `connected` | Spotify connection state |
| `track.title` | Track title |
| `track.artist` | Artist name |
| `track.duration_ms` | Total track duration in milliseconds |
| `track.progress_ms` | Current playback position in milliseconds |
| `track.is_playing` | Whether playback is active |

When no track is available, the `track` field may be null.

## 10. Troubleshooting

### Spotify authentication fails

- Verify the Client ID and Client Secret.
- Confirm that the redirect URI matches the Spotify Developer Dashboard configuration.
- Check the OAuth permissions.
- Ensure the Spotify account can access playback information.

### Python bridge does not start

- Activate the correct virtual environment.
- Install dependencies using `requirements.txt`.
- Check whether port 5000 is already in use.
- Inspect the terminal output for Python errors.

### ESP32 cannot connect to the bridge

- Verify the IP address in `secrets.h`.
- Confirm that the computer and ESP32 share the same Wi-Fi network.
- Use a 2.4 GHz network.
- Check firewall permissions for port 5000.
- Verify that the Flask server is running.

### OLED does not display playback information

- Verify the I²C wiring.
- Confirm that the SH1106 display driver is configured.
- Check the serial monitor for connection and HTTP errors.
- Verify that the bridge endpoint returns valid JSON.

## 11. Security and Configuration Notes

The following files should remain local and must not be committed:

- `bridge/.env`
- `firmware/include/secrets.h`
- Spotify authentication cache files.
- Python virtual environment directories.
- PlatformIO generated build directories.

The repository's `.gitignore` is configured to exclude sensitive credentials and generated files.

## 12. Current Implementation Scope

The current version provides a read-only Spotify playback display. It retrieves track metadata and playback progress but does not send playback-control commands to Spotify.

Playback information depends on Spotify API availability and a running Python bridge.