
import time
import threading

from flask import Flask, jsonify

from spotify_client import create_spotify_client, get_current_track


app = Flask(__name__)

POLL_INTERVAL = 1.0

playback_data = {
    "connected": False,
    "track": None,
    "error": None,
    "sampled_at": 0.0
}

state_lock = threading.Lock()


def spotify_polling_loop():

    print("Connecting to Spotify...")

    try:
        spotify = create_spotify_client()
        print("Spotify authentication successful.")

    except Exception as error:
        with state_lock:
            playback_data["error"] = str(error)

        print("Spotify connection failed:", error)
        return

    while True:

        cycle_start = time.monotonic()

        try:
            track = get_current_track(spotify)

            with state_lock:
                playback_data["connected"] = True
                playback_data["track"] = track
                playback_data["sampled_at"] = time.monotonic()
                playback_data["error"] = None

            if track:
                print(
                    f"Now playing: {track['title']} - "
                    f"{track['artist']}"
                )

        except Exception as error:

            with state_lock:
                playback_data["error"] = str(error)

            print("Spotify polling error:", error)

        elapsed = time.monotonic() - cycle_start
        time.sleep(max(0, POLL_INTERVAL - elapsed))


@app.route("/")
def home():

    return jsonify({
        "service": "Spotify Desk Partner",
        "status": "running"
    })


@app.route("/playback")
def get_playback():

    with state_lock:
        response = dict(playback_data)

    track = response["track"]

    if track and track["is_playing"]:
        elapsed = time.monotonic() - response["sampled_at"]

        track = dict(track)

        track["progress_ms"] += int(elapsed * 1000)

        track["progress_ms"] = min(
            track["progress_ms"],
            track["duration_ms"]
        )

    response["track"] = track

    return jsonify(response)


if __name__ == "__main__":

    polling_thread = threading.Thread(
        target=spotify_polling_loop,
        daemon=True
    )

    polling_thread.start()

    print("Spotify Desk Partner bridge starting...")
    print("Playback endpoint: http://127.0.0.1:5000/playback")

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False,
        use_reloader=False
    )
