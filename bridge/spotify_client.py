
import os

import spotipy
from dotenv import load_dotenv
from spotipy.oauth2 import SpotifyOAuth


# ==========================================
# SPOTIFY DESK PARTNER
# Spotify API Client
# ==========================================

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

load_dotenv(os.path.join(BASE_DIR, ".env"))

CLIENT_ID = os.getenv("SPOTIPY_CLIENT_ID")
CLIENT_SECRET = os.getenv("SPOTIPY_CLIENT_SECRET")
REDIRECT_URI = os.getenv("SPOTIPY_REDIRECT_URI")

SCOPE = "user-read-currently-playing"


def create_spotify_client():

    if not CLIENT_ID or not CLIENT_SECRET or not REDIRECT_URI:
        raise RuntimeError(
            "Spotify credentials are missing from bridge/.env"
        )

    auth_manager = SpotifyOAuth(
        client_id=CLIENT_ID,
        client_secret=CLIENT_SECRET,
        redirect_uri=REDIRECT_URI,
        scope=SCOPE,
        open_browser=True,
        cache_path=os.path.join(BASE_DIR, ".spotify_cache"),
    )

    return spotipy.Spotify(auth_manager=auth_manager)


def get_current_track(sp):

    playback = sp.current_user_playing_track()

    if not playback or not playback.get("item"):
        return None

    track = playback["item"]

    artists = ", ".join(
        artist["name"] for artist in track["artists"]
    )

    return {
        "title": track["name"],
        "artist": artists,
        "duration_ms": track["duration_ms"],
        "progress_ms": playback.get("progress_ms", 0),
        "is_playing": playback.get("is_playing", False),
        "track_id": track["id"],
    }


if __name__ == "__main__":

    spotify = create_spotify_client()

    print("\nSpotify Desk Partner")
    print("--------------------")

    current_track = get_current_track(spotify)

    if current_track:

        print("Title:", current_track["title"])
        print("Artist:", current_track["artist"])
        print("Duration:", current_track["duration_ms"], "ms")
        print("Progress:", current_track["progress_ms"], "ms")
        print("Playing:", current_track["is_playing"])

    else:

        print("No track is currently playing.")
