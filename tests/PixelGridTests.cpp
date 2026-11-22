#include "PixelGrid.h"
#include <iostream>
#include <stdexcept>
#include <string>

static int checks = 0;
void require(bool condition, const std::string& message) {
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}
int main() {
    try {
        using namespace mirror;
        auto retina = pixelViewport(2560, 1536);
        require(retina.scale == 4 && retina.x == 0 && retina.y == 0, "Retina uses exact 4x pixels");
        auto standard = pixelViewport(1280, 768);
        require(standard.scale == 2, "Standard display uses exact 2x pixels");
        auto resized = pixelViewport(2010, 1300);
        require(resized.scale == 3 && resized.x == 45 && resized.y == 74,
                "Integer scaling and letterboxing");
        require(resized.localX(45 + 143 * 3) == 143 && resized.localY(74 + 211 * 3) == 211,
                "Pointer transform reverses letterboxing");
        require(resized.localX(0) < 0, "Margin clicks stay outside the board");
        std::cout << "PASS viewport: " << checks << " checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}
