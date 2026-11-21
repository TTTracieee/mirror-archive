#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

namespace mirror {
struct LevelData {
    cv::Mat observed;
    cv::Mat reference;
    cv::Rect answer; // Scoring annotation; never supplied to VisionEngine.
};

class LevelFactory {
public:
    static constexpr int width=960, height=600;

    void load(const std::string& artDirectory) {
        const std::array<std::string,3> names={"storeroom.png","maproom.png","greenhouse.png"};
        for(size_t i=0;i<scenes.size();++i){
            auto source=cv::imread(artDirectory+"/"+names[i]);
            if(source.empty())throw std::runtime_error("Missing scene: "+names[i]);
            // One art pixel per logical display pixel; CV works at twice that resolution.
            cv::Mat small;
            cv::resize(source,small,{width/2,height/2},0,0,cv::INTER_NEAREST);
            cv::resize(small,scenes[i],{width,height},0,0,cv::INTER_NEAREST);
        }
        auto source=cv::imread(artDirectory+"/moth.png",cv::IMREAD_UNCHANGED);
        if(source.empty()||source.channels()!=4)throw std::runtime_error("Missing RGBA moth sprite");
        cv::Mat small;
        cv::resize(source,small,{40,28},0,0,cv::INTER_NEAREST);
        cv::resize(small,mothSprite,{80,56},0,0,cv::INTER_NEAREST);
    }

    LevelData archive() const {
        LevelData data;
        data.answer={744,459,80,40};
        data.reference=scenes[0](cv::Rect(733,446,104,70)).clone();
        scenes[0].convertTo(data.observed,CV_8UC3,.37,65);
        cv::RNG random(923);
        for(int i=0;i<30000;++i){
            auto& pixel=data.observed.at<cv::Vec3b>(random.uniform(0,height),random.uniform(0,width));
            pixel=random.uniform(0,2)?cv::Vec3b(215,215,215):cv::Vec3b(42,42,42);
        }
        return data;
    }

    LevelData study() const {
        LevelData data;
        data.observed=scenes[1].clone();
        data.answer={138,392,144,114};
        auto seal=scenes[1](cv::Rect(138,390,146,116));
        // A separately oriented archive fragment requires geometric, not pixelwise matching.
        auto transform=cv::getRotationMatrix2D({73,58},14,1.08);
        transform.at<double>(0,2)+=18;transform.at<double>(1,2)+=20;
        cv::warpAffine(seal,data.reference,transform,{182,156},cv::INTER_NEAREST,
                       cv::BORDER_CONSTANT,cv::Scalar(155,216,245));
        return data;
    }

    static cv::Point mothCenter(int index,int frame) {
        constexpr std::array<int,6> xs={150,400,710,800,300,600};
        constexpr std::array<int,6> ys={116,180,108,268,340,405};
        const int direction=index==3?-1:1;
        const int x=70+((xs[index]-70+direction*frame*2)%820+820)%820;
        return {x,ys[index]+cvRound(12*std::sin(frame*.025+index))};
    }

    cv::Mat motionFrame(int frame) const {
        cv::Mat image=scenes[2].clone();
        for(int i=0;i<6;++i){
            const auto center=mothCenter(i,frame);
            for(int y=0;y<mothSprite.rows;++y)for(int x=0;x<mothSprite.cols;++x){
                auto source=mothSprite.at<cv::Vec4b>(y,x);
                auto& destination=image.at<cv::Vec3b>(center.y+y-28,center.x+x-40);
                const float alpha=source[3]/255.f;
                for(int c=0;c<3;++c)destination[c]=cv::saturate_cast<uchar>(source[c]*alpha+destination[c]*(1-alpha));
            }
        }
        return image;
    }

    static cv::Rect motionAnswer(int frame) {
        auto point=mothCenter(3,frame);return {point.x-40,point.y-28,80,56};
    }
    const cv::Mat& scene(int index) const {return scenes.at(index);}

private:
    std::array<cv::Mat,3> scenes;
    cv::Mat mothSprite;
};
} // namespace mirror
