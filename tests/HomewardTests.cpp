#include "HomewardGame.h"
#include "StoryScript.h"
#include <iostream>
#include <set>
#include <sstream>
static int checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
    using namespace homeward;cv::setNumThreads(2);
    const std::string data=argc>1?argv[1]:"bin/data";
    cv::Mat photo=cv::imread(data+"/art/family-home.png"),small;
    check(!photo.empty(),"Family photograph exists");cv::resize(photo,small,{480,320});
    BrushRepair repair;repair.load(small);
    check(cv::norm(repair.image(),repair.damaged,cv::NORM_INF)==0,"Unbrushed photo is genuinely damaged");
    BrushRepair full;full.load(small);
    for(auto tool:{Filter::Clean,Filter::Contrast,Filter::Detail})full.stroke({0,160},{480,160},200,tool);
    check(cv::norm(full.image(),small,cv::NORM_L1)<cv::norm(full.damaged,small,cv::NORM_L1),"Observation-based filtering improves damaged pixels");
    repair.stroke({0,0},{200,0});
    check(cv::countNonZero(repair.mask)>100,"Brush is clipped safely at photograph edge");
    check(cv::norm(repair.image()(cv::Rect(300,200,50,50)),repair.damaged(cv::Rect(300,200,50,50)),cv::NORM_INF)==0,"Brush leaves remote pixels untouched");
    repair.beginStroke();auto before=repair.image().clone();
    repair.stroke({300,200},{350,200},12,Filter::Contrast);
    check(cv::norm(repair.image(),before,cv::NORM_INF)>0,"Contrast tool changes selected pixels");
    repair.undo();check(cv::norm(repair.image(),before,cv::NORM_INF)==0,"Undo restores all filter masks");
    repair.reset();check(cv::countNonZero(repair.mask)==0,"Replay resets restoration");

    for(auto limits:std::array<std::pair<int,int>,3>{{{40,135},{110,180},{20,220}}}){
        cv::Mat ramp(24,256,CV_8UC3);
        for(int y=0;y<ramp.rows;++y)for(int x=0;x<ramp.cols;++x){uchar v=limits.first+x*(limits.second-limits.first)/255;ramp.at<cv::Vec3b>(y,x)={v,v,v};}
        auto corrected=BrushRepair::autoContrast(ramp);cv::Mat gray;cv::cvtColor(corrected,gray,cv::COLOR_BGR2GRAY);double low,high;cv::minMaxLoc(gray,&low,&high);
        check(high-low>limits.second-limits.first,"Contrast adapts to different observed intensity ranges");
    }
    cv::Mat flat(24,24,CV_8UC3,cv::Scalar(100,110,120));
    check(cv::norm(BrushRepair::autoContrast(flat),flat,cv::NORM_INF)==0,"Flat input stays stable without division by zero");
    for(auto tool:{Filter::Clean,Filter::Contrast,Filter::Detail}){
        repair.reset();repair.stroke({150,130},{240,130},16,tool);repair.setStrength(tool,0);
        check(cv::norm(repair.image(),repair.damaged,cv::NORM_INF)==0,"Zero strength preserves damaged pixels");
        repair.setStrength(tool,.5);auto medium=repair.image();repair.setStrength(tool,1);auto strong=repair.image();
        check(cv::norm(medium,repair.damaged,cv::NORM_L1)>0,"Half strength applies a real filter");
        check(cv::norm(strong,repair.damaged,cv::NORM_L1)>cv::norm(medium,repair.damaged,cv::NORM_L1),"Strength changes the filter's visible effect");
        check(cv::norm(strong(cv::Rect(350,230,40,40)),repair.damaged(cv::Rect(350,230,40,40)),cv::NORM_INF)==0,"Strength never modifies unpainted regions");
        repair.beginStroke();repair.setStrength(tool,.1);repair.undo();check(repair.strength(tool)==1,"Undo restores the previous strength");
    }
    auto gift=repair.image();auto saved=gift.clone();repair.reset();
    check(cv::norm(gift,saved,cv::NORM_INF)==0,"Player's returned photo is an independent snapshot");
    BrushRepair natural;natural.load(small);
    check(!natural.peopleRestored(),"Untouched family does not trigger repair-specific thanks");
    natural.stroke({220,205},{300,205},100,Filter::Contrast);natural.setStrength(Filter::Contrast,0);
    check(!natural.peopleRestored(),"Zero-strength painting does not count as visible restoration");
    natural.setStrength(Filter::Contrast,.8f);
    check(natural.peopleRestored(),"Visible work in the family area earns a personal response");
    auto first=natural.image();natural.stroke({220,205},{300,205},100,Filter::Contrast);
    check(cv::norm(first,natural.image(),cv::NORM_INF)==0,"Repeated strokes do not accumulate filter damage");
    BrushRepair same;same.load(small);
    check(cv::norm(same.damaged,natural.damaged,cv::NORM_INF)==0,"Photo aging is deterministic");
    same.damaged=cv::Mat(40,40,CV_8UC3,cv::Scalar(100,100,100));same.reset();
    same.damaged.at<cv::Vec3b>(20,20)={230,230,230};same.stroke({20,20},{20,20},9);
    check(same.image().at<cv::Vec3b>(20,20)[0]==100,"Clean removes isolated dust at the brush centre");
    same.damaged.at<cv::Vec3b>(20,28)={230,230,230};auto feather=same.image().at<cv::Vec3b>(20,28)[0];
    check(feather>100&&feather<230,"The brush edge blends gradually inside the painted area");
    check(same.image().at<cv::Vec3b>(0,0)[0]==100,"Soft edges do not spill outside the brush");
    // Use the same small pixel digits as the game, then measure visible ink separation.
    auto clue=small.clone();cv::Rect plate(213,71,50,17),stain(203,61,70,37);
    cv::rectangle(clue,plate,{161,216,246},cv::FILLED);
    cv::Mat inkMask=cv::Mat::zeros(clue.size(),CV_8U),paperMask=inkMask.clone();paperMask(plate).setTo(255);
    std::array<std::array<int,7>,3> digits{{{{14,17,1,2,4,8,31}},{{14,17,19,21,25,17,14}},{{14,17,17,14,17,17,14}}}};
    for(int d=0;d<3;++d)for(int y=0;y<7;++y)for(int bit=0;bit<5;++bit)if(digits[d][y]&(1<<(4-bit))){
        cv::Rect dot(221+d*12+bit*2,73+y*2,2,2);clue(dot).setTo(cv::Scalar(34,44,54));inkMask(dot).setTo(255);paperMask(dot).setTo(0);
    }
    BrushRepair faded,ordinary;faded.load(clue,stain);ordinary.load(clue);
    cv::Mat outside(clue.size(),CV_8U,cv::Scalar(255));outside(stain).setTo(0);cv::Mat outsideDifference;cv::absdiff(faded.damaged,ordinary.damaged,outsideDifference);
    check(cv::mean(outsideDifference,outside)==cv::Scalar(0,0,0,0),"Clue aging leaves the rest of the photograph unchanged");
    auto separation=[&](const cv::Mat& image){cv::Mat gray;cv::cvtColor(image,gray,cv::COLOR_BGR2GRAY);return cv::mean(gray,paperMask)[0]-cv::mean(gray,inkMask)[0];};
    double faint=separation(faded.image());
    for(auto tool:{Filter::Clean,Filter::Contrast,Filter::Detail})faded.stroke({200,80},{275,80},25,tool);
    double readable=separation(faded.image());std::cout<<"Doorplate ink contrast: "<<faint<<" -> "<<readable<<"\n";
    check(faint<14,"The unpainted doorplate starts with faint ink");
    check(readable>25&&readable>faint*2.5,"Real filters recover readable contrast from the observation");
    check(cv::norm(faded.image()(cv::Rect(350,200,50,50)),faded.damaged(cv::Rect(350,200,50,50)),cv::NORM_INF)==0,"Clue repair stays inside the player's brush");
    auto envelope=story::foundEnvelope(),map=story::foundMap(),arrival=story::reachedHome();
    check(envelope.size()==1&&envelope[0].words.find("map")==std::string::npos,"The box reveals only an envelope");
    check(map.front().words.find("map inside")!=std::string::npos,"Opening the envelope reveals its map");
    check(arrival.size()==1&&arrival[0].words.find("back")!=std::string::npos,"Only arrival reveals the writing on the back");
    auto personal=story::ending(true),quiet=story::ending(false);
    check(personal[3].words!=quiet[3].words&&personal[3].words.find("held my hand")!=std::string::npos,"Lia responds to the player's actual family repair");
    auto opening=story::opening(),ending=story::ending();
    check(opening.front().speaker=="You"&&opening.front().narration,"The player introduces their archive without a speaker plate");
    check(!opening[5].narration&&opening[5].speaker=="You","Spoken player dialogue retains its nameplate");
    check(ending[7].narration&&ending.back().narration,"Departure and closing thoughts are narration");
    int bell=-1,greeting=-1,handover=-1,leave=-1;
    for(size_t i=0;i<opening.size();++i){if(opening[i].cue==story::Cue::Doorbell)bell=i;if(opening[i].speaker=="tonton"&&greeting<0)greeting=i;}
    check(bell>=0&&bell<greeting,"Doorbell comes before the assistant's greeting");
    for(size_t i=0;i<ending.size();++i){if(ending[i].cue==story::Cue::ReturnPhoto)handover=i;if(ending[i].cue==story::Cue::LiaLeaves)leave=i;}
    check(handover>=0&&leave>handover,"Photo is returned before Lia leaves");
    bool playerAfter=false,helperAfter=false;
    for(size_t i=leave+1;i<ending.size();++i){playerAfter|=ending[i].speaker=="You";helperAfter|=ending[i].speaker=="tonton";}
    check(playerAfter&&helperAfter,"The player and helper share the final conversation");
    for(auto script:{opening,ending,personal,envelope,map,arrival})for(auto speech:script){std::istringstream lines(speech.words);std::string line;int count=0;while(std::getline(lines,line)){++count;check(line.size()<=44,"NPC line fits the large native pixel font");}check(count<=3,"NPC speech stays short");}

    Maze maze;auto route=maze.solution();
    std::cout<<"Shortest maze route: "<<route.size()<<" gestures\n";
    check(!route.empty(),"Maze has a route home");
    check(!maze.move(Direction::Left),"Blocked directions do not move");
    check(maze.destination({5,7},Direction::Right)==cv::Point(9,7),"Stop at the corner instead of automatically turning north");
    check(maze.destination({9,7},Direction::Up)==cv::Point(9,5),"A fresh command follows the next straight leg");
    check(maze.route({9,7},Direction::Right).empty(),"Continuing into the corner wall cannot turn the envelope");
    for(auto d:route)check(maze.move(d),"Solution uses legal moves");
    check(maze.solved(),"Route reaches correct home");
    for(auto p:maze.decoys){maze.position=p;check(!maze.solved(),"Wrong houses do not complete story");check(!maze.solution().empty(),"Wrong house is returnable");}
    // Every reachable stopping point must have a route home (no soft-locks).
    maze.reset();std::queue<cv::Point> pending;pending.push(maze.position);std::set<int> visited;
    while(!pending.empty()){
        auto p=pending.front();pending.pop();if(!visited.insert(p.y*13+p.x).second)continue;
        maze.position=p;check(p==maze.home||!maze.solution().empty(),"Every reachable stop can still finish");
        for(auto d:directions){auto path=maze.route(p,d);auto q=maze.destination(p,d);
            for(size_t i=0;i<path.size();++i){
                check(path[i]==p+step(d)*int(i+1),"Every animated tile stays in the requested direction");
                if(i+1<path.size())check(!maze.house(path[i])&&maze.exits(path[i])==2,"A command never passes a fork or house");
            }
            if(q!=p)check(maze.house(q)||maze.exits(q)!=2||!maze.road(q+step(d)),"A straight route stops at a decision or corner");
            if(q!=p)pending.push(q);}
    }

    const double dt=1./30;
    for(auto direction:directions){
        PalmGuide palm;auto delta=step(direction);
        float x=.5f+delta.x*.3f,y=.5f+delta.y*.3f;
        for(int i=0;i<30;++i)check(palm.update(x,y,true,dt)==Direction::None,"Palm must first return to center");
        for(int i=0;i<8;++i)palm.update(.5,.5,true,dt);
        check(palm.ready(),"Center arms palm control");
        int events=0;for(int i=0;i<60;++i){auto d=palm.update(x,y,true,dt);if(d!=Direction::None){++events;check(d==direction,"Palm picks correct sector");}}
        check(events==1,"Holding a side emits exactly one command");
        palm.update(.5,.5,false,dt);check(!palm.ready(),"Tracking loss requires recentering");
    }
    Maze controlled;PalmGuide held;
    for(int i=0;i<8;++i)held.update(.5,.5,true,dt);
    for(int i=0;i<120;++i){auto d=held.update(.8,.5,true,dt);if(d!=Direction::None)controlled.move(d);}
    check(controlled.position==cv::Point(3,7),"Holding a hand to the side stops at the first junction");
    for(int i=0;i<8;++i)held.update(.5,.5,true,dt);
    for(int i=0;i<60;++i){auto d=held.update(.8,.5,true,dt);if(d!=Direction::None)controlled.move(d);}
    check(controlled.position==cv::Point(5,7),"A new centered gesture permits the next straight corridor");
    PalmGuide palm;for(int i=0;i<8;++i)palm.update(.5,.5,true,dt);
    for(int i=0;i<30;++i)check(palm.update(.8,.8,true,dt)==Direction::None,"Diagonal palm is not a command");
    for(int i=0;i<30;++i)check(palm.update(.8,.5,true,dt,{true,false,true,true})==Direction::None,"Blocked side cannot move");

    for(auto direction:directions){
        SwipeGate gate;auto delta=step(direction);Motion motion{{float(delta.x*140),float(delta.y*140)},1,true};
        for(int i=0;i<30;++i)check(gate.update(motion,dt)==Direction::None,"Camera must first settle");
        for(int i=0;i<16;++i)gate.update({},dt);
        int events=0;Direction result=Direction::None;
        for(int i=0;i<12;++i){auto d=gate.update(motion,dt);if(d!=Direction::None){++events;result=d;}}
        check(events==1&&result==direction,"One continuous wave emits exactly one direction");
        motion.velocity*=-1;
        for(int i=0;i<20;++i)check(gate.update(motion,dt)==Direction::None,"Immediate return motion is suppressed");
        for(int i=0;i<20;++i)gate.update({},dt);
        check(gate.ready(),"Rest rearms the next deliberate gesture");
    }
    SwipeGate gate;for(int i=0;i<20;++i)gate.update({},dt);
    for(int i=0;i<60;++i)check(gate.update({{110,110},1,true},dt)==Direction::None,"Ambiguous diagonal movement does not choose");
    cv::Mat previous(160,160,CV_8U);cv::RNG random(39);random.fill(previous,cv::RNG::UNIFORM,0,255);cv::GaussianBlur(previous,previous,{5,5},1);
    auto still=measureFlow(previous,previous,dt);check(!still.active,"Static camera texture has no direction");
    for(auto direction:directions){auto d=step(direction);cv::Mat current;
        cv::Mat affine=(cv::Mat_<double>(2,3)<<1,0,d.x*4,0,1,d.y*4);
        cv::warpAffine(previous,current,affine,previous.size(),cv::INTER_LINEAR,cv::BORDER_REFLECT);
        auto flow=measureFlow(previous,current,dt);
        check(flow.active&&flow.coherence>.9,"Farneback recovers coherent movement from image pairs");
        check(flow.velocity.dot(cv::Point2f(d.x,d.y))>90,"Measured direction has correct sign and magnitude");
    }

    DigitReader reader;reader.load(data+"/digits/digits.png");
    check(reader.read(cv::Mat::zeros(108,90,CV_8U))==-1,"Empty handwriting never becomes zero");
    cv::Mat dot=cv::Mat::zeros(108,90,CV_8U);cv::circle(dot,{45,54},5,255,cv::FILLED);
    check(reader.read(dot)==-1,"A dot is not a digit");
    auto sheet=cv::imread(data+"/digits/digits.png",cv::IMREAD_GRAYSCALE);int correct=0;
    for(int r=0;r<50;++r)for(int c=50;c<100;++c)correct+=reader.read(sheet(cv::Rect(c*20,r*20,20,20)))==r/5;
    std::cout<<"Held-out handwritten samples: "<<correct<<"/2500\n";
    check(correct>2100,"Independent digit sample accuracy exceeds 84 percent");
    for(int digit:{2,0,8}){
        cv::Mat ink=cv::Mat::zeros(108,90,CV_8U);std::vector<cv::Point> path;
        if(digit==2)path={{14,26},{20,17},{35,12},{50,15},{59,24},{60,34},{55,44},{44,55},{30,67},{17,82},{63,82}};
        else for(int j=0;j<=64;++j){float t=j*CV_PI*2/64;path.push_back(digit==0?cv::Point(45+24*std::cos(t),54+37*std::sin(t)):cv::Point(45+23*std::sin(2*t),54+36*std::cos(t)));}
        for(size_t i=1;i<path.size();++i)cv::line(ink,path[i-1],path[i],255,5,cv::LINE_AA);
        int predicted=reader.read(ink);std::cout<<"Mouse-style "<<digit<<" -> "<<predicted<<"\n";
        check(predicted==digit,"Mouse-written password must be recognized without injecting the answer");
    }
    for(int thickness:{3,5,8})for(int tilt:{-12,0,12}){
        cv::Mat ink=cv::Mat::zeros(108,90,CV_8U),rotated;
        cv::ellipse(ink,{45,35},{20,19},0,0,360,255,thickness,cv::LINE_AA);
        cv::ellipse(ink,{45,73},{23,19},0,0,360,255,thickness,cv::LINE_AA);
        cv::warpAffine(ink,rotated,cv::getRotationMatrix2D({45,54},tilt,1),ink.size());
        auto result=reader.guess(rotated);std::cout<<"Two-loop 8 ("<<thickness<<","<<tilt<<") -> "<<result.digit<<"\n";
        check(result.digit==8||std::find(result.candidates.begin(),result.candidates.end(),8)!=result.candidates.end(),"Two-stroke eight is recognized or offered for confirmation");
    }
    std::cout<<"PASS homeward: "<<checks<<" checks\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL homeward: "<<e.what()<<"\n";return 1;}}
