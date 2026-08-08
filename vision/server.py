"""
The vision process: capture -> hand tracking -> gestures -> JSON -> game.

Runs completely independently of the game. If the game isn't running yet,
or restarts, this keeps going and waits for it to reconnect.
"""

import json
import os
import socket
import time

import cv2

from camera import Camera
from hand_tracker import HandTracker, AIM_LANDMARK_INDEX
from gestures import GestureRecognizer

HOST = "127.0.0.1"
PORT = 50505
TARGET_FPS = 30

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(SCRIPT_DIR, "hand_landmarker.task")

# Keep True while tuning gestures -- the on-screen thumb ratio is how you
# pick a threshold. Set False for the actual demo to save CPU/GPU.
SHOW_PREVIEW = True

ACTIVE_REGION_MIN = 0.25
ACTIVE_REGION_MAX = 0.75


def map_to_aim(value):
    span = ACTIVE_REGION_MAX - ACTIVE_REGION_MIN
    mapped = (value - ACTIVE_REGION_MIN) / span
    return min(max(mapped, 0.0), 1.0)


def pick_aim_hand(hands):
    """Highest-confidence hand drives aim and gestures. With two hands up
    (bow, later), this keeps control on whichever the tracker is surest
    of rather than flickering between them by list order."""
    if not hands:
        return None
    return max(hands, key=lambda h: h["confidence"])


def build_message(hands, recognizer):
    """The one place the wire format is defined."""
    aim_hand = pick_aim_hand(hands)

    # Always update, even with no hand -- the debouncers need to see the
    # empty frames so their states decay instead of freezing.
    result = recognizer.update(aim_hand)

    if aim_hand is None:
        return {
            "tracking": False,
            "hand_count": 0,
            "confidence": 0.0,
            "aim": {"x": 0.5, "y": 0.5},
            "gesture": result["gesture"],
            "firing": result["firing"],
            "hands": [],
        }, result

    anchor = aim_hand["landmarks"][AIM_LANDMARK_INDEX]
    message = {
        "tracking": True,
        "hand_count": len(hands),
        "confidence": aim_hand["confidence"],
        "aim": {
            "x": round(map_to_aim(anchor["x"]), 4),
            "y": round(map_to_aim(anchor["y"]), 4),
        },
        "gesture": result["gesture"],
        "firing": result["firing"],
        "hands": hands,
    }
    return message, result


def draw_preview(frame, hands, result):
    for hand in hands:
        for lm in hand["landmarks"]:
            x = int(lm["x"] * frame.shape[1])
            y = int(lm["y"] * frame.shape[0])
            cv2.circle(frame, (x, y), 4, (0, 255, 0), -1)

        anchor = hand["landmarks"][AIM_LANDMARK_INDEX]
        ax = int(anchor["x"] * frame.shape[1])
        ay = int(anchor["y"] * frame.shape[0])
        cv2.circle(frame, (ax, ay), 10, (0, 0, 255), 2)

    colours = {"gun": (0, 255, 0), "bow": (255, 200, 0),
               "fireball": (0, 120, 255), "none": (150, 150, 150)}
    cv2.putText(frame, f"MODE: {result['gesture'].upper()}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, colours[result["gesture"]], 2)

    # Both tuning numbers. THUMB is the gun trigger; PINCH is the bow draw.
    cv2.putText(frame, f"THUMB: {result['thumb_ratio']:.3f}", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)
    cv2.putText(frame, f"PINCH: {result['pinch_ratio']:.3f}", (10, 85),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)

    cv2.putText(frame, f"GAP:   {result['gap_ratio']:.3f}", (10, 110),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)

    if result["firing"]:
        cv2.putText(frame, "FIRING", (10, 150),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 0, 255), 3)

    cv2.imshow("DeadAim Vision (press q to close preview)", frame)
    return cv2.waitKey(1) & 0xFF != ord("q")


def stream_to_game(conn, camera, tracker, recognizer, show_preview):
    frame_interval = 1.0 / TARGET_FPS
    start_time = time.time()
    last_timestamp_ms = -1

    while True:
        loop_start = time.time()
        frame = camera.read_latest()

        if frame is not None:
            # detect_for_video requires strictly increasing timestamps --
            # this guard makes that impossible to violate even if two
            # frames land in the same millisecond.
            timestamp_ms = int((time.time() - start_time) * 1000)
            if timestamp_ms <= last_timestamp_ms:
                timestamp_ms = last_timestamp_ms + 1
            last_timestamp_ms = timestamp_ms

            hands = tracker.process(frame, timestamp_ms)
            message, gesture_result = build_message(hands, recognizer)

            if show_preview:
                show_preview = draw_preview(frame.copy(), hands, gesture_result)
        else:
            message, _ = build_message([], recognizer)  # camera warming up

        conn.sendall((json.dumps(message) + "\n").encode("utf-8"))

        remaining = frame_interval - (time.time() - loop_start)
        if remaining > 0:
            time.sleep(remaining)


def main():
    if not os.path.exists(MODEL_PATH):
        print(f"[vision] Model file not found: {MODEL_PATH}")
        return

    camera = Camera()
    if not camera.is_opened():
        print("[vision] Could not open the webcam.")
        return
    camera.start()

    tracker = HandTracker(MODEL_PATH)
    recognizer = GestureRecognizer()

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(1)
    print(f"[vision] Ready. Waiting for the game on {HOST}:{PORT} ...")

    try:
        while True:
            conn, addr = server.accept()
            print(f"[vision] Game connected: {addr}")
            try:
                stream_to_game(conn, camera, tracker, recognizer, SHOW_PREVIEW)
            except (BrokenPipeError, ConnectionResetError):
                print("[vision] Game disconnected -- waiting for it again.")
            finally:
                conn.close()
    except KeyboardInterrupt:
        print("\n[vision] Shutting down.")
    finally:
        tracker.close()
        camera.stop()
        cv2.destroyAllWindows()
        server.close()


if __name__ == "__main__":
    main()