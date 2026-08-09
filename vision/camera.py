"""
Threaded webcam capture that always exposes only the newest frame.

Why a thread: OpenCV's VideoCapture buffers frames internally. If our
processing loop (MediaPipe) runs slower than the camera's frame rate,
read() starts returning progressively older frames and input lag grows
the longer the game runs. A dedicated thread that continuously reads and
overwrites a single slot means we always work from the most recent frame
-- old frames are dropped, never queued.
"""

import threading
import cv2


class Camera:
    def __init__(self, device_index=0, width=640, height=480):
        
        self._capture = cv2.VideoCapture(device_index)
        self._capture.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self._capture.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

        self._lock = threading.Lock()
        self._latest_frame = None
        self._running = False
        self._thread = None

    def is_opened(self):
        return self._capture.isOpened()

    def start(self):
        self._running = True
        self._thread = threading.Thread(target=self._capture_loop, daemon=True)
        self._thread.start()

    def _capture_loop(self):
        while self._running:
            ok, frame = self._capture.read()
            if not ok:
                continue

            frame = cv2.flip(frame, 1)

            with self._lock:
                self._latest_frame = frame  

    def read_latest(self):
        with self._lock:
            return self._latest_frame

    def stop(self):
        self._running = False
        if self._thread is not None:
            self._thread.join(timeout=1.0)
        self._capture.release()