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
    def test_palm_position_and_loss(self):
        palm = tracker.PalmFilter()
        self.assertEqual(palm.update(landmarks(.12)), (.5, .5))
        points = landmarks(.12)
        for point in points:
            point.x += .2
        self.assertAlmostEqual(palm.update(points)[0], .59)
        self.assertIsNone(palm.update(None))
        self.assertAlmostEqual(palm.update(points)[0], .7)

    def test_palm_ignores_finger_pose(self):
        palm = tracker.PalmFilter()
        self.assertEqual(palm.update(landmarks(.12)), palm.update(landmarks(.02)))


if __name__ == "__main__":
    unittest.main()
