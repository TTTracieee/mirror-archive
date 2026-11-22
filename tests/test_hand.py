import importlib.util
from pathlib import Path
from types import SimpleNamespace
import unittest

path = Path(__file__).resolve().parents[1] / "bin/data/hand/tracker.py"
spec = importlib.util.spec_from_file_location("tracker", path)
tracker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tracker)


def landmarks(gap, scale=1):
    points = [SimpleNamespace(x=.5, y=.5) for _ in range(21)]
    points[5].x = .5 - .1 * scale
    points[17].x = .5 + .1 * scale
    points[4].x = .5 - gap * scale / 2
    points[8].x = .5 + gap * scale / 2
    return points


class HandTests(unittest.TestCase):
    def test_pinch_hysteresis(self):
        pinch = tracker.PinchFilter()
        self.assertEqual(pinch.update(landmarks(.12))[2], 0)
        self.assertEqual(pinch.update(landmarks(.06))[2], 1)
        self.assertEqual(pinch.update(landmarks(.09))[2], 1)
        self.assertEqual(pinch.update(landmarks(.12))[2], 0)

    def test_distance_invariance(self):
        for scale in [.5, 1, 2]:
            pinch = tracker.PinchFilter()
            self.assertEqual(pinch.update(landmarks(.06, scale))[2], 1)
            self.assertEqual(pinch.update(landmarks(.13, scale))[2], 0)

    def test_loss_cancels_and_resets(self):
        pinch = tracker.PinchFilter()
        pinch.update(landmarks(.06))
        self.assertIsNone(pinch.update(None))
        self.assertFalse(pinch.closed)
        self.assertIsNone(pinch.position)

    def test_cursor_smoothing(self):
        pinch = tracker.PinchFilter()
        pinch.update(landmarks(.12))
        points = landmarks(.12)
        for point in points:
            point.x += .1
        self.assertAlmostEqual(pinch.update(points)[0], .54)


if __name__ == "__main__":
    unittest.main()
