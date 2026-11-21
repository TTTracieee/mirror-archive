#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace mirror {
// A single gesture is either an exploration stroke or a choice, never both.
struct PointerGesture {
    cv::Point2f start;
    bool pressed=false,dragged=false;
    void begin(cv::Point2f point){start=point;pressed=true;dragged=false;}
    bool move(cv::Point2f point){
        if(pressed&&cv::norm(point-start)>=4)dragged=true;
        return pressed&&dragged;
    }
    bool release(cv::Point2f point){
        move(point);const bool choose=pressed&&!dragged;cancel();return choose;
    }
    void cancel(){pressed=dragged=false;}
};

struct Campaign {
    int chapter=0;
    bool solved=false,complete=false;
    void start(){chapter=0;solved=complete=false;}
    void solve(){solved=true;}
    bool advance(){
        if(!solved||complete)return false;
        if(chapter==2){complete=true;return true;}
        ++chapter;solved=false;return true;
    }
};

struct Session {
    bool paused=false;
    cv::Mat mask;
    void reset(cv::Size size){mask=cv::Mat::zeros(size,CV_8UC1);paused=false;}
    void brush(cv::Point center,int radius){
        if(radius>0&&!mask.empty())cv::circle(mask,center,radius,cv::Scalar(255),-1,cv::LINE_8);
    }
};
} // namespace mirror
