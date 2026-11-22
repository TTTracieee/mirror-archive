#pragma once
#include "ofMain.h"
#include <array>
#include <cctype>

namespace pixel {
inline const ofColor ink{63, 35, 32}, shadow{39, 25, 35}, wood{157, 86, 48};
inline const ofColor paper{245, 216, 155}, light{255, 239, 184}, edge{102, 53, 38};
inline const ofColor green{100, 143, 65}, teal{61, 177, 162}, gold{247, 189, 65}, red{190, 77, 58};

// Original 5x7 bitmap alphabet. Each set bit is an opaque canvas pixel.
// No font atlas, system font, hinting, antialiasing or fractional glyph scale.
inline std::array<unsigned char, 7> glyph(char value) {
    switch (std::toupper(static_cast<unsigned char>(value))) {
    case 'A':
        return {14, 17, 17, 31, 17, 17, 17};
    case 'B':
        return {30, 17, 17, 30, 17, 17, 30};
    case 'C':
        return {14, 17, 16, 16, 16, 17, 14};
    case 'D':
        return {30, 17, 17, 17, 17, 17, 30};
    case 'E':
        return {31, 16, 16, 30, 16, 16, 31};
    case 'F':
        return {31, 16, 16, 30, 16, 16, 16};
    case 'G':
        return {14, 17, 16, 23, 17, 17, 15};
    case 'H':
        return {17, 17, 17, 31, 17, 17, 17};
    case 'I':
        return {14, 4, 4, 4, 4, 4, 14};
    case 'J':
        return {7, 2, 2, 2, 2, 18, 12};
    case 'K':
        return {17, 18, 20, 24, 20, 18, 17};
    case 'L':
        return {16, 16, 16, 16, 16, 16, 31};
    case 'M':
        return {17, 27, 21, 21, 17, 17, 17};
    case 'N':
        return {17, 25, 21, 19, 17, 17, 17};
    case 'O':
        return {14, 17, 17, 17, 17, 17, 14};
    case 'P':
        return {30, 17, 17, 30, 16, 16, 16};
    case 'Q':
        return {14, 17, 17, 17, 21, 18, 13};
    case 'R':
        return {30, 17, 17, 30, 20, 18, 17};
    case 'S':
        return {15, 16, 16, 14, 1, 1, 30};
    case 'T':
        return {31, 4, 4, 4, 4, 4, 4};
    case 'U':
        return {17, 17, 17, 17, 17, 17, 14};
    case 'V':
        return {17, 17, 17, 17, 17, 10, 4};
    case 'W':
        return {17, 17, 17, 21, 21, 21, 10};
    case 'X':
        return {17, 17, 10, 4, 10, 17, 17};
    case 'Y':
        return {17, 17, 10, 4, 4, 4, 4};
    case 'Z':
        return {31, 1, 2, 4, 8, 16, 31};
    case '0':
        return {14, 17, 19, 21, 25, 17, 14};
    case '1':
        return {4, 12, 4, 4, 4, 4, 14};
    case '2':
        return {14, 17, 1, 2, 4, 8, 31};
    case '3':
        return {30, 1, 1, 14, 1, 1, 30};
    case '4':
        return {2, 6, 10, 18, 31, 2, 2};
    case '5':
        return {31, 16, 16, 30, 1, 1, 30};
    case '6':
        return {14, 16, 16, 30, 17, 17, 14};
    case '7':
        return {31, 1, 2, 4, 8, 8, 8};
    case '8':
        return {14, 17, 17, 14, 17, 17, 14};
    case '9':
        return {14, 17, 17, 15, 1, 1, 14};
    case '+':
        return {0, 4, 4, 31, 4, 4, 0};
    case '-':
        return {0, 0, 0, 31, 0, 0, 0};
    case '/':
        return {1, 1, 2, 4, 8, 16, 16};
    case ':':
        return {0, 4, 4, 0, 4, 4, 0};
    case '.':
        return {0, 0, 0, 0, 0, 6, 6};
    case ',':
        return {0, 0, 0, 0, 6, 6, 4};
    case '!':
        return {4, 4, 4, 4, 4, 0, 4};
    case '?':
        return {14, 17, 1, 2, 4, 0, 4};
    case '<':
        return {1, 2, 4, 8, 4, 2, 1};
    case '>':
        return {16, 8, 4, 2, 4, 8, 16};
    case '^':
        return {4, 14, 21, 4, 4, 4, 4};
    case '[':
        return {14, 8, 8, 8, 8, 8, 14};
    case ']':
        return {14, 2, 2, 2, 2, 2, 14};
    case '(':
        return {2, 4, 8, 8, 8, 4, 2};
    case ')':
        return {8, 4, 2, 2, 2, 4, 8};
    case 39:
        return {4, 4, 2, 0, 0, 0, 0};
    case ' ':
        return {};
    default:
        return {14, 17, 1, 2, 4, 0, 4};
    }
}
inline int textWidth(const std::string& value, int scale = 1) {
    int width = 0, row = 0;
    for (char c : value) {
        if (c == '\n') {
            width = std::max(width, row);
            row = 0;
        } else
            row += 6 * scale;
    }
    return std::max(0, std::max(width, row) - scale);
}
inline void text(const std::string& value, int x, int y, ofColor color, int scale = 1) {
    ofSetColor(color);
    int column = x;
    for (char c : value) {
        if (c == '\n') {
            column = x;
            y += 10 * scale;
            continue;
        }
        const auto rows = glyph(c);
        for (int r = 0; r < 7; ++r)
            for (int bit = 0; bit < 5; ++bit)
                if (rows[r] & (1 << (4 - bit)))
                    ofDrawRectangle(column + bit * scale, y + r * scale, scale, scale);
        column += 6 * scale;
    }
}
} // namespace pixel
