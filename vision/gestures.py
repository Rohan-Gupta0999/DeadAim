"""
Gesture recognition from MediaPipe hand landmarks.

Design note: everything here works on plain dicts from hand_tracker.py --
this module never imports MediaPipe. That keeps gesture logic testable
and swappable independently of the tracker underneath it.

Pose summary -- every pair differs on at least one BINARY finger test,
which is what makes them mutually exclusive rather than merely unlikely
to collide:

              index     middle    ring      pinky
    Gun       extended  extended  curled    curled     (+ joined, thumb = trigger)
    Bow       pinched   curled    curled    curled
    Fireball  extended  extended  extended  extended   (+ thumb out)
"""

import math

# MediaPipe hand landmark indices
WRIST = 0
THUMB_TIP = 4
INDEX_MCP = 5
INDEX_PIP = 6
INDEX_TIP = 8
MIDDLE_MCP = 9
MIDDLE_PIP = 10
MIDDLE_TIP = 12
RING_PIP = 14
RING_TIP = 16
PINKY_PIP = 18
PINKY_TIP = 20

# --- Tuning knobs -----------------------------------------------------
# GUN: thumb tip -> index knuckle, over hand size. Below this = firing.
THUMB_DOWN_RATIO = 0.55

# GUN: index tip -> middle tip, over hand size. Below this the two
# fingers read as joined into a "barrel" rather than spread in a V.
FINGERS_JOINED_RATIO = 0.35

# BOW: thumb tip -> index TIP, over hand size. Below this = pinching.
PINCH_RATIO = 0.35

MIN_HAND_CONFIDENCE = 0.6
# ----------------------------------------------------------------------


def _distance(a, b):
    return math.hypot(a["x"] - b["x"], a["y"] - b["y"])


def _hand_size(lm):
    """Wrist to middle knuckle -- a stable reference length that scales
    with how close the hand is to the camera. Every other measurement is
    divided by this so thresholds work at any distance."""
    return max(_distance(lm[WRIST], lm[MIDDLE_MCP]), 1e-6)


def _is_finger_extended(lm, pip_index, tip_index):
    """A finger is extended when its tip is further from the wrist than
    its middle joint -- rotation-invariant, unlike comparing Y values."""
    return _distance(lm[WRIST], lm[tip_index]) > _distance(lm[WRIST], lm[pip_index])


def _three_fingers_curled(lm):
    """Middle, ring and pinky curled -- the bow pose's requirement."""
    return (not _is_finger_extended(lm, MIDDLE_PIP, MIDDLE_TIP)
            and not _is_finger_extended(lm, RING_PIP, RING_TIP)
            and not _is_finger_extended(lm, PINKY_PIP, PINKY_TIP))


def _ring_and_pinky_curled(lm):
    """The gun pose curls only these two -- the middle finger is part of
    the barrel, which is exactly what makes the gun and bow poses
    mutually exclusive: bow requires the middle curled, gun requires it
    extended. Same test, opposite answers, so both can never be true."""
    return (not _is_finger_extended(lm, RING_PIP, RING_TIP)
            and not _is_finger_extended(lm, PINKY_PIP, PINKY_TIP))


def _all_fingers_extended(lm):
    """Index, middle, ring and pinky all out. Ring+pinky are what make
    this impossible to confuse with the gun pose, which requires both
    curled -- a binary disagreement, not a threshold one."""
    return (_is_finger_extended(lm, INDEX_PIP, INDEX_TIP)
            and _is_finger_extended(lm, MIDDLE_PIP, MIDDLE_TIP)
            and _is_finger_extended(lm, RING_PIP, RING_TIP)
            and _is_finger_extended(lm, PINKY_PIP, PINKY_TIP))


def thumb_openness(lm):
    """Thumb tip to index knuckle, normalised. Small = thumb folded."""
    return _distance(lm[THUMB_TIP], lm[INDEX_MCP]) / _hand_size(lm)


def pinch_distance(lm):
    """Thumb tip to index TIP, normalised. Small = pinching."""
    return _distance(lm[THUMB_TIP], lm[INDEX_TIP]) / _hand_size(lm)


def finger_gap(lm):
    """Index tip to middle tip, normalised. Small = fingers joined."""
    return _distance(lm[INDEX_TIP], lm[MIDDLE_TIP]) / _hand_size(lm)


class StableFlag:
    """Debounces a noisy boolean.

    A raw per-frame test flickers -- one bad frame in thirty would
    otherwise become a stray shot. This requires N consecutive frames to
    agree before the output flips.

    Different counts for on and off (hysteresis) matter: a state sitting
    right at its threshold would otherwise oscillate every frame.
    """

    def __init__(self, frames_to_on, frames_to_off, initial=False):
        self._frames_to_on = frames_to_on
        self._frames_to_off = frames_to_off
        self._state = initial
        self._streak = 0

    def update(self, raw_value):
        if raw_value == self._state:
            self._streak = 0
            return self._state

        self._streak += 1
        needed = self._frames_to_on if raw_value else self._frames_to_off
        if self._streak >= needed:
            self._state = raw_value
            self._streak = 0

        return self._state

    @property
    def state(self):
        return self._state


class GunGesture:
    """Index and middle extended and held together, ring and pinky
    curled. Thumb drop = trigger.

    The thumb is deliberately NOT part of the pose test -- it's the
    trigger, so the pose must stay valid while firing.

    Why the middle finger is in the barrel: the bow's release motion
    passes through "index extended, everything else curled, thumb low",
    which was the original gun pose mid-fire. Requiring the middle
    finger extended makes that impossible, since the bow pose requires
    it curled.
    """

    POSE_FRAMES_ON = 5
    POSE_FRAMES_OFF = 8
    TRIGGER_FRAMES_ON = 2
    TRIGGER_FRAMES_OFF = 3

    def __init__(self):
        self._pose = StableFlag(self.POSE_FRAMES_ON, self.POSE_FRAMES_OFF)
        self._trigger = StableFlag(self.TRIGGER_FRAMES_ON, self.TRIGGER_FRAMES_OFF)

    def update(self, hand):
        if hand is None:
            self._pose.update(False)
            self._trigger.update(False)
            return {"active": False, "firing": False,
                    "thumb_ratio": 0.0, "gap_ratio": 0.0}

        lm = hand["landmarks"]
        gap = finger_gap(lm)

        raw_pose = (_is_finger_extended(lm, INDEX_PIP, INDEX_TIP)
                    and _is_finger_extended(lm, MIDDLE_PIP, MIDDLE_TIP)
                    and gap < FINGERS_JOINED_RATIO
                    and _ring_and_pinky_curled(lm))
        active = self._pose.update(raw_pose)

        openness = thumb_openness(lm)
        # Trigger only evaluated inside the pose -- the single biggest
        # accidental-fire preventer.
        self._trigger.update(active and openness < THUMB_DOWN_RATIO)

        return {
            "active": active,
            "firing": self._trigger.state and active,
            "thumb_ratio": round(openness, 3),
            "gap_ratio": round(gap, 3),
        }


class BowGesture:
    """Index tip touching thumb tip, other three curled.

    Note this reports only whether the pinch is held. The RELEASE -- the
    thing that actually looses the arrow -- is detected on the C++ side
    as the falling edge of this state. Sending a one-shot "released"
    event over the socket would risk being read twice (double arrow) or
    missed entirely, since the game polls the latest message rather than
    consuming a queue.
    """

    # Slightly slower to engage than to drop: a deliberate pinch, but a
    # quick release, because the release is the shot.
    DRAW_FRAMES_ON = 3
    DRAW_FRAMES_OFF = 3

    def __init__(self):
        self._draw = StableFlag(self.DRAW_FRAMES_ON, self.DRAW_FRAMES_OFF)

    def update(self, hand):
        if hand is None:
            self._draw.update(False)
            return {"active": False, "pinch_ratio": 0.0}

        lm = hand["landmarks"]
        pinch = pinch_distance(lm)
        raw_draw = (pinch < PINCH_RATIO) and _three_fingers_curled(lm)

        return {
            "active": self._draw.update(raw_draw),
            "pinch_ratio": round(pinch, 3),
        }


class FireballGesture:
    """Flat open palm -- all four fingers extended, thumb out.

    Requiring the thumb out makes this a deliberate "high five" rather
    than four fingers up with the thumb tucked, which is a much more
    common accidental hand shape.

    Note the hysteresis runs the opposite way to the gun's: slow to
    engage, quick to drop. Dropping IS the throw, so it has to be
    responsive -- same reasoning as the bow.
    """

    POSE_FRAMES_ON = 5
    POSE_FRAMES_OFF = 3

    def __init__(self):
        self._pose = StableFlag(self.POSE_FRAMES_ON, self.POSE_FRAMES_OFF)

    def update(self, hand):
        if hand is None:
            self._pose.update(False)
            return {"active": False}

        lm = hand["landmarks"]
        # Reuses THUMB_DOWN_RATIO deliberately: it measures thumb tip to
        # index knuckle, and "above the threshold" is exactly "thumb is
        # out". Same measurement, opposite side of the comparison.
        raw = _all_fingers_extended(lm) and thumb_openness(lm) > THUMB_DOWN_RATIO
        return {"active": self._pose.update(raw)}


class GestureRecognizer:
    """Owns every recogniser and resolves which one wins.

    All three are updated every frame even though only one can win --
    their debouncers need to see every frame or their state freezes at
    whatever it held when they were last consulted.

    The order below is a formality: the three poses are pairwise
    mutually exclusive on binary finger tests, so no two can be active
    at once. It's here for determinism, not arbitration.
    """

    def __init__(self):
        self._gun = GunGesture()
        self._bow = BowGesture()
        self._fireball = FireballGesture()

    def update(self, hand):
        usable = hand if (hand is not None
                          and hand["confidence"] >= MIN_HAND_CONFIDENCE) else None

        bow = self._bow.update(usable)
        fireball = self._fireball.update(usable)
        gun = self._gun.update(usable)

        if bow["active"]:
            gesture, firing = "bow", False
        elif fireball["active"]:
            gesture, firing = "fireball", False
        elif gun["active"]:
            gesture, firing = "gun", gun["firing"]
        else:
            gesture, firing = "none", False

        return {
            "gesture": gesture,
            "firing": firing,
            "thumb_ratio": gun["thumb_ratio"],
            "gap_ratio": gun["gap_ratio"],
            "pinch_ratio": bow["pinch_ratio"],
        }