#pragma once
#include <algorithm>

namespace mirror {
constexpr int canvasWidth = 640;
constexpr int canvasHeight = 384;

struct PixelViewport {
    int scale, x, y;
    float localX(float screenX) const {
        return (screenX - x) / scale;
    }
    float localY(float screenY) const {
        return (screenY - y) / scale;
    }
};

// Display and input share this transform, including the letterboxed margins.
inline PixelViewport pixelViewport(int width, int height) {
    const int scale = std::max(1, std::min(width / canvasWidth, height / canvasHeight));
    return {scale, (width - canvasWidth * scale) / 2, (height - canvasHeight * scale) / 2};
}
} // namespace mirror
