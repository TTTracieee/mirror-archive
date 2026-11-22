#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

namespace homeward {
enum class Direction { None, Left, Right, Up, Down };
inline cv::Point step(Direction d) {
    switch (d) {
    case Direction::Left:
        return {-1, 0};
    case Direction::Right:
        return {1, 0};
    case Direction::Up:
        return {0, -1};
    case Direction::Down:
        return {0, 1};
    default:
        return {};
    }
}
inline constexpr std::array<Direction, 4> directions{Direction::Left, Direction::Right,
                                                     Direction::Up, Direction::Down};

enum class Filter { Clean, Contrast, Detail };
class BrushRepair {
    struct Edit {
        std::array<cv::Mat, 3> filterMasks;
        std::array<float, 3> filterStrengths;
    };
    std::array<cv::Mat, 3> filterMasks;
    std::array<float, 3> filterStrengths{1.f, .8f, .45f};
    std::vector<Edit> history;
    static void blend(const cv::Mat& processed, cv::Mat& result, const cv::Mat& layer,
                      float amount) {
        if (amount <= 0 || cv::countNonZero(layer) == 0)
            return;
        // Feather inward so untouched pixels remain exactly unchanged.
        cv::Mat distance;
        cv::distanceTransform(layer, distance, cv::DIST_L2, 3);
        for (int y = 0; y < result.rows; ++y)
            for (int x = 0; x < result.cols; ++x) {
                float a = std::min(1.f, distance.at<float>(y, x) / 5.f) * amount;
                auto& pixel = result.at<cv::Vec3b>(y, x);
                auto target = processed.at<cv::Vec3b>(y, x);
                for (int c = 0; c < 3; ++c)
                    pixel[c] = cv::saturate_cast<uchar>(pixel[c] * (1 - a) + target[c] * a);
            }
    }

public:
    cv::Mat damaged, mask;
    // Estimate exposure from observed luminance only; no access to degradation parameters.
    static cv::Mat autoContrast(const cv::Mat& observed) {
        if (observed.empty() || observed.type() != CV_8UC3)
            throw std::invalid_argument("Contrast needs an 8-bit color image");
        cv::Mat gray, stats;
        cv::cvtColor(observed, gray, cv::COLOR_BGR2GRAY);
        cv::medianBlur(gray, stats, 3);
        std::array<int, 256> histogram{};
        for (int y = 0; y < stats.rows; ++y)
            for (int x = 0; x < stats.cols; ++x)
                ++histogram[stats.at<uchar>(y, x)];
        auto percentile = [&](float fraction) {
            int total = 0, target = std::max(1, int(stats.total() * fraction));
            for (int i = 0; i < 256; ++i) {
                total += histogram[i];
                if (total >= target)
                    return i;
            }
            return 255;
        };
        int low = percentile(.02f), high = percentile(.98f);
        if (high - low < 8)
            return observed.clone();
        double gain = std::min(3., 235. / (high - low));
        cv::Mat result;
        // Raise weak local detail around its own mean, keeping the wall's illumination.
        cv::Mat luminance, mean, squareMean, detailSource = observed.clone();
        gray.convertTo(luminance, CV_32F);
        cv::GaussianBlur(luminance, mean, {0, 0}, 3.);
        cv::GaussianBlur(luminance.mul(luminance), squareMean, {0, 0}, 3.);
        for (int y = 0; y < observed.rows; ++y)
            for (int x = 0; x < observed.cols; ++x) {
                float average = mean.at<float>(y, x);
                float deviation =
                    std::sqrt(std::max(0.f, squareMean.at<float>(y, x) - average * average));
                float localGain = deviation < 1 ? 1 : std::clamp(22.f / (deviation + 1), 1.f, 6.f);
                float delta = (localGain - 1) * (luminance.at<float>(y, x) - average);
                auto& pixel = detailSource.at<cv::Vec3b>(y, x);
                for (int c = 0; c < 3; ++c)
                    pixel[c] = cv::saturate_cast<uchar>(pixel[c] + delta);
            }
        detailSource.convertTo(result, CV_8U, gain, 10 - low * gain);
        // Local luminance equalization recovers uneven fading without knowing where the clue is.
        cv::Mat lab, local;
        cv::cvtColor(result, lab, cv::COLOR_BGR2Lab);
        std::vector<cv::Mat> channels;
        cv::split(lab, channels);
        cv::createCLAHE(3., {16, 12})->apply(channels[0], channels[0]);
        cv::merge(channels, lab);
        cv::cvtColor(lab, local, cv::COLOR_Lab2BGR);
        cv::addWeighted(result, .65, local, .35, 0, result);
        return result;
    }
    void load(const cv::Mat& source, cv::Rect fadedArea = {}) {
        cv::Mat faded;
        cv::GaussianBlur(source, faded, {3, 3}, .55);
        faded.convertTo(damaged, CV_8U, .62, 43);
        // A smooth, local age stain compresses the ink range but never erases the observation.
        cv::Mat localPaper;
        cv::GaussianBlur(damaged, localPaper, {0, 0}, 5.);
        auto area = fadedArea & cv::Rect(0, 0, damaged.cols, damaged.rows);
        for (int y = area.y; y < area.br().y; ++y)
            for (int x = area.x; x < area.br().x; ++x) {
                float edge =
                    std::min({x - area.x + 1, area.br().x - x, y - area.y + 1, area.br().y - y}) /
                    10.f;
                float weight = std::min(1.f, edge);
                float amount = .91f * weight * weight * (3 - 2 * weight);
                auto& pixel = damaged.at<cv::Vec3b>(y, x);
                const auto paper = localPaper.at<cv::Vec3b>(y, x);
                for (int c = 0; c < 3; ++c)
                    pixel[c] =
                        cv::saturate_cast<uchar>(pixel[c] * (1 - amount) + paper[c] * amount);
            }
        cv::RNG random(208);
        for (size_t i = 0; i < damaged.total() / 40; ++i) {
            int x = random.uniform(0, damaged.cols), y = random.uniform(0, damaged.rows);
            bool light = random.uniform(0, 2);
            damaged.at<cv::Vec3b>(y, x) = light ? cv::Vec3b(191, 216, 229) : cv::Vec3b(48, 62, 78);
        }
        reset();
    }
    void reset() {
        for (auto& layer : filterMasks)
            layer = cv::Mat::zeros(damaged.size(), CV_8U);
        mask = cv::Mat::zeros(damaged.size(), CV_8U);
        filterStrengths = {1.f, .8f, .45f};
        history.clear();
    }
    float strength(Filter filter) const {
        return filterStrengths[int(filter)];
    }
    void setStrength(Filter filter, float value) {
        filterStrengths[int(filter)] = std::clamp(value, 0.f, 1.f);
    }
    void beginStroke() {
        Edit edit;
        for (int i = 0; i < 3; ++i)
            edit.filterMasks[i] = filterMasks[i].clone();
        edit.filterStrengths = filterStrengths;
        history.push_back(edit);
        if (history.size() > 30)
            history.erase(history.begin());
    }
    bool undo() {
        if (history.empty())
            return false;
        filterMasks = history.back().filterMasks;
        filterStrengths = history.back().filterStrengths;
        history.pop_back();
        mask = filterMasks[0] | filterMasks[1] | filterMasks[2];
        return true;
    }
    void stroke(cv::Point from, cv::Point to, int radius = 19, Filter filter = Filter::Clean) {
        auto& layer = filterMasks[int(filter)];
        cv::line(layer, from, to, 255, radius * 2, cv::LINE_8);
        cv::circle(layer, to, radius, 255, cv::FILLED);
        mask = filterMasks[0] | filterMasks[1] | filterMasks[2];
    }
    bool peopleRestored() const {
        // Story feedback measures visible work in the family area, not face recognition.
        cv::Rect area = cv::Rect(205, 125, 110, 160) & cv::Rect(0, 0, damaged.cols, damaged.rows);
        if (area.empty())
            return false;
        auto result = image();
        cv::Mat difference, gray, visible;
        cv::absdiff(result(area), damaged(area), difference);
        cv::cvtColor(difference, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, visible, 6, 255, cv::THRESH_BINARY);
        return cv::countNonZero(visible) > area.area() * .45;
    }
    // Rebuild from the observation in a fixed order; repeated brushing cannot compound damage.
    cv::Mat image() const {
        cv::Mat result = damaged.clone(), processed;
        if (cv::countNonZero(filterMasks[0]) && filterStrengths[0] > 0) {
            cv::Mat median, difference, gray, outliers;
            cv::medianBlur(result, median, 3);
            cv::absdiff(result, median, difference);
            cv::cvtColor(difference, gray, cv::COLOR_BGR2GRAY);
            cv::threshold(gray, outliers, 24, 255, cv::THRESH_BINARY);
            processed = result.clone();
            median.copyTo(processed, outliers);
            blend(processed, result, filterMasks[0], filterStrengths[0]);
        }
        if (cv::countNonZero(filterMasks[1]) && filterStrengths[1] > 0) {
            processed = autoContrast(result);
            blend(processed, result, filterMasks[1], filterStrengths[1]);
        }
        if (cv::countNonZero(filterMasks[2]) && filterStrengths[2] > 0) {
            cv::Mat soft;
            cv::GaussianBlur(result, soft, {0, 0}, .8);
            cv::addWeighted(result, 1.6, soft, -.6, 0, processed);
            blend(processed, result, filterMasks[2], filterStrengths[2]);
        }
        return result;
    }
};

struct DigitGuess {
    int digit = -1;
    std::vector<int> candidates;
};
class DigitReader {
    cv::Ptr<cv::ml::KNearest> classifier = cv::ml::KNearest::create();

public:
    static cv::Mat normalize(const cv::Mat& ink) {
        cv::Mat binary;
        cv::threshold(ink, binary, 35, 255, cv::THRESH_BINARY);
        std::vector<cv::Point> points;
        cv::findNonZero(binary, points);
        cv::Mat canvas = cv::Mat::zeros(20, 20, CV_8U);
        if (points.size() < 4)
            return canvas;
        auto bounds = cv::boundingRect(points);
        double scale = 16. / std::max(bounds.width, bounds.height);
        cv::Mat small;
        cv::resize(binary(bounds), small,
                   {std::max(1, int(std::round(bounds.width * scale))),
                    std::max(1, int(std::round(bounds.height * scale)))},
                   0, 0, cv::INTER_AREA);
        small.copyTo(
            canvas(cv::Rect((20 - small.cols) / 2, (20 - small.rows) / 2, small.cols, small.rows)));
        auto m = cv::moments(canvas);
        if (std::abs(m.mu02) > 1e-3) {
            double skew = std::clamp(m.mu11 / m.mu02, -.35, .35);
            cv::Mat transform = (cv::Mat_<double>(2, 3) << 1, skew, -10 * skew, 0, 1, 0);
            cv::warpAffine(canvas, canvas, transform, {20, 20},
                           cv::WARP_INVERSE_MAP | cv::INTER_LINEAR);
        }
        return canvas;
    }
    static cv::Mat feature(const cv::Mat& ink) {
        auto image = normalize(ink);
        cv::Mat gx, gy, magnitude, angle;
        cv::Sobel(image, gx, CV_32F, 1, 0);
        cv::Sobel(image, gy, CV_32F, 0, 1);
        cv::cartToPolar(gx, gy, magnitude, angle);
        cv::Mat histogram = cv::Mat::zeros(1, 128, CV_32F);
        for (int y = 0; y < 20; ++y)
            for (int x = 0; x < 20; ++x) {
                int bin = std::min(7, int(angle.at<float>(y, x) * 8 / (2 * CV_PI)));
                histogram.at<float>(0, ((y / 5) * 4 + x / 5) * 8 + bin) +=
                    magnitude.at<float>(y, x);
            }
        histogram /= cv::sum(histogram)[0] + 1e-6;
        cv::sqrt(histogram, histogram);
        return histogram;
    }
    void load(const std::string& path) {
        auto sheet = cv::imread(path, cv::IMREAD_GRAYSCALE);
        if (sheet.size() != cv::Size(2000, 1000))
            throw std::runtime_error("Missing digit samples");
        cv::Mat samples, labels;
        // All augmentations come only from training columns. The other half stays unseen.
        for (int row = 0; row < 50; ++row)
            for (int col = 0; col < 50; ++col) {
                auto digit = sheet(cv::Rect(col * 20, row * 20, 20, 20));
                samples.push_back(feature(digit));
                labels.push_back(float(row / 5));
                cv::Mat thick;
                cv::dilate(digit, thick, cv::getStructuringElement(cv::MORPH_ELLIPSE, {2, 2}));
                samples.push_back(feature(thick));
                labels.push_back(float(row / 5));
            }
        classifier->train(samples, cv::ml::ROW_SAMPLE, labels);
    }
    DigitGuess guess(const cv::Mat& ink) const {
        DigitGuess out;
        if (ink.empty() || cv::countNonZero(ink) < 30)
            return out;
        std::vector<cv::Point> points;
        cv::findNonZero(ink, points);
        if (cv::boundingRect(points).height < ink.rows * .18)
            return out;
        cv::Mat results, neighbors, distances;
        classifier->findNearest(feature(ink), 7, results, neighbors, distances);
        std::array<float, 10> score{};
        for (int i = 0; i < 7; ++i)
            score[int(neighbors.at<float>(0, i))] += 1.f / (.03f + distances.at<float>(0, i));
        std::array<int, 10> order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::stable_sort(order.begin(), order.end(),
                         [&](int a, int b) { return score[a] > score[b]; });
        out.digit = order[0];
        out.candidates.push_back(order[0]);
        if (score[order[1]] > score[order[0]] * .35)
            out.candidates.push_back(order[1]);
        return out;
    }
    int read(const cv::Mat& ink) const {
        return guess(ink).digit;
    }
};

// Discrete palm-position control. A held direction cannot repeat; returning to
// the center is required after every command or tracking loss.
class PalmGuide {
    bool armed = false;
    double centerDwellSeconds = 0, directionDwellSeconds = 0;
    Direction selectedDirection = Direction::None;

public:
    void reset() {
        armed = false;
        centerDwellSeconds = directionDwellSeconds = 0;
        selectedDirection = Direction::None;
    }
    bool ready() const {
        return armed;
    }
    float progress() const {
        return float(std::min(1., directionDwellSeconds / .42));
    }
    Direction direction() const {
        return selectedDirection;
    }
    static Direction zone(float x, float y) {
        x -= .5f;
        y -= .5f;
        if (std::abs(x) < .12f && std::abs(y) < .12f)
            return Direction::None;
        if (std::abs(x) > std::abs(y) * 1.35f && std::abs(x) > .16f)
            return x < 0 ? Direction::Left : Direction::Right;
        if (std::abs(y) > std::abs(x) * 1.35f && std::abs(y) > .16f)
            return y < 0 ? Direction::Up : Direction::Down;
        return Direction::None;
    }
    Direction update(float x, float y, bool visible, double dt,
                     const std::array<bool, 4>& allowed = {true, true, true, true}) {
        if (!visible || dt <= 0 || dt > .25) {
            reset();
            return Direction::None;
        }
        bool central = std::abs(x - .5f) < .12f && std::abs(y - .5f) < .12f;
        if (central) {
            centerDwellSeconds += dt;
            selectedDirection = Direction::None;
            directionDwellSeconds = 0;
            if (centerDwellSeconds >= .2)
                armed = true;
            return Direction::None;
        }
        centerDwellSeconds = 0;
        if (!armed)
            return Direction::None;
        auto next = zone(x, y);
        int index = int(next) - 1;
        if (next == Direction::None || !allowed[index]) {
            selectedDirection = Direction::None;
            directionDwellSeconds = 0;
            return Direction::None;
        }
        if (next != selectedDirection) {
            selectedDirection = next;
            directionDwellSeconds = 0;
        } else
            directionDwellSeconds += dt;
        if (directionDwellSeconds < .42)
            return Direction::None;
        armed = false;
        directionDwellSeconds = 0;
        selectedDirection = Direction::None;
        return next;
    }
};

struct Maze {
    // Follow only the chosen straight corridor. Stop before a turn or at a decision.
    const std::array<std::string, 9> tiles{{"#############", "#...#.......#", "#.#.#.#####.#",
                                            "#.#...#.....#", "#.#####.###.#", "#.....#...#.#",
                                            "###.#.###.#.#", "#.........#.#", "#############"}};
    cv::Point position{1, 7};
    const cv::Point home{11, 1};
    std::array<cv::Point, 2> decoys{{{1, 1}, {11, 7}}};
    void reset() {
        position = {1, 7};
    }
    bool road(cv::Point p) const {
        return p.y >= 0 && p.y < 9 && p.x >= 0 && p.x < 13 && tiles[p.y][p.x] == '.';
    }
    int exits(cv::Point p) const {
        int n = 0;
        for (auto d : directions)
            n += road(p + step(d));
        return n;
    }
    bool house(cv::Point p) const {
        return p == home || p == decoys[0] || p == decoys[1];
    }
    std::vector<cv::Point> route(cv::Point origin, Direction direction) const {
        const auto delta = step(direction);
        auto next = origin + delta;
        std::vector<cv::Point> path;
        if (direction == Direction::None || !road(next))
            return path;
        while (road(next)) {
            path.push_back(next);
            if (house(next) || exits(next) != 2 || !road(next + delta))
                break;
            next += delta;
        }
        return path;
    }
    cv::Point destination(cv::Point origin, Direction d) const {
        auto path = route(origin, d);
        return path.empty() ? origin : path.back();
    }
    bool move(Direction d) {
        auto target = destination(position, d);
        bool moved = target != position;
        position = target;
        return moved;
    }
    bool solved() const {
        return position == home;
    }
    std::vector<Direction> solution() const {
        std::queue<std::pair<cv::Point, std::vector<Direction>>> pending;
        std::array<bool, 117> visited{};
        pending.push({position, {}});
        while (!pending.empty()) {
            auto [p, path] = pending.front();
            pending.pop();
            if (p == home)
                return path;
            if (visited[p.y * 13 + p.x])
                continue;
            visited[p.y * 13 + p.x] = true;
            for (auto d : directions) {
                auto q = destination(p, d);
                if (q == p)
                    continue;
                auto next = path;
                next.push_back(d);
                pending.push({q, next});
            }
        }
        return {};
    }
};
} // namespace homeward
