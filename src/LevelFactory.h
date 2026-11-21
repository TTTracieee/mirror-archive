#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <cmath>
#include <vector>

namespace mirror {
struct LevelData {
    cv::Mat observed;
    cv::Mat reference;
    cv::Rect answer;
};

class LevelFactory {
public:
    static constexpr int width = 900, height = 560;
    static cv::Mat pixelArt(const cv::Mat& source) {
        cv::Mat small, result;
        cv::resize(source, small, {source.cols/2,source.rows/2}, 0, 0, cv::INTER_NEAREST);
        cv::resize(small, result, source.size(), 0, 0, cv::INTER_NEAREST);
        return result;
    }
    static cv::Mat paper(unsigned seed = 17, bool dark = false) {
        cv::Mat image(height, width, CV_8UC3);
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            int grain = ((x/8+y/8+seed)%5==0)?3:0;
            auto& pixel = image.at<cv::Vec3b>(y, x);
            pixel = dark ? cv::Vec3b(53+grain, 42+grain, 35+grain)
                         : cv::Vec3b(160+grain, 197+grain, 228+grain);
        }
        for (int x = 60; x < width; x += 60)
            cv::line(image, {x, 20}, {x, height-20}, dark ? cv::Scalar(56,45,38) : cv::Scalar(157,194,224), 2);
        cv::rectangle(image, {18,18}, {width-19,height-19}, dark ? cv::Scalar(97,80,63) : cv::Scalar(96,138,176), 2);
        return image;
    }

    static std::vector<cv::Point> star(cv::Point center, int outer, int inner, int points, float angle = -.5f*CV_PI) {
        std::vector<cv::Point> polygon;
        for (int i=0; i<points*2; ++i) {
            float a = angle + float(i)*CV_PI/points;
            int r = i%2 ? inner : outer;
            polygon.emplace_back(center.x+cvRound(std::cos(a)*r), center.y+cvRound(std::sin(a)*r));
        }
        return polygon;
    }

    static LevelData relics() {
        auto image = paper();
        const std::array<cv::Scalar, 4> inks = {cv::Scalar(157,155,38), cv::Scalar(56,98,163),
                                              cv::Scalar(71,151,153), cv::Scalar(137,87,131)};
        // Multiple turquoise and multiple seven-point seals: neither cue alone solves it.
        const std::array<int, 12> hues = {0,1,2,3, 2,0,1,0, 3,2,0,1};
        const std::array<int, 12> points = {5,7,6,5, 8,6,7,7, 6,5,8,6};
        LevelData level;
        for (int i=0; i<12; ++i) {
            cv::Point center(120+(i%4)*220, 104+(i/4)*174);
            cv::circle(image, center+cv::Point(3,4), 64, cv::Scalar(147,174,187), -1, cv::LINE_AA);
            cv::circle(image, center, 63, cv::Scalar(105,157,184), -1, cv::LINE_AA);
            cv::circle(image, center, 58, cv::Scalar(174,204,217), -1, cv::LINE_AA);
            cv::circle(image, center, 53, cv::Scalar(113,160,180), 1, cv::LINE_AA);
            auto shape = star(center, 47, 23, points[i]);
            cv::fillPoly(image, std::vector<std::vector<cv::Point>>{shape}, inks[hues[i]], cv::LINE_AA);
            cv::polylines(image, shape, true, cv::Scalar(74,105,108), 1, cv::LINE_AA);
            cv::circle(image, center, 13, cv::Scalar(164,197,215), -1, cv::LINE_AA);
            cv::circle(image, center, 8, inks[hues[i]], -1, cv::LINE_AA);
            for (int j=0; j<24; ++j) {
                double a = j*CV_PI/12;
                cv::circle(image, center+cv::Point(cvRound(cos(a)*58), cvRound(sin(a)*58)), 1, cv::Scalar(85,129,157), -1);
            }
            if (i==7) level.answer = {center.x-51,center.y-51,102,102};
        }
        pixelArt(image).convertTo(level.observed, CV_8UC3, .37, 65);
        cv::RNG random(923);
        for (int i=0; i<26000; ++i) {
            auto& pixel=level.observed.at<cv::Vec3b>(random.uniform(0,height),random.uniform(0,width));
            pixel = random.uniform(0,2) ? cv::Vec3b(215,215,215) : cv::Vec3b(42,42,42);
        }
        return level;
    }

    static cv::Mat engraving(int seed) {
        const int size=190;
        cv::Mat image(size,size,CV_8UC3,cv::Scalar(176,207,222));
        cv::RNG random(seed*71+43);
        cv::rectangle(image,{6,6},{size-7,size-7},cv::Scalar(60,96,94),2);
        cv::rectangle(image,{11,11},{size-12,size-12},cv::Scalar(112,141,144),1);
        // Seeded etched details create stable, non-repeating local descriptors.
        for(int i=0;i<100;++i){
            cv::Point p(random.uniform(16,174),random.uniform(16,174));
            int radius=random.uniform(2,7);
            cv::Scalar ink(random.uniform(48,125),random.uniform(66,141),random.uniform(61,140));
            if(i%3==0)cv::rectangle(image,cv::Rect(p.x,p.y,radius,radius),ink,-1);
            else cv::circle(image,p,radius,ink,i%2?-1:1,cv::LINE_AA);
        }
        auto central=star({95,95},51,22,7);
        cv::fillPoly(image,std::vector<std::vector<cv::Point>>{central},cv::Scalar(95,125,137),cv::LINE_AA);
        cv::polylines(image,central,true,cv::Scalar(38,75,71),2,cv::LINE_AA);
        cv::circle(image,{95,95},17,cv::Scalar(174,206,218),-1,cv::LINE_AA);
        for(int i=0;i<12;++i){
            double a=i*CV_PI/6;
            cv::line(image,{95,95},{95+cvRound(cos(a)*13),95+cvRound(sin(a)*13)},cv::Scalar(45,91,90),1,cv::LINE_AA);
        }
        return image;
    }

    static LevelData gallery() {
        LevelData level; level.observed=paper(54); level.reference=engraving(8);
        const std::array<double,6> angles={-13,9,-5,16,-19,7};
        for(int i=0;i<6;++i){
            cv::Point center(160+(i%3)*290,142+(i/3)*270);
            cv::Mat tile=engraving(i==4?8:20+i), rotated, alpha;
            const auto transform=cv::getRotationMatrix2D({95,95},angles[i],i==4?.91:1.0);
            cv::warpAffine(tile,rotated,transform,{230,230},cv::INTER_LINEAR,cv::BORDER_CONSTANT,{0,0,0});
            cv::Mat mask(190,190,CV_8UC1,cv::Scalar(255));
            cv::warpAffine(mask,alpha,transform,{230,230});
            cv::Rect region(center.x-95,center.y-95,230,230);
            rotated.copyTo(level.observed(region),alpha);
            if(i==4)level.answer={center.x-65,center.y-65,130,130};
        }
        level.observed=pixelArt(level.observed);
        return level;
    }

    static cv::Point mothCenter(int index, int frame) {
        int x;
        if(index==3)x=760-(frame*2+index*93)%650;
        else x=110+(frame*2+index*113)%650;
        return {x,65+index*85};
    }
    static cv::Mat moth(int seed) {
        cv::Mat tile(64,94,CV_8UC3,cv::Scalar(53,42,35));
        cv::Scalar wing(137,162,148);
        cv::ellipse(tile,{29,27},{21,17},-32,0,360,wing,-1,cv::LINE_AA);
        cv::ellipse(tile,{65,27},{21,17},32,0,360,wing,-1,cv::LINE_AA);
        cv::ellipse(tile,{32,43},{15,13},25,0,360,wing,-1,cv::LINE_AA);
        cv::ellipse(tile,{62,43},{15,13},-25,0,360,wing,-1,cv::LINE_AA);
        cv::RNG random(seed+161);
        for(int i=0;i<75;++i){
            int x=random.uniform(10,84),y=random.uniform(10,54);
            if(tile.at<cv::Vec3b>(y,x)[0]>100)cv::circle(tile,{x,y},random.uniform(1,3),cv::Scalar(61,86,73),-1,cv::LINE_AA);
        }
        cv::ellipse(tile,{47,33},{4,24},0,0,360,cv::Scalar(201,207,173),-1,cv::LINE_AA);
        cv::line(tile,{46,14},{36,5},cv::Scalar(167,191,171),1,cv::LINE_AA);
        cv::line(tile,{48,14},{58,5},cv::Scalar(167,191,171),1,cv::LINE_AA);
        return pixelArt(tile);
    }
    static cv::Mat motionFrame(int frame) {
        cv::Mat image=paper(31,true);
        static const cv::Mat specimen=moth(0);
        for(int i=0;i<6;++i){
            cv::line(image,{38,65+i*85},{862,65+i*85},cv::Scalar(66,65,55),2);
            auto center=mothCenter(i,frame);
            specimen.copyTo(image(cv::Rect(center.x-47,center.y-32,94,64)));
        }
        return image;
    }
    static cv::Rect motionAnswer(int frame) {
        auto p=mothCenter(3,frame);return {p.x-48,p.y-34,96,68};
    }
};
} // namespace mirror
