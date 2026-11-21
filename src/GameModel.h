#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <array>
#include <cmath>

namespace mirror {
struct Session {
    float energy = 100.f;
    int mistakes = 0;
    int hints = 0;
    int scans = 0;
    bool paused = false;
    bool hasMatch = false;
    bool solved = false;
    std::array<bool, 3> evidence{};
    std::array<cv::Mat, 4> masks;

    explicit Session(cv::Size size = {900, 560}) { reset(size); }
    void reset(cv::Size size) {
        energy = 100; mistakes = hints = scans = 0;
        paused = hasMatch = solved = false; evidence.fill(false);
        for (auto& mask : masks) mask = cv::Mat::zeros(size, CV_8UC1);
    }
    bool spend(float amount) {
        if (amount < 0 || energy + .0001f < amount) return false;
        energy = std::max(0.f, energy - amount); return true;
    }
    // A layer bills only newly uncovered pixels; revisiting an area is free.
    bool brush(int layer, cv::Point center, int radius, float fullImageCost) {
        if (layer < 0 || layer >= int(masks.size()) || radius <= 0) return false;
        auto& mask = masks[layer];
        cv::Rect bounds(center.x-radius-2, center.y-radius-2, radius*2+5, radius*2+5);
        bounds &= cv::Rect(0, 0, mask.cols, mask.rows);
        if (bounds.empty()) return false;
        cv::Mat stamp = cv::Mat::zeros(bounds.size(), CV_8UC1);
        cv::circle(stamp, center - bounds.tl(), radius, cv::Scalar(255), -1, cv::LINE_AA);
        cv::Mat proposed; cv::max(mask(bounds), stamp, proposed);
        const float amount = float(cv::sum(proposed)[0] - cv::sum(mask(bounds))[0])
            / (255.f * float(mask.total())) * fullImageCost;
        if (!spend(amount)) return false;
        proposed.copyTo(mask(bounds)); return true;
    }
    bool ready(int level) const {
        if (level == 0) return evidence[1] && evidence[2];
        if (level == 1) return hasMatch;
        return evidence[1] || evidence[2];
    }
    int stars() const {
        if (!solved) return 0;
        return energy >= 50 && mistakes == 0 && hints == 0 ? 3 : (energy >= 20 && mistakes < 3 ? 2 : 1);
    }
};
} // namespace mirror
