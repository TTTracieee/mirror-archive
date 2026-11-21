#include "VisionEngine.h"
#include "GameModel.h"
#include "LevelFactory.h"
#include "PixelGrid.h"
#include <iostream>
#include <stdexcept>

static int checks=0;
void require(bool condition,const std::string& message){
    ++checks;if(!condition)throw std::runtime_error(message);
}
int main(int argc,char** argv){
    try{
        using namespace mirror;
        auto retina=pixelViewport(2560,1536);
        require(retina.scale==4&&retina.x==0&&retina.y==0,"Retina uses exact 4x pixels");
        auto standard=pixelViewport(1280,768);
        require(standard.scale==2,"Standard display uses exact 2x pixels");
        auto resized=pixelViewport(2010,1300);
        require(resized.scale==3&&resized.x==45&&resized.y==74,"Integer scaling and letterboxing");
        require(resized.localX(45+143*3)==143&&resized.localY(74+211*3)==211,"Pointer transform reverses letterboxing");
        require(resized.localX(0)<0,"Margin clicks stay outside the board");
        cv::setNumThreads(2);cv::setRNGSeed(19);
        LevelFactory factory;
        factory.load(argc>1?argv[1]:"bin/data/art");
        auto archive=factory.archive();
        auto restored=VisionEngine::restore(archive.observed);
        require(restored.size()==archive.observed.size(),"Restore preserves dimensions");
        double damagedError=cv::norm(archive.observed,factory.scene(0),cv::NORM_L1);
        double repairedError=cv::norm(restored,factory.scene(0),cv::NORM_L1);
        std::cout<<"Restoration error: "<<damagedError<<" -> "<<repairedError<<"\n";
        require(repairedError<damagedError*.5,"Restoration must improve observed pixels, not swap in original art");
        auto repaired=VisionEngine::repairLens(archive.observed);
        cv::Mat greenEdges;
        cv::inRange(repaired,cv::Scalar(162,235,185),cv::Scalar(162,235,185),greenEdges);
        require(cv::countNonZero(greenEdges(archive.answer))>15,"HSV/morphology/Canny must find detail on the teal key");
        require(cv::countNonZero(greenEdges)>cv::countNonZero(greenEdges(archive.answer))*2,"Teal distractors also produce real responses");
        cv::Mat zero=cv::Mat::zeros(archive.observed.size(),CV_8U);
        cv::Mat full(archive.observed.size(),CV_8U,cv::Scalar(255));
        require(cv::norm(VisionEngine::blend(archive.observed,repaired,zero),archive.observed,cv::NORM_INF)==0,"Zero mask preserves observation");
        require(cv::norm(VisionEngine::blend(archive.observed,repaired,full),repaired,cv::NORM_INF)==0,"Full mask reveals computed result");

        auto study=factory.study();
        auto match=VisionEngine::match(study.reference,study.observed,{60,319,300,260});
        std::cout<<"Lighthouse seal: "<<match.ratioMatches<<" ratio matches / "<<match.inliers<<" inliers\n";
        require(match.verified,"Rotated/scaled archive fragment must verify against the scene");
        for(auto center:std::vector<cv::Point>{{400,294},{547,389},{752,330},{820,125}}){
            auto decoy=VisionEngine::match(study.reference,study.observed,{center.x-130,center.y-115,260,230});
            std::cout<<"Decoy: "<<decoy.ratioMatches<<" / "<<decoy.inliers<<"\n";
            require(!decoy.verified,"Different nautical seals and background must not authenticate");
        }
        cv::Mat blank(100,100,CV_8UC3,cv::Scalar(100,100,100));
        require(!VisionEngine::match(study.reference,blank,{0,0,100,100}).verified,"Textureless scan is handled");
        require(!VisionEngine::match(study.reference,study.observed,{-500,-500,20,20}).verified,"Out-of-image ROI is handled");

        auto before=factory.motionFrame(30),after=factory.motionFrame(31);
        auto flow=VisionEngine::flow(before,after);
        for(int i=0;i<6;++i){
            auto point=LevelFactory::mothCenter(i,31);
            auto velocity=cv::mean(flow(cv::Rect(point.x-20,point.y-14,40,28)));
            std::cout<<"Moth "<<i<<" dx="<<velocity[0]<<"\n";
            require(i==3?velocity[0]<-1:velocity[0]>1,"Flow must recover direction from composited scene frames");
            require(VisionEngine::movingLeft(flow,point)==(i==3),"Measured flow must validate only the left-moving moth");
        }
        auto staticFlow=VisionEngine::flow(before,before);
        require(cv::norm(staticFlow,cv::NORM_INF)<.1,"Static sequence cannot invent movement");
        require(!VisionEngine::movingLeft(staticFlow,LevelFactory::mothCenter(3,30)),"A still frame cannot pass motion validation");
        require(cv::norm(VisionEngine::motionLens(before,after,flow),after,cv::NORM_L1)>10000,"Real moving regions receive flow arrows");
        require(cv::norm(VisionEngine::motionLens(before,before,staticFlow),before,cv::NORM_INF)==0,"Static regions receive no fabricated arrows");
        for(int frame:{0,30,410,9999})for(int i=0;i<6;++i){
            auto p=LevelFactory::mothCenter(i,frame);
            if(p.x<40||p.x+40>960||p.y<28||p.y+28>600)throw std::runtime_error("Sprite leaves safe compositing bounds");
        }
        require(true,"Motion remains in bounds across wrapping");

        Session session;session.reset({960,600});session.brush({0,0},90);
        require(cv::countNonZero(session.mask)>100,"Brush clips safely at the image edge");
        auto mask=session.mask.clone();session.brush({0,0},90);
        require(cv::norm(mask,session.mask,cv::NORM_INF)==0,"Revisiting is idempotent");
        session.brush({800,450},90);
        require(cv::countNonZero(session.mask)>cv::countNonZero(mask),"Exploration is unlimited");
        session.paused=true;session.reset({960,600});
        require(!session.paused&&cv::countNonZero(session.mask)==0,"Chapter reset clears observations and pause");

        PointerGesture gesture;
        gesture.begin({10,10});gesture.move({11,10});
        require(gesture.release({11,11}),"Small pointer jitter still chooses");
        gesture.begin({10,10});require(gesture.move({16,10}),"Dragging begins beyond threshold");
        require(!gesture.release({10,10}),"Returning a stroke to its origin must never choose");
        gesture.begin({10,10});require(!gesture.release({30,10}),"Release displacement cannot masquerade as a click");
        require(!gesture.release({10,10}),"Stray release cannot choose");
        gesture.begin({10,10});gesture.cancel();require(!gesture.pressed,"Navigation cancels a pending gesture");

        Campaign campaign;campaign.start();
        require(!campaign.advance()&&campaign.chapter==0,"Cannot skip an unsolved chapter");
        campaign.solve();require(campaign.advance()&&campaign.chapter==1&&!campaign.solved,"Key leads directly to the study");
        campaign.solve();require(campaign.advance()&&campaign.chapter==2,"Map leads directly to the film");
        campaign.solve();require(campaign.advance()&&campaign.complete,"Third repair completes the story");
        require(!campaign.advance()&&campaign.chapter==2,"Completion cannot advance out of bounds");
        campaign.start();require(campaign.chapter==0&&!campaign.complete&&!campaign.solved,"Start always begins the whole story");
        std::cout<<"PASS: "<<checks<<" checks\n";return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
