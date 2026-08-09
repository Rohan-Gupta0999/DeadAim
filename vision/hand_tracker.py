"""
Wraps MediaPipe's Hand Landmarker so the rest of the vision process deals
in plain Python dictionaries instead of MediaPipe-specific types. Keeping
this boundary here means the gesture module (next) never imports MediaPipe
directly, and swapping the tracker later touches only this file.
"""

import cv2
import mediapipe as mp
from mediapipe.tasks import python as mp_python
from mediapipe.tasks.python import vision as mp_vision


AIM_LANDMARK_INDEX = 9


class HandTracker:
    def __init__(self, model_path, max_hands=2,
                 detection_confidence=0.5, tracking_confidence=0.5):
        base_options = mp_python.BaseOptions(model_asset_path=model_path)
        options = mp_vision.HandLandmarkerOptions(
            base_options=base_options,
            running_mode=mp_vision.RunningMode.VIDEO,
            num_hands=max_hands,
            min_hand_detection_confidence=detection_confidence,
            min_tracking_confidence=tracking_confidence,
        )
        self._landmarker = mp_vision.HandLandmarker.create_from_options(options)

    def process(self, frame_bgr, timestamp_ms):
        """Returns a list of plain dicts -- one per detected hand."""
        frame_rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
        mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame_rgb)
        result = self._landmarker.detect_for_video(mp_image, timestamp_ms)
        return self._to_plain_data(result)

    def _to_plain_data(self, result):
        hands = []
        for i, landmarks in enumerate(result.hand_landmarks):
            handedness = "Unknown"
            confidence = 0.0
            if i < len(result.handedness) and result.handedness[i]:
                handedness = result.handedness[i][0].category_name
                confidence = float(result.handedness[i][0].score)

            hands.append({
                "handedness": handedness,
                "confidence": round(confidence, 4),
                
                "landmarks": [
                    {"x": round(lm.x, 4), "y": round(lm.y, 4), "z": round(lm.z, 4)}
                    for lm in landmarks
                ],
            })
        return hands

    def close(self):
        self._landmarker.close()