#include "VisionEngine.h"
#include "GameModel.h"
#include "LevelFactory.h"
#include <iostream>
#include <stdexcept>

static int checks=0;
void require(bool condition,const std::string& message){
    ++checks;if(!condition)throw std::runtime_error(message);
}
int main(){
    try{
        using namespace mirror;
        cv::setNumThreads(2);cv::setRNGSeed(19);
        auto relics=LevelFactory::relics();
        auto restored=VisionEngine::restore(relics.observed);
        auto chroma=VisionEngine::chroma(relics.observed);
        auto rawEdges=VisionEngine::edges(relics.observed,false);
        auto cleanEdges=VisionEngine::edges(relics.observed,true);
        require(restored.size()==relics.observed.size(),"Restore preserves dimensions");
        cv::Mat grayRaw=VisionEngine::gray(rawEdges),grayClean=VisionEngine::gray(cleanEdges);
        require(cv::countNonZero(grayRaw>100)>cv::countNonZero(grayClean>100)*1.4,"Median preprocessing must remove noisy Canny edges");
        auto target=chroma.at<cv::Vec3b>(278-30,780);
        require(target[0]>60&&target[1]>60,"Turquoise object survives HSV segmentation");
        cv::Mat zero=cv::Mat::zeros(relics.observed.size(),CV_8U),full(relics.observed.size(),CV_8U,cv::Scalar(255));
        require(cv::norm(VisionEngine::blend(relics.observed,restored,zero),relics.observed,cv::NORM_INF)==0,"Zero mask returns observation");
        require(cv::norm(VisionEngine::blend(relics.observed,restored,full),restored,cv::NORM_INF)==0,"Full mask returns computed result");
        auto gallery=LevelFactory::gallery();
        auto match=VisionEngine::match(gallery.reference,gallery.observed,{320,282,260,260});
        std::cout<<"SIFT original: "<<match.ratioMatches<<" ratio matches / "<<match.inliers<<" inliers\n";
        require(match.verified,"Rotated/scaled original must verify with SIFT and RANSAC");
        for(auto center:std::vector<cv::Point>{{160,142},{450,142},{740,142},{160,412},{740,412}}){
            auto decoy=VisionEngine::match(gallery.reference,gallery.observed,{center.x-130,center.y-130,260,260});
            std::cout<<"Decoy: "<<decoy.ratioMatches<<" / "<<decoy.inliers<<"\n";
            require(!decoy.verified,"Shared star silhouette must not authenticate a different engraving");
        }
        cv::Mat blank(100,100,CV_8UC3,cv::Scalar(100,100,100));
        require(!VisionEngine::match(gallery.reference,blank,{0,0,100,100}).verified,"Textureless scan is handled");
        require(!VisionEngine::match(gallery.reference,gallery.observed,{-500,-500,20,20}).verified,"Out-of-image ROI is handled");
        auto before=LevelFactory::motionFrame(30),after=LevelFactory::motionFrame(31);
        auto flow=VisionEngine::flow(before,after);
        for(int i=0;i<6;++i){
            auto p=LevelFactory::mothCenter(i,30);
            const auto velocity=cv::mean(flow(cv::Rect(p.x-20,p.y-15,40,30)));
            std::cout<<"Moth "<<i<<" dx="<<velocity[0]<<"\n";
            require(i==3?velocity[0]<-1:velocity[0]>1,"Flow must recover actual motion direction from image pairs");
        }
        auto staticFlow=VisionEngine::flow(before,before);
        require(cv::norm(staticFlow,cv::NORM_INF)<.1,"Static sequence cannot invent movement");
        require(cv::norm(VisionEngine::difference(before,after),VisionEngine::difference(before,before),cv::NORM_L1)>10000,"Frame difference responds to movement");
        Session session;
        require(!session.ready(0)&&!session.ready(1)&&!session.ready(2),"Cases require evidence");
        session.brush(0,{100,100},50,135);float paid=session.energy;
        require(paid<100,"New area consumes energy");
        session.brush(0,{100,100},50,135);require(std::abs(session.energy-paid)<.001,"Revisiting a layer is free");
        session.brush(1,{100,100},50,135);require(session.energy<paid,"Different lenses have separate area budgets");
        session.energy=0;cv::Mat mask=session.masks[0].clone();
        require(!session.brush(0,{700,400},50,135),"Unaffordable exploration is rejected");
        require(cv::norm(mask,session.masks[0],cv::NORM_INF)==0,"Rejected exploration cannot reveal pixels");
        require(!session.spend(-1),"Negative costs cannot mint energy");
        session.evidence[1]=true;require(!session.ready(0),"Color alone cannot close the seal case");
        session.evidence[2]=true;require(session.ready(0),"Both independent clues unlock a claim");
        session.reset({900,560});require(!session.evidence[1]&&session.energy==100,"Restart clears evidence and energy");
        session.solved=true;require(session.stars()==3,"Efficient solution earns three seals");
        session.mistakes=1;require(session.stars()==2,"False claim affects rating");
        std::cout<<"PASS: "<<checks<<" checks\n";return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
