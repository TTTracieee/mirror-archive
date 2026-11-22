#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <cmath>
#include <stdexcept>

namespace mirror {
struct Restoration {
    float clean=0,clarity=0;
    cv::Mat apply(const cv::Mat& damaged) const {
        cv::Mat median,denoised,expanded,soft,result;
        cv::medianBlur(damaged,median,3);
        cv::addWeighted(damaged,1-clean,median,clean,0,denoised);
        const double gain=1+2.1*clarity;
        denoised.convertTo(expanded,CV_32F,gain,128*(1-gain));
        cv::GaussianBlur(expanded,soft,{0,0},1.0);
        cv::addWeighted(expanded,1+.8*clarity,soft,-.8*clarity,0,result);
        result.convertTo(result,CV_8U);return result;
    }
};

struct CombinationLock {
    std::array<int,3> digits{1,0,7};
    static constexpr std::array<int,3> code{3,1,4};
    void turn(int wheel,int steps){
        if(wheel<0||wheel>2)return;
        digits[wheel]=((digits[wheel]+steps)%10+10)%10;
        if(wheel<2)digits[wheel+1]=((digits[wheel+1]+steps)%10+10)%10;
    }
    bool solved()const{return digits==code;}
};

struct Registration {
    cv::Mat transform;
    std::vector<cv::Point2f> from,to;
    int inliers=0;
    bool valid()const{return !transform.empty();}
};

// Estimate only from two scans. Neither crop coordinates nor a full reference
// photograph enters the matcher or the snap decision.
inline Registration registerScans(const cv::Mat& source,const cv::Mat& target){
    Registration out;cv::Mat a,b,da,db;
    cv::cvtColor(source,a,cv::COLOR_BGR2GRAY);cv::cvtColor(target,b,cv::COLOR_BGR2GRAY);
    std::vector<cv::KeyPoint> ka,kb;
    auto sift=cv::SIFT::create(1600);
    sift->detectAndCompute(a,cv::noArray(),ka,da);
    sift->detectAndCompute(b,cv::noArray(),kb,db);
    if(da.empty()||db.rows<2)return out;
    std::vector<std::vector<cv::DMatch>> nearest;
    cv::BFMatcher(cv::NORM_L2).knnMatch(da,db,nearest,2);
    std::vector<cv::Point2f> pa,pb;
    for(const auto& pair:nearest)if(pair.size()==2&&pair[0].distance<.7*pair[1].distance){
        pa.push_back(ka[pair[0].queryIdx].pt);pb.push_back(kb[pair[0].trainIdx].pt);
    }
    if(pa.size()<10)return out;
    cv::Mat mask;
    auto affine=cv::estimateAffinePartial2D(pa,pb,mask,cv::RANSAC,2,2000,.995);
    if(affine.empty()||!cv::checkRange(affine))return out;
    out.inliers=cv::countNonZero(mask);
    double scale=std::hypot(affine.at<double>(0,0),affine.at<double>(1,0));
    if(out.inliers<10||out.inliers<pa.size()*.55||scale<.7||scale>1.4)return out;
    out.transform=cv::Mat::eye(3,3,CV_64F);affine.copyTo(out.transform(cv::Rect(0,0,3,2)));
    for(size_t i=0;i<pa.size();++i)if(mask.at<uchar>(int(i))){out.from.push_back(pa[i]);out.to.push_back(pb[i]);}
    return out;
}
inline cv::Point2f mapped(const cv::Mat& m,cv::Point2f p){
    return {float(m.at<double>(0,0)*p.x+m.at<double>(0,1)*p.y+m.at<double>(0,2)),
            float(m.at<double>(1,0)*p.x+m.at<double>(1,1)*p.y+m.at<double>(1,2))};
}
struct PhotoPiece {
    cv::Mat image,pose;
    cv::Point2f center;
    float angle=0,scale=.5f;
    bool fixed=false;
    cv::Mat currentPose()const{
        const double r=angle*CV_PI/180,c=std::cos(r)*scale,s=std::sin(r)*scale;
        return (cv::Mat_<double>(3,3)<<c,-s,center.x-c*image.cols/2+s*image.rows/2,
            s,c,center.y-s*image.cols/2-c*image.rows/2,0,0,1);
    }
    bool contains(cv::Point2f point)const{
        auto local=mapped(currentPose().inv(),point);
        return local.x>=0&&local.y>=0&&local.x<image.cols&&local.y<image.rows;
    }
};
struct PhotoPuzzle {
    std::array<PhotoPiece,5> pieces;
    std::array<std::array<Registration,5>,5> matches;
    std::vector<cv::Point2f> sparkle;
    void load(const cv::Mat& photograph){
        cv::Mat photo;cv::resize(photograph,photo,{640,426},0,0,cv::INTER_NEAREST);
        const std::array<cv::Rect,5> crops{{{180,100,280,240},{0,0,360,240},{280,0,360,240},{0,186,360,240},{280,186,360,240}}};
        const std::array<cv::Point2f,5> starts{{{242,178},{112,122},{393,117},{102,226},{398,226}}};
        const std::array<float,5> angles{0,180,0,90,270};
        for(int i=0;i<5;++i){pieces[i]={};pieces[i].image=photo(crops[i]).clone();pieces[i].center=starts[i];pieces[i].angle=angles[i];}
        pieces[0].fixed=true;pieces[0].pose=pieces[0].currentPose();
        for(int i=0;i<5;++i)for(int j=0;j<5;++j)if(i!=j)matches[i][j]=registerScans(pieces[i].image,pieces[j].image);
        sparkle.clear();
    }
    cv::Mat expectedPose(int index)const{
        for(int j=0;j<5;++j)if(j!=index&&pieces[j].fixed&&matches[index][j].valid())
            return pieces[j].pose*matches[index][j].transform;
        return {};
    }
    bool snap(int index){
        if(index<1||index>=5||pieces[index].fixed)return false;
        auto& p=pieces[index];auto expected=expectedPose(index);if(expected.empty())return false;
        auto current=p.currentPose();double error=0;
        for(auto corner:std::vector<cv::Point2f>{{0,0},{float(p.image.cols),0},{0,float(p.image.rows)},{float(p.image.cols),float(p.image.rows)}})
            error=std::max(error,cv::norm(mapped(expected,corner)-mapped(current,corner)));
        if(error>23)return false;
        p.pose=expected;p.center=mapped(expected,{p.image.cols/2.f,p.image.rows/2.f});
        p.angle=float(std::atan2(expected.at<double>(1,0),expected.at<double>(0,0))*180/CV_PI);
        p.scale=float(std::hypot(expected.at<double>(0,0),expected.at<double>(1,0)));p.fixed=true;
        sparkle.clear();
        for(int j=0;j<5;++j)if(j!=index&&pieces[j].fixed&&matches[index][j].valid()){
            for(size_t k=0;k<matches[index][j].from.size()&&k<12;++k)sparkle.push_back(mapped(expected,matches[index][j].from[k]));break;
        }
        return true;
    }
    int count()const{int n=0;for(const auto& p:pieces)n+=p.fixed;return n;}
    // Feather overlapping warped observations. The final exhibit is assembled
    // from the registered scans; it never substitutes the source photograph.
    cv::Mat composite(cv::Size size={480,300})const{
        cv::Mat sum=cv::Mat::zeros(size,CV_32FC3),weights=cv::Mat::zeros(size,CV_32F);
        for(const auto& p:pieces)if(p.fixed){
            cv::Mat weight(p.image.size(),CV_32F);
            for(int y=0;y<weight.rows;++y)for(int x=0;x<weight.cols;++x)
                weight.at<float>(y,x)=std::min(1.f,(1+std::min({x,y,weight.cols-1-x,weight.rows-1-y}))/16.f);
            cv::Mat f,channels[3]={weight,weight,weight},w3,warped,ww;
            cv::merge(channels,3,w3);p.image.convertTo(f,CV_32F);cv::multiply(f,w3,f);
            cv::warpAffine(f,warped,p.pose(cv::Rect(0,0,3,2)),size,cv::INTER_NEAREST);
            cv::warpAffine(weight,ww,p.pose(cv::Rect(0,0,3,2)),size,cv::INTER_NEAREST);
            sum+=warped;weights+=ww;
        }
        cv::max(weights,1e-5,weights);cv::Mat ws[3]={weights,weights,weights},w3,result;
        cv::merge(ws,3,w3);cv::divide(sum,w3,result);result.convertTo(result,CV_8UC3);return result;
    }
};
}
