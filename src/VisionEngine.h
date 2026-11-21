#pragma once
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace mirror {

struct MatchEvidence {
    cv::Mat view;
    std::vector<cv::Point2f> polygon;
    int queryKeypoints = 0;
    int candidateKeypoints = 0;
    int ratioMatches = 0;
    int inliers = 0;
    bool verified = false;
};

// Vision only receives observed pixels. Answer regions live in LevelFactory.
class VisionEngine {
public:
    static cv::Mat gray(const cv::Mat& image) {
        cv::Mat result;
        if (image.channels() == 1) return image.clone();
        cv::cvtColor(image, result, cv::COLOR_BGR2GRAY);
        return result;
    }

    static cv::Mat restore(const cv::Mat& observed) {
        cv::Mat denoised, restored;
        cv::medianBlur(observed, denoised, 5);
        // Inverse of the known exposure window, not an inverse of lost detail.
        denoised.convertTo(restored, CV_8UC3, 2.7, -175.0);
        return restored;
    }

    static cv::Mat chroma(const cv::Mat& observed) {
        cv::Mat clean = restore(observed), hsv, selection, view;
        cv::cvtColor(clean, hsv, cv::COLOR_BGR2HSV);
        cv::inRange(hsv, cv::Scalar(77, 65, 35), cv::Scalar(103, 255, 255), selection);
        const auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, {5, 5});
        cv::morphologyEx(selection, selection, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(selection, selection, cv::MORPH_CLOSE, kernel);
        cv::cvtColor(gray(clean), view, cv::COLOR_GRAY2BGR);
        view.convertTo(view, -1, .24);
        clean.copyTo(view, selection);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(selection, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::drawContours(view, contours, -1, cv::Scalar(179, 224, 147), 2, cv::LINE_AA);
        return view;
    }

    static cv::Mat edges(const cv::Mat& observed, bool denoise = true) {
        cv::Mat input = denoise ? restore(observed) : observed.clone(), edge;
        cv::Canny(gray(input), edge, 45, 100);
        cv::Mat view(input.size(), CV_8UC3, cv::Scalar(39, 49, 39));
        view.setTo(cv::Scalar(187, 219, 211), edge);
        return view;
    }

    static cv::Mat keypoints(const cv::Mat& observed) {
        std::vector<cv::KeyPoint> points;
        cv::SIFT::create(900)->detect(gray(observed), points);
        cv::Mat view;
        cv::drawKeypoints(observed, points, view, cv::Scalar(99, 226, 218),
                         cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        return view;
    }

    static MatchEvidence match(const cv::Mat& reference, const cv::Mat& observed,
                               const cv::Rect& requestedRegion) {
        MatchEvidence result;
        result.view = observed.clone();
        const cv::Rect region = requestedRegion & cv::Rect(0, 0, observed.cols, observed.rows);
        if (reference.empty() || region.width < 24 || region.height < 24) return result;
        auto sift = cv::SIFT::create(1500);
        std::vector<cv::KeyPoint> a, b;
        cv::Mat da, db;
        sift->detectAndCompute(gray(reference), cv::noArray(), a, da);
        sift->detectAndCompute(gray(observed(region)), cv::noArray(), b, db);
        result.queryKeypoints = static_cast<int>(a.size());
        result.candidateKeypoints = static_cast<int>(b.size());
        if (da.empty() || db.rows < 2) return result;
        std::vector<std::vector<cv::DMatch>> nearest;
        cv::BFMatcher(cv::NORM_L2).knnMatch(da, db, nearest, 2);
        std::vector<cv::Point2f> source, destination;
        for (const auto& pair : nearest) {
            if (pair.size() == 2 && pair[0].distance < .72f * pair[1].distance) {
                source.push_back(a[pair[0].queryIdx].pt);
                destination.push_back(b[pair[0].trainIdx].pt + cv::Point2f(region.x, region.y));
            }
        }
        result.ratioMatches = static_cast<int>(source.size());
        if (source.size() < 6) return result;
        cv::Mat inlierMask;
        auto homography = cv::findHomography(source, destination, cv::RANSAC, 3.0, inlierMask);
        if (homography.empty()) return result;
        result.inliers = cv::countNonZero(inlierMask);
        for (size_t i = 0; i < destination.size(); ++i) {
            if (inlierMask.at<uchar>(static_cast<int>(i)))
                cv::circle(result.view, destination[i], 3, cv::Scalar(155, 238, 175), -1, cv::LINE_AA);
        }
        std::vector<cv::Point2f> corners = {{0,0}, {float(reference.cols-1),0},
            {float(reference.cols-1),float(reference.rows-1)}, {0,float(reference.rows-1)}};
        cv::perspectiveTransform(corners, result.polygon, homography);
        std::vector<cv::Point> integerCorners;
        bool finite = true;
        for (const auto& p : result.polygon) {
            finite &= std::isfinite(p.x) && std::isfinite(p.y) && p.x >= -observed.cols
                && p.x <= observed.cols * 2 && p.y >= -observed.rows && p.y <= observed.rows * 2;
            if (finite) integerCorners.emplace_back(cvRound(p.x), cvRound(p.y));
        }
        double area = finite ? std::abs(cv::contourArea(integerCorners)) : 0;
        result.verified = finite && result.inliers >= 8 && result.inliers >= source.size() * .45
            && cv::isContourConvex(integerCorners) && area > 900 && area < observed.total() * .6;
        if (result.verified)
            cv::polylines(result.view, integerCorners, true, cv::Scalar(155, 238, 175), 3, cv::LINE_AA);
        return result;
    }

    static cv::Mat difference(const cv::Mat& previous, const cv::Mat& current) {
        cv::Mat difference, threshold, view(current.size(), CV_8UC3, cv::Scalar(36, 45, 36));
        cv::absdiff(gray(previous), gray(current), difference);
        cv::threshold(difference, threshold, 9, 255, cv::THRESH_BINARY);
        cv::morphologyEx(threshold, threshold, cv::MORPH_CLOSE,
                        cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3}));
        view.setTo(cv::Scalar(137, 203, 223), threshold);
        return view;
    }

    static cv::Mat flow(const cv::Mat& previous, const cv::Mat& current) {
        cv::Mat result;
        cv::calcOpticalFlowFarneback(gray(previous), gray(current), result, .5, 3, 19, 3, 5, 1.2, 0);
        return result;
    }

    static cv::Mat flowView(const cv::Mat& observed, const cv::Mat& flow, bool leftOnly) {
        cv::Mat view;
        observed.convertTo(view, -1, .47);
        for (int y = 12; y < flow.rows - 12; y += 12) {
            for (int x = 12; x < flow.cols - 12; x += 12) {
                const auto velocity = flow.at<cv::Point2f>(y, x);
                if (cv::norm(velocity) < .55 || (leftOnly && velocity.x > -.6)) continue;
                const cv::Scalar color = velocity.x < 0 ? cv::Scalar(123, 218, 245) : cv::Scalar(191, 190, 114);
                cv::arrowedLine(view, {x, y}, cv::Point(cvRound(x + velocity.x * 4), cvRound(y + velocity.y * 4)),
                                color, 1, cv::LINE_AA, 0, .35);
            }
        }
        return view;
    }

    static cv::Mat blend(const cv::Mat& observed, const cv::Mat& processed, const cv::Mat& mask) {
        CV_Assert(observed.size() == processed.size() && mask.size() == observed.size());
        cv::Mat a, b, alpha, alpha3, result;
        observed.convertTo(a, CV_32FC3); processed.convertTo(b, CV_32FC3);
        mask.convertTo(alpha, CV_32F, 1.0 / 255);
        cv::cvtColor(alpha, alpha3, cv::COLOR_GRAY2BGR);
        cv::Mat composite = a.mul(cv::Scalar::all(1) - alpha3) + b.mul(alpha3);
        composite.convertTo(result, CV_8UC3);
        return result;
    }
};
} // namespace mirror
