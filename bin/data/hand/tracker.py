"""Offline palm tracking. Preview JPEGs travel only over the local process pipe."""
import argparse
import math
import os
from pathlib import Path
import sys
import time


class PinchFilter:
    def __init__(self):
        self.closed = False
        self.position = None

    def update(self, points):
        if not points:
            self.closed = False
            self.position = None
            return None
        palm = max(math.hypot(points[5].x - points[17].x,
                              points[5].y - points[17].y), .025)
        ratio = math.hypot(points[4].x - points[8].x,
                           points[4].y - points[8].y) / palm
        self.closed = ratio < (.55 if self.closed else .35)
        raw = ((points[4].x + points[8].x) / 2,
               (points[4].y + points[8].y) / 2)
        if self.position is None:
            self.position = raw
        else:
            self.position = tuple(a * .6 + b * .4 for a, b in zip(self.position, raw))
        return (*self.position, int(self.closed))


class PalmFilter:
    def __init__(self):
        self.position = None

    def update(self, points):
        if not points:
            self.position = None
            return None
        ids = (0, 5, 9, 13, 17)
        raw = tuple(sum(getattr(points[i], axis) for i in ids) / len(ids)
                    for axis in ("x", "y"))
        if self.position is None:
            self.position = raw
        else:
            self.position = tuple(a * .55 + b * .45 for a, b in zip(self.position, raw))
        return self.position


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model", required=True)
    parser.add_argument("--probe", action="store_true")
    parser.add_argument("--camera-probe", action="store_true")
    args = parser.parse_args()
    # MediaPipe's drawing utilities import matplotlib even without a plot.
    # Keep its one-time font cache with the isolated camera environment.
    os.environ.setdefault("MPLCONFIGDIR", str(Path(sys.prefix) / "cache/matplotlib"))
    import cv2
    import mediapipe as mp
    from mediapipe.tasks import python
    from mediapipe.tasks.python import vision
    options = vision.HandLandmarkerOptions(
        base_options=python.BaseOptions(model_asset_path=args.model),
        running_mode=vision.RunningMode.VIDEO, num_hands=1,
        min_hand_detection_confidence=.6, min_tracking_confidence=.6)
    with vision.HandLandmarker.create_from_options(options) as detector:
        if args.probe:
            import numpy as np
            blank = mp.Image(image_format=mp.ImageFormat.SRGB,
                             data=np.zeros((240, 320, 3), dtype=np.uint8))
            assert not detector.detect_for_video(blank, 1).hand_landmarks
            print("PROBE_OK", flush=True)
            return
        camera = cv2.VideoCapture(0)
        if not camera.isOpened():
            print("ERROR CAMERA_UNAVAILABLE", flush=True)
            return
        camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        palm = PalmFilter()
        last_preview = 0.0
        print("READY", flush=True)
        try:
            while True:
                ok, frame = camera.read()
                if not ok:
                    print("ERROR CAMERA_READ", flush=True)
                    break
                frame = cv2.resize(cv2.flip(frame, 1), (320, 240))
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
                result = detector.detect_for_video(image, time.monotonic_ns() // 1000000)
                if args.camera_probe:
                    print(f"CAMERA_OK hands={len(result.hand_landmarks)}", flush=True)
                    break
                points = result.hand_landmarks[0] if result.hand_landmarks else None
                state = palm.update(points)
                stamp = time.monotonic()
                if state:
                    print(f"PALM {stamp:.6f} {state[0]:.5f} {state[1]:.5f}", flush=True)
                else:
                    print(f"LOST {stamp:.6f}", flush=True)
                if stamp - last_preview >= .10:
                    ok, jpeg = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 65])
                    if ok:
                        print("FRAME " + jpeg.tobytes().hex(), flush=True)
                    last_preview = stamp
        finally:
            camera.release()


if __name__ == "__main__":
    try:
        main()
    except BrokenPipeError:
        pass
    except Exception as error:
        print(f"ERROR {type(error).__name__}", flush=True)
        print(str(error), file=sys.stderr)
        sys.exit(1)
