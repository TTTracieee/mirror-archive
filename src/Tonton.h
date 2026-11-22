#pragma once
#include "PixelUI.h"

namespace pixel {
// Original code-native pixel sprite, with separate limbs for celebration poses.
inline void tonton(int x, int y, int scale, float time, int happy = 0) {
    const int jump =
        happy ? int(std::abs(std::sin(time * (happy == 2 ? 10 : 6))) * (happy == 3 ? 5 : 3)) : 0;
    const int bob = happy ? 0 : int(std::sin(time * 2) > 0);
    ofPushMatrix();
    ofTranslate(x, y + (bob - jump) * scale);
    ofScale(scale, scale);
    auto block = [](int x, int y, int w, int h, ofColor c) {
        ofSetColor(c);
        ofDrawRectangle(x, y, w, h);
    };
    const ofColor pink(249, 166, 166), pale(255, 196, 177), rose(216, 111, 130), dark(100, 57, 59),
        coat(73, 116, 105);
    block(-11, 15 + jump, 22, 2, ofColor(40, 28, 34, 80));
    block(-10, 0, 20, 14, dark);
    block(-9, 0, 18, 12, pink);
    block(-6, 11, 4, 5, dark);
    block(3, 11, 4, 5, dark);
    block(-8, 5, 16, 7, coat);
    block(-2, 6, 4, 5, gold);
    block(-1, 7, 2, 3, light);
    int arm = happy ? -4 : 4;
    block(-14, arm, 5, 7, dark);
    block(-13, arm, 4, 5, pink);
    block(9, arm, 5, 7, dark);
    block(9, arm, 4, 5, pink);
    block(-12, -20, 7, 10, dark);
    block(5, -20, 7, 10, dark);
    block(-11, -19, 5, 7, pink);
    block(6, -19, 5, 7, pink);
    block(-10, -17, 3, 4, rose);
    block(7, -17, 3, 4, rose);
    block(-13, -13, 26, 14, dark);
    block(-11, -16, 22, 20, dark);
    block(-12, -12, 24, 12, pink);
    block(-10, -15, 20, 17, pink);
    block(-8, -14, 15, 2, pale);
    if (happy) {
        block(-8, -8, 5, 1, dark);
        block(-7, -9, 3, 1, dark);
        block(4, -8, 5, 1, dark);
        block(5, -9, 3, 1, dark);
    } else if (std::fmod(time, 4.2f) < .16f) {
        block(-7, -8, 3, 1, dark);
        block(5, -8, 3, 1, dark);
    } else {
        block(-7, -10, 2, 4, dark);
        block(5, -10, 2, 4, dark);
        block(-7, -10, 1, 1, light);
        block(5, -10, 1, 1, light);
    }
    block(-11, -5, 4, 2, rose);
    block(8, -5, 4, 2, rose);
    block(-4, -5, 9, 5, rose);
    block(-3, -4, 7, 3, pale);
    block(-2, -3, 1, 2, dark);
    block(2, -3, 1, 2, dark);
    block(-2, 1, 5, 1, dark);
    block(-10, -16, 20, 3, coat);
    block(-7, -20, 14, 4, coat);
    block(-4, -21, 8, 1, dark);
    block(-1, -19, 3, 3, gold);
    if (happy == 1) {
        block(10, 0, 6, 4, wood);
        block(12, -3, 2, 3, dark);
    }
    if (happy == 3) {
        block(-7, 3, 14, 10, wood);
        block(-6, 4, 12, 7, paper);
        block(-2, 5, 4, 5, gold);
    }
    ofPopMatrix();
}
} // namespace pixel
