#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>

namespace homeward {
enum class Direction {None,Left,Right,Up,Down};
inline cv::Point step(Direction d){
    switch(d){case Direction::Left:return {-1,0};case Direction::Right:return {1,0};
        case Direction::Up:return {0,-1};case Direction::Down:return {0,1};default:return {};}
}
inline constexpr std::array<Direction,4> directions{Direction::Left,Direction::Right,Direction::Up,Direction::Down};

enum class Filter {Clean,Contrast,Detail};
class BrushRepair {
    std::array<cv::Mat,3> layers;
    std::vector<std::array<cv::Mat,3>> history;
public:
    cv::Mat damaged,filtered,mask;
    void load(const cv::Mat& source){
        cv::Mat faded;cv::GaussianBlur(source,faded,{3,3},.55);faded.convertTo(damaged,CV_8U,.48,64);
        cv::RNG random(208);
        for(size_t i=0;i<damaged.total()/5;++i){int x=random.uniform(0,damaged.cols),y=random.uniform(0,damaged.rows);
            uchar v=random.uniform(0,2)?235:18;damaged.at<cv::Vec3b>(y,x)={v,v,v};}
        cv::medianBlur(damaged,filtered,3);filtered.convertTo(filtered,CV_8U,1/.48,-64/.48);
        reset();
    }
    void reset(){for(auto& layer:layers)layer=cv::Mat::zeros(damaged.size(),CV_8U);mask=cv::Mat::zeros(damaged.size(),CV_8U);history.clear();}
    void beginStroke(){std::array<cv::Mat,3> snapshot;for(int i=0;i<3;++i)snapshot[i]=layers[i].clone();history.push_back(snapshot);if(history.size()>30)history.erase(history.begin());}
    bool undo(){if(history.empty())return false;layers=history.back();history.pop_back();mask=layers[0]|layers[1]|layers[2];return true;}
    void stroke(cv::Point from,cv::Point to,int radius=19,Filter filter=Filter::Clean){
        auto& layer=layers[int(filter)];cv::line(layer,from,to,255,radius*2,cv::LINE_8);cv::circle(layer,to,radius,255,cv::FILLED);
        mask=layers[0]|layers[1]|layers[2];
    }
    cv::Mat image()const{
        cv::Mat result=damaged.clone(),processed;
        cv::medianBlur(result,processed,3);processed.copyTo(result,layers[0]);
        result.convertTo(processed,CV_8U,1/.48,-64/.48);processed.copyTo(result,layers[1]);
        cv::Mat soft;cv::GaussianBlur(result,soft,{0,0},.8);cv::addWeighted(result,1.5,soft,-.5,0,processed);processed.copyTo(result,layers[2]);
        return result;
    }
};

struct DigitGuess {int digit=-1;std::vector<int> candidates;};
class DigitReader {
    cv::Ptr<cv::ml::KNearest> model=cv::ml::KNearest::create();
public:
    static cv::Mat normalize(const cv::Mat& ink){
        cv::Mat binary;cv::threshold(ink,binary,35,255,cv::THRESH_BINARY);
        std::vector<cv::Point> points;cv::findNonZero(binary,points);cv::Mat canvas=cv::Mat::zeros(20,20,CV_8U);
        if(points.size()<4)return canvas;
        auto bounds=cv::boundingRect(points);double scale=16./std::max(bounds.width,bounds.height);
        cv::Mat small;cv::resize(binary(bounds),small,{std::max(1,int(std::round(bounds.width*scale))),std::max(1,int(std::round(bounds.height*scale)))},0,0,cv::INTER_AREA);
        small.copyTo(canvas(cv::Rect((20-small.cols)/2,(20-small.rows)/2,small.cols,small.rows)));
        auto m=cv::moments(canvas);
        if(std::abs(m.mu02)>1e-3){double skew=std::clamp(m.mu11/m.mu02,-.35,.35);
            cv::Mat transform=(cv::Mat_<double>(2,3)<<1,skew,-10*skew,0,1,0);
            cv::warpAffine(canvas,canvas,transform,{20,20},cv::WARP_INVERSE_MAP|cv::INTER_LINEAR);}
        return canvas;
    }
    static cv::Mat feature(const cv::Mat& ink){
        auto image=normalize(ink);cv::Mat gx,gy,magnitude,angle;
        cv::Sobel(image,gx,CV_32F,1,0);cv::Sobel(image,gy,CV_32F,0,1);cv::cartToPolar(gx,gy,magnitude,angle);
        cv::Mat histogram=cv::Mat::zeros(1,128,CV_32F);
        for(int y=0;y<20;++y)for(int x=0;x<20;++x){int bin=std::min(7,int(angle.at<float>(y,x)*8/(2*CV_PI)));
            histogram.at<float>(0,((y/5)*4+x/5)*8+bin)+=magnitude.at<float>(y,x);}
        histogram/=cv::sum(histogram)[0]+1e-6;cv::sqrt(histogram,histogram);return histogram;
    }
    void load(const std::string& path){
        auto sheet=cv::imread(path,cv::IMREAD_GRAYSCALE);if(sheet.size()!=cv::Size(2000,1000))throw std::runtime_error("Missing digit samples");
        cv::Mat samples,labels;
        // All augmentations come only from training columns. The other half stays unseen.
        for(int row=0;row<50;++row)for(int col=0;col<50;++col){auto digit=sheet(cv::Rect(col*20,row*20,20,20));
            samples.push_back(feature(digit));labels.push_back(float(row/5));
            cv::Mat thick;cv::dilate(digit,thick,cv::getStructuringElement(cv::MORPH_ELLIPSE,{2,2}));
            samples.push_back(feature(thick));labels.push_back(float(row/5));}
        model->train(samples,cv::ml::ROW_SAMPLE,labels);
    }
    DigitGuess guess(const cv::Mat& ink)const{
        DigitGuess out;if(ink.empty()||cv::countNonZero(ink)<30)return out;
        std::vector<cv::Point> points;cv::findNonZero(ink,points);if(cv::boundingRect(points).height<ink.rows*.18)return out;
        cv::Mat results,neighbors,distances;model->findNearest(feature(ink),7,results,neighbors,distances);
        std::array<float,10> score{};
        for(int i=0;i<7;++i)score[int(neighbors.at<float>(0,i))]+=1.f/(.03f+distances.at<float>(0,i));
        std::array<int,10> order{0,1,2,3,4,5,6,7,8,9};std::stable_sort(order.begin(),order.end(),[&](int a,int b){return score[a]>score[b];});
        out.digit=order[0];out.candidates.push_back(order[0]);
        if(score[order[1]]>score[order[0]]*.35)out.candidates.push_back(order[1]);
        return out;
    }
    int read(const cv::Mat& ink)const{return guess(ink).digit;}
};

// Discrete palm-position control. A held direction cannot repeat; returning to
// the center is required after every command or tracking loss.
class PalmGuide {
    bool armed=false;double centerTime=0,dwell=0;Direction selected=Direction::None;
public:
    void reset(){armed=false;centerTime=dwell=0;selected=Direction::None;}
    bool ready()const{return armed;}
    float progress()const{return float(std::min(1.,dwell/.42));}
    Direction direction()const{return selected;}
    static Direction zone(float x,float y){
        x-=.5f;y-=.5f;if(std::abs(x)<.12f&&std::abs(y)<.12f)return Direction::None;
        if(std::abs(x)>std::abs(y)*1.35f&&std::abs(x)>.16f)return x<0?Direction::Left:Direction::Right;
        if(std::abs(y)>std::abs(x)*1.35f&&std::abs(y)>.16f)return y<0?Direction::Up:Direction::Down;
        return Direction::None;
    }
    Direction update(float x,float y,bool visible,double dt,const std::array<bool,4>& allowed={true,true,true,true}){
        if(!visible||dt<=0||dt>.25){reset();return Direction::None;}
        bool central=std::abs(x-.5f)<.12f&&std::abs(y-.5f)<.12f;
        if(central){centerTime+=dt;selected=Direction::None;dwell=0;if(centerTime>=.2)armed=true;return Direction::None;}
        centerTime=0;if(!armed)return Direction::None;
        auto next=zone(x,y);int index=int(next)-1;
        if(next==Direction::None||!allowed[index]){selected=Direction::None;dwell=0;return Direction::None;}
        if(next!=selected){selected=next;dwell=0;}else dwell+=dt;
        if(dwell<.42)return Direction::None;
        armed=false;dwell=0;selected=Direction::None;return next;
    }
};

struct Motion {
    cv::Point2f velocity{};
    float coherence=0;
    bool active=false;
};
// Measured on mirrored 320x240 camera frames. ROI excludes most face/background.
inline Motion measureFlow(const cv::Mat& previous,const cv::Mat& current,double dt){
    Motion out;
    if(previous.empty()||current.empty()||previous.size()!=current.size()||dt<=0||dt>.2)return out;
    cv::Mat field;cv::calcOpticalFlowFarneback(previous,current,field,.5,3,15,3,5,1.2,0);
    std::vector<float> xs,ys;float total=0;cv::Point2f sum{};
    for(int y=4;y<field.rows-4;y+=4)for(int x=4;x<field.cols-4;x+=4){
        auto v=field.at<cv::Point2f>(y,x);float speed=float(cv::norm(v));
        if(speed<.85||speed>22)continue;
        xs.push_back(v.x);ys.push_back(v.y);sum+=v;total+=speed;
    }
    if(xs.size()<24||total==0)return out;
    out.coherence=float(cv::norm(sum))/total;
    if(out.coherence<.72)return out;
    auto median=[](std::vector<float>& values){auto mid=values.begin()+values.size()/2;std::nth_element(values.begin(),mid,values.end());return *mid;};
    out.velocity={float(median(xs)/dt),float(median(ys)/dt)};out.active=true;return out;
}

class SwipeGate {
    bool armed=false;
    double quiet=0,cooldown=0,age=0;
    cv::Point2f travel{};
public:
    void reset(){armed=false;quiet=cooldown=age=0;travel={};}
    bool ready()const{return armed;}
    Direction update(const Motion& motion,double dt){
        if(dt<=0||dt>.2){reset();return Direction::None;}
        cooldown=std::max(0.,cooldown-dt);
        bool moving=motion.active&&cv::norm(motion.velocity)>25;
        if(!moving){
            quiet+=dt;travel={};age=0;
            if(quiet>=.45&&cooldown<=0)armed=true;
            return Direction::None;
        }
        quiet=0;if(!armed)return Direction::None;
        travel+=motion.velocity*float(dt);age+=dt;
        if(age>.35){travel={};age=0;return Direction::None;}
        if(age<.065||cv::norm(travel)<13)return Direction::None;
        Direction result=Direction::None;
        if(std::abs(travel.x)>1.65*std::abs(travel.y))result=travel.x>0?Direction::Right:Direction::Left;
        else if(std::abs(travel.y)>1.65*std::abs(travel.x))result=travel.y>0?Direction::Down:Direction::Up;
        if(result!=Direction::None){armed=false;cooldown=.85;travel={};age=0;}
        return result;
    }
};

struct Maze {
    // Two routes, a misleading house and a long returnable dead end. Movement
    // stops at a junction, dead end or house; no precision timing is needed.
    const std::array<std::string,9> tiles{{
        "#############", "#...#.......#", "#.#.#.#####.#",
        "#.#...#.....#", "#.#####.###.#", "#.....#...#.#",
        "###.#.###.#.#", "#.........#.#", "#############"}};
    cv::Point position{1,7};
    const cv::Point home{11,1};
    std::array<cv::Point,2> decoys{{{1,1},{11,7}}};
    void reset(){position={1,7};}
    bool road(cv::Point p)const{return p.y>=0&&p.y<9&&p.x>=0&&p.x<13&&tiles[p.y][p.x]=='.';}
    int exits(cv::Point p)const{int n=0;for(auto d:directions)n+=road(p+step(d));return n;}
    bool house(cv::Point p)const{return p==home||p==decoys[0]||p==decoys[1];}
    std::vector<cv::Point> route(cv::Point origin,Direction direction)const{
        auto next=origin+step(direction);std::vector<cv::Point> path;
        if(direction==Direction::None||!road(next))return path;
        auto previous=origin;
        for(int guard=0;guard<117;++guard){path.push_back(next);if(house(next)||exits(next)!=2)break;
            cv::Point after=next;for(auto d:directions){auto q=next+step(d);if(road(q)&&q!=previous){after=q;break;}}
            previous=next;next=after;
        }return path;
    }
    cv::Point destination(cv::Point origin,Direction d)const{auto path=route(origin,d);return path.empty()?origin:path.back();}
    bool move(Direction d){auto target=destination(position,d);bool moved=target!=position;position=target;return moved;}
    bool solved()const{return position==home;}
    std::vector<Direction> solution()const{
        std::queue<std::pair<cv::Point,std::vector<Direction>>> pending;
        std::array<bool,117> visited{};pending.push({position,{}});
        while(!pending.empty()){
            auto [p,path]=pending.front();pending.pop();
            if(p==home)return path;
            if(visited[p.y*13+p.x])continue;visited[p.y*13+p.x]=true;
            for(auto d:directions){auto q=destination(p,d);if(q==p)continue;auto next=path;next.push_back(d);pending.push({q,next});}
        }return {};
    }
};
}
