#include "ofApp.h"
#include <filesystem>

namespace {
using namespace pixel;
constexpr int W=mirror::canvasWidth,H=mirror::canvasHeight;
const std::array<std::string,3> titles={"LOST SEAL","THE ORIGINAL","NIGHT MOTHS"};
const std::array<std::array<std::string,3>,3> toolNames={{{"CLEAN","COLOR","EDGES"},{"ZOOM","POINTS","MATCH -9"},{"TRAILS","FLOW","LEFTWARD"}}};
const ofRectangle bookButton{534,8,28,25},helpButton{570,8,28,25},homeButton{606,8,28,25};
const ofRectangle markButton{480,347,146,29},keepButton{490,246,126,25};
const ofRectangle hintButton{490,293,88,26},restartButton{584,293,32,26};
const ofRectangle toggleButton{490,211,126,23},compareButton{402,347,62,29};
const ofRectangle closeButton{542,49,23,23},exportButton{430,316,128,24};
ofRectangle toolButton(int i){return {12.f+i*130,347,122,29};}
ofRectangle caseButton(int i){return {28.f+i*198,280,188,86};}
}

void ofApp::setup(){
    ofSetFrameRate(60);ofSetVerticalSync(true);ofEnableAlphaBlending();ofSetEscapeQuitsApp(false);
    cv::setNumThreads(2);
    ofDisableAntiAliasing();ofDisableSmoothing();ofSetWindowTitle("Mirror Archive");
    const auto resources=ofFilePath::getCurrentExeDir()+"../Resources/data/";
    if(ofDirectory::doesDirectoryExist(resources))ofSetDataPathRoot(resources);
    if(!hero.load("art/pixel-cottage.png"))ofLogError()<<"Pixel background missing.";
    else hero.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    ofFbo::Settings settings;settings.width=W;settings.height=H;
    settings.internalformat=GL_RGBA;settings.textureTarget=GL_TEXTURE_2D;settings.numSamples=0;
    canvas.allocate(settings);canvas.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    const auto applicationSupport=ofFilePath::getUserHomeDir()+"/Library/Application Support/MirrorArchive";
    savePath=applicationSupport+"/progress.json";
    if(!qaMode){
        ofDirectory::createDirectory(applicationSupport,true,true);
        try{if(ofFile::doesFileExist(savePath)){auto j=ofLoadJson(savePath);for(int i=0;i<3;++i)bestStars[i]=std::clamp(j.at("bestStars").at(i).get<int>(),0,3);}}
        catch(const std::exception& e){ofLogWarning()<<"Progress ignored: "<<e.what();}
    }
}

void ofApp::loadLevel(int index){
    level=std::clamp(index,0,2);screen=Screen::Play;tool=0;radius=72;motionTick=0;
    session.reset({900,560});claimMode=false;dragging=false;help=false;notebook=false;
    cleanEdges=true;compare=false;notes.clear();snapshots.clear();lastBrush={-1,-1};
    scanMessage="";lastPoint={450,280};
    if(level==0){data=mirror::LevelFactory::relics();processed[0]=mirror::VisionEngine::restore(data.observed);
        processed[1]=mirror::VisionEngine::chroma(data.observed);processed[2]=mirror::VisionEngine::edges(data.observed);
        processed[3]=mirror::VisionEngine::edges(data.observed,false);
    }else if(level==1){data=mirror::LevelFactory::gallery();processed[0]=data.observed.clone();
        processed[1]=mirror::VisionEngine::keypoints(data.observed);processed[2]=data.observed.clone();
        upload(data.reference,referenceTexture);
    }else{
        data.observed=mirror::LevelFactory::motionFrame(0);previous=data.observed.clone();
        flow=cv::Mat::zeros(data.observed.size(),CV_32FC2);
        processed[0]=mirror::VisionEngine::difference(previous,data.observed);
        processed[1]=mirror::VisionEngine::flowView(data.observed,flow,false);
        processed[2]=mirror::VisionEngine::flowView(data.observed,flow,true);
        data.answer=mirror::LevelFactory::motionAnswer(0);
    }
    nextMotion=ofGetElapsedTimef();dirty=true;refreshTexture();
    toastUntil=0;
}

void ofApp::update(){
    if(qaMode)runQA();
    if(screen!=Screen::Play)return;
    if(level==2 && !session.paused && !help && !notebook && ofGetElapsedTimef()>=nextMotion){
        nextMotion=ofGetElapsedTimef()+1.f/24;
        previous=data.observed;data.observed=mirror::LevelFactory::motionFrame(++motionTick);
        flow=mirror::VisionEngine::flow(previous,data.observed);
        processed[0]=mirror::VisionEngine::difference(previous,data.observed);
        processed[1]=mirror::VisionEngine::flowView(data.observed,flow,false);
        processed[2]=mirror::VisionEngine::flowView(data.observed,flow,true);
        data.answer=mirror::LevelFactory::motionAnswer(motionTick);dirty=true;
    }
    if(dirty)refreshTexture();
}

void ofApp::upload(const cv::Mat& image,ofTexture& texture){
    cv::Mat rgb;cv::cvtColor(image,rgb,cv::COLOR_BGR2RGB);
    ofPixels pixels;pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);texture.loadData(pixels);
    texture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
}
void ofApp::refreshTexture(){
    int layer=level==0 && tool==2 && !cleanEdges?3:tool;
    display=mirror::VisionEngine::blend(data.observed,processed[layer],session.masks[layer]);
    upload(compare?data.observed:display,sceneTexture);dirty=false;
}
void ofApp::selectTool(int index){tool=std::clamp(index,0,2);claimMode=false;dirty=true;lastBrush={-1,-1};}
void ofApp::notify(const std::string& message){toastMessage=message;toastUntil=ofGetElapsedTimef()+2.5f;}
glm::vec2 ofApp::logical(float x,float y)const{
    auto view=mirror::pixelViewport(ofGetWidth(),ofGetHeight());
    return {view.localX(x),view.localY(y)};
}
bool ofApp::over(const ofRectangle& rect)const{return rect.inside(logical(ofGetMouseX(),ofGetMouseY()));}
cv::Point ofApp::scenePoint(glm::vec2 p)const{return {std::clamp(int((p.x-sceneRect.x)/sceneRect.width*900),0,899),std::clamp(int((p.y-sceneRect.y)/sceneRect.height*560),0,559)};}

void ofApp::brushAt(glm::vec2 position){
    if(!sceneRect.inside(position))return;
    auto point=scenePoint(position);lastPoint=point;
    if(level==1 && tool==2){scanAt(point);return;}
    int layer=level==0 && tool==2 && !cleanEdges?3:tool;
    glm::vec2 p(point.x,point.y);
    if(lastBrush.x<0)lastBrush=p;
    float distance=glm::distance(lastBrush,p);int steps=std::max(1,int(distance/std::max(8,radius/3)));
    for(int i=1;i<=steps;++i){
        glm::vec2 q=glm::mix(lastBrush,p,float(i)/steps);
        if(!session.brush(layer,{int(q.x),int(q.y)},radius,level==1?75:135)){
            notify("LOW ENERGY");break;
        }
    }
    lastBrush=p;dirty=true;
}
void ofApp::scanAt(cv::Point point){
    if(!session.spend(9)){notify("NEED 9 ENERGY");return;}
    ++session.scans;
    const cv::Rect region(point.x-130,point.y-130,260,260);
    const auto result=mirror::VisionEngine::match(data.reference,data.observed,region);
    const cv::Rect clipped=region & cv::Rect(0,0,900,560);
    result.view(clipped).copyTo(processed[2](clipped));
    session.masks[2](clipped).setTo(255);
    scanMessage=ofToString(result.inliers)+" INLIERS";
    session.hasMatch|=result.verified;
    if(result.verified){session.evidence[2]=true;notify("MATCH FOUND!");}
    else notify("NO MATCH");
    dirty=true;
}
void ofApp::captureEvidence(){
    if(cv::countNonZero(session.masks[tool])<500){notify("EXPLORE FIRST");return;}
    if(level==2 && tool>0){
        cv::Mat motionMask;std::vector<cv::Mat> axes;cv::split(flow,axes);
        cv::compare(cv::abs(axes[0]),.55,motionMask,cv::CMP_GT);
        cv::bitwise_and(motionMask,session.masks[tool],motionMask);
        if(cv::countNonZero(motionMask)<30){notify("NO MOTION HERE");return;}
    }
    if(level==1 && tool==2 && !session.hasMatch){notify("FIND A MATCH FIRST");return;}
    if(notes.size()>=6){notify("NOTEBOOK FULL");return;}
    session.evidence[tool]=true;
    refreshTexture();
    cv::Mat rgb;cv::cvtColor(display,rgb,cv::COLOR_BGR2RGB);ofPixels pixels;
    pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);ofImage snapshot;snapshot.setFromPixels(pixels);
    snapshot.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    snapshots.push_back(snapshot);
    notes.push_back(toolNames[level][tool]+(level==1&&tool==2?" / "+scanMessage:""));
    notify("CLUE SAVED");
}
void ofApp::exportEvidence(){
    if(snapshots.empty()){notify("NO CLUES YET");return;}
    const std::string folder=ofFilePath::getEnclosingDirectory(savePath)+"evidence/"+ofGetTimestampString("%Y%m%d-%H%M%S")+"/";
    try{
        std::filesystem::create_directories(folder);
        if(!std::filesystem::is_directory(folder))throw std::runtime_error("Cannot create export directory");
        for(size_t i=0;i<snapshots.size();++i)
            if(!snapshots[i].save(folder+"clue-"+ofToString(i+1)+".png"))throw std::runtime_error("Cannot save clue image");
        ofJson metadata;metadata["case"]=level+1;metadata["energy"]=session.energy;metadata["notes"]=notes;
        if(!ofSavePrettyJson(folder+"evidence.json",metadata))throw std::runtime_error("Cannot save evidence metadata");
        notebook=false;
        notify("CLUES EXPORTED");
    }catch(const std::exception& e){notebook=false;notify(std::string("Export failed: ")+e.what());}
}
void ofApp::claimAt(cv::Point point){
    if(!session.ready(level)){notify(level==0?"SAVE COLOR + EDGES":level==1?"FIND A MATCH FIRST":"SAVE A FLOW CLUE");return;}
    if(!data.answer.contains(point)){
        ++session.mistakes;session.energy=std::max(0.f,session.energy-12);claimMode=false;
        notify("WRONG OBJECT  -12");return;
    }
    session.solved=true;screen=Screen::Result;
    bestStars[level]=std::max(bestStars[level],session.stars());saveProgress();
}
void ofApp::useHint(){
    if(!session.spend(12)){notify("NEED 12 ENERGY");return;}
    ++session.hints;
    const std::array<std::string,3> hints={"TEAL SEALS: COUNT THE POINTS","LOOK IN THE LOWER ROW","WATCH THE FOURTH MOTH"};
    notify(hints[level]);toastUntil=ofGetElapsedTimef()+9;
}
void ofApp::saveProgress(){
    if(qaMode)return;
    try{ofJson j;j["bestStars"]=bestStars;if(!ofSavePrettyJson(savePath,j))throw std::runtime_error("Cannot write progress file");}
    catch(const std::exception& e){ofLogWarning()<<"Cannot save progress: "<<e.what();notify("SAVE FAILED");}
}
void ofApp::exit(){saveProgress();}

void ofApp::panel(float x,float y,float w,float h,ofColor color,float){pixel::panel(int(x),int(y),int(w),int(h),color);}
void ofApp::label(const std::string& value,float x,float y,ofColor color,int style){
    const int scale=style>=3?3:style==2?2:1;
    pixel::text(value,int(x),int(y)-7*scale,color,scale);
}
void ofApp::button(const std::string& value,const ofRectangle& rect,bool active){
    const auto fill=active?green:over(rect)?light:paper;
    pixel::panel(rect.x,rect.y,rect.width,rect.height,fill);
    const int x=int(rect.x+(rect.width-pixel::textWidth(value))/2);
    pixel::text(value,x,int(rect.y+(rect.height-7)/2),active?light:ink);
}
void ofApp::drawIcon(int icon,float x,float y,float size,ofColor color){
    pixel::sprite(icon,int(x),int(y),std::max(1,int(size)/13),color);
}

void ofApp::draw(){
    ofBackground(shadow);
    canvas.begin();ofClear(shadow);ofPushStyle();ofFill();ofDisableAntiAliasing();ofDisableSmoothing();
    if(screen==Screen::Home)drawHome();else if(screen==Screen::Play)drawPlay();else drawResult();
    if(help||notebook)drawModal();
    if(screen==Screen::Play&&!help&&!notebook&&ofGetElapsedTimef()<toastUntil){
        const int width=std::min(438,pixel::textWidth(toastMessage)+24);
        const int x=int(sceneRect.x+(sceneRect.width-width)/2);
        pixel::panel(x,303,width,25,paper);pixel::text(toastMessage,x+12,312,ink);
    }
    ofPopStyle();canvas.end();
    const auto view=mirror::pixelViewport(ofGetWidth(),ofGetHeight());
    ofSetColor(255);canvas.draw(view.x,view.y,W*view.scale,H*view.scale);
}
void ofApp::drawHome(){
    ofSetColor(255);if(hero.isAllocated())hero.draw(0,0,W,H);
    pixel::panel(159,18,322,64,wood);
    pixel::text("MIRROR ARCHIVE",205,43,ink,3);
    pixel::text("MIRROR ARCHIVE",203,41,light,3);
    drawIcon(2,177,49,13,gold);drawIcon(2,465,49,13,gold);
    // A few whole-pixel glints, without subpixel movement or blurred light sprites.
    for(int i=0;i<5;++i){
        int x=100+i*107,y=125+(i*37)%92;
        if(std::sin(ofGetElapsedTimef()*1.5+i)>0.5){
            ofSetColor(gold);ofDrawRectangle(x-2,y,5,1);ofDrawRectangle(x,y-2,1,5);
        }
    }
    for(int i=0;i<3;++i){
        auto rect=caseButton(i);const bool hover=over(rect);
        pixel::panel(rect.x,rect.y,rect.width,rect.height,hover?light:paper);
        drawIcon(i==0?2:i==1?12:8,rect.x+30,rect.y+30,26,i==0?teal:i==1?gold:ofColor(157,132,192));
        pixel::text(titles[i],rect.x+58,rect.y+24,ink);
        for(int j=0;j<3;++j)drawIcon(2,rect.x+73+j*25,rect.y+56,13,j<bestStars[i]?gold:ofColor(179,142,91));
        if(hover)pixel::text(">",rect.x+169,rect.y+25,red);
    }
    button("?",{606,8,28,25});
}

void ofApp::drawPlay(){
    ofSetColor(shadow);ofDrawRectangle(0,0,W,H);
    // Plank frame: the playable image remains the dominant part of the screen.
    for(int y=42;y<H;y+=24){ofSetColor(y%48?ofColor(83,49,39):ofColor(89,53,40));ofDrawRectangle(0,y,W,23);}
    pixel::panel(5,3,630,38,wood);
    pixel::text(ofToString(level+1)+" "+titles[level],17,15,light,2);
    drawIcon(7,393,21,13,gold);
    ofSetColor(ink);ofDrawRectangle(408,14,82,13);
    for(int i=0;i<20;++i){ofSetColor(session.energy>i*5?(session.energy<20?red:green):edge);ofDrawRectangle(410+i*4,16,3,9);}
    pixel::text(ofToString(int(std::ceil(session.energy))),498,17,light);
    button("",bookButton);drawIcon(5,548,20,13,gold);
    button("?",helpButton);button("",homeButton);drawIcon(6,620,20,13,green);
    pixel::panel(8,49,462,292,wood);ofSetColor(255);sceneTexture.draw(sceneRect);
    pixel::panel(480,49,146,292,paper);
    pixel::text(level==1?"FIND THE ORIGINAL":"FIND",level==1?502:540,65,ink);
    if(level==0){
        // Same seven-point symbol as the clue; drawn on the low-resolution canvas.
        auto shape=mirror::LevelFactory::star({553,111},25,12,7);
        ofSetColor(ink);ofBeginShape();for(auto p:shape)ofVertex(p.x+1,p.y+2);ofEndShape(true);
        ofSetColor(teal);ofBeginShape();for(auto p:shape)ofVertex(p.x,p.y);ofEndShape(true);
        pixel::text("TEAL + 7 POINTS",512,148,ink);
    }else if(level==1){
        ofSetColor(ink);ofDrawRectangle(500,84,106,106);
        ofSetColor(255);referenceTexture.draw(503,87,100,100);
    }else{
        drawIcon(8,553,111,39,ofColor(185,158,220));
        drawIcon(4,553,151,13,gold);pixel::text("FLIES LEFT",524,172,ink);
    }
    auto check=[&](const std::string& name,int y,bool complete){
        ofSetColor(ink);ofDrawRectangle(498,y-1,11,10);ofSetColor(complete?green:wood);ofDrawRectangle(499,y,9,8);
        if(complete)pixel::text("+",501,y+1,light);
        pixel::text(name,516,y+1,ink);
    };
    if(level==0){check("COLOR",176,session.evidence[1]);check("OUTLINE",192,session.evidence[2]);}
    else if(level==1){check("MATCH",199,session.hasMatch);if(!scanMessage.empty())pixel::text(scanMessage,505,219,ink);}
    else check("MOTION",192,session.ready(level));
    if(level==0&&tool==2)button(cleanEdges?"SMOOTH: ON":"SMOOTH: OFF",toggleButton,cleanEdges);
    if(level==2)button(session.paused?"RESUME":"PAUSE",toggleButton,session.paused);
    button("SAVE CLUE",keepButton);button("HINT -12",hintButton);
    button("",restartButton);drawIcon(11,600,305,13,red);
    for(int i=0;i<3;++i){
        auto rect=toolButton(i);bool active=tool==i&&!claimMode;
        button("",rect,active);drawIcon(i,int(rect.x)+17,361,13,active?light:teal);
        pixel::text(ofToString(i+1)+" "+toolNames[level][i],rect.x+32,358,active?light:ink);
    }
    button(compare?"BACK":"BEFORE",compareButton,compare);
    button(claimMode?"PICK OBJECT":"MARK",markButton,true);
    if(level==1&&tool==0&&cv::countNonZero(session.masks[0])>0){
        pixel::panel(22,218,118,108,wood);ofSetColor(255);
        sceneTexture.drawSubsection(28,224,106,96,std::clamp(lastPoint.x-23,0,854),std::clamp(lastPoint.y-21,0,518),46,42);
    }
    auto mouse=logical(ofGetMouseX(),ofGetMouseY());
    if(sceneRect.inside(mouse)&&!help&&!notebook){
        int x=int(mouse.x),y=int(mouse.y),r=(level==1&&tool==2?130:radius)/2;
        ofSetColor(light);
        const auto dot=[&](int px,int py){if(sceneRect.inside(px,py))ofDrawRectangle(px,py,1,1);};
        if(claimMode){for(int i=-7;i<=7;++i){dot(x+i,y);dot(x,y+i);}}
        else if(level==1&&tool==2){
            for(int dx:{-r,r})for(int dy:{-r,r}){
                for(int i=0;i<8;++i){dot(x+dx-(dx>0?7:0)+i,y+dy);dot(x+dx,y+dy-(dy>0?7:0)+i);}
            }
        }else{
            for(int yStep=-r;yStep<=r;++yStep){int xStep=int(std::sqrt(float(r*r-yStep*yStep)));dot(x-xStep,y+yStep);dot(x+xStep,y+yStep);}
        }
    }
}

void ofApp::drawResult(){
    ofSetColor(255);if(hero.isAllocated())hero.draw(0,0,W,H);
    ofSetColor(24,20,36,110);ofDrawRectangle(0,0,W,H);
    pixel::panel(148,75,344,239,paper);
    pixel::text("CASE SOLVED!",249,102,ink,2);
    for(int i=0;i<3;++i)drawIcon(2,266+i*54,160,39,i<session.stars()?gold:ofColor(175,145,110));
    const std::string energy="ENERGY "+ofToString(int(std::ceil(session.energy)));
    pixel::text(energy,(W-pixel::textWidth(energy))/2,207,ink);
    button("REPLAY",{172,253,126,35});button(level<2?"NEXT >":"HOME",{318,253,150,35},true);
}
void ofApp::drawModal(){
    ofSetColor(24,20,36,180);ofDrawRectangle(0,0,W,H);
    pixel::panel(64,40,512,309,paper);button("X",closeButton);
    pixel::text(notebook?"CLUES":"CONTROLS",82,57,ink,2);
    if(notebook){
        if(notes.empty())pixel::text("NO CLUES YET",275,178,wood);
        for(size_t i=0;i<notes.size();++i){
            int x=80+(i%3)*160,y=91+(i/3)*108;
            pixel::panel(x,y,144,99,light);ofSetColor(255);snapshots[i].draw(x+5,y+5,134,78);
            pixel::text(notes[i].substr(0,22),x+6,y+86,ink);
        }
        button("EXPORT",exportButton);
    }else{
        const std::array<std::pair<std::string,std::string>,9> controls={{{"DRAG","EXPLORE"},{"1 2 3","SWITCH TOOL"},{"SCROLL","BRUSH SIZE"},{"E / N","SAVE / VIEW CLUES"},{"M","MARK OBJECT"},{"TAB","BEFORE / AFTER"},{"SPACE","PAUSE MOTHS"},{"X","SMOOTH EDGES"},{"R / ESC","RETRY / HOME"}}};
        for(int i=0;i<int(controls.size());++i){pixel::text(controls[i].first,100,100+i*24,wood);pixel::text(controls[i].second,274,100+i*24,ink);}
    }
}

void ofApp::mousePressed(int x,int y,int buttonCode){
    if(buttonCode!=OF_MOUSE_BUTTON_LEFT)return;
    auto p=logical(x,y);
    if(help||notebook){
        if(closeButton.inside(p)){help=false;notebook=false;}
        else if(notebook&&exportButton.inside(p))exportEvidence();
        return;
    }
    if(screen==Screen::Home){
        for(int i=0;i<3;++i)if(caseButton(i).inside(p)){loadLevel(i);return;}
        if(ofRectangle(606,8,28,25).inside(p))help=true;
        return;
    }
    if(screen==Screen::Result){
        if(ofRectangle(318,253,150,35).inside(p)){if(level<2)loadLevel(level+1);else screen=Screen::Home;}
        else if(ofRectangle(172,253,126,35).inside(p))loadLevel(level);
        return;
    }
    if(bookButton.inside(p)){notebook=true;return;}
    if(helpButton.inside(p)){help=true;return;}
    if(homeButton.inside(p)){screen=Screen::Home;return;}
    for(int i=0;i<3;++i)if(toolButton(i).inside(p)){selectTool(i);return;}
    if(markButton.inside(p)){claimMode=!claimMode;return;}
    if(keepButton.inside(p)){captureEvidence();return;}
    if(hintButton.inside(p)){useHint();return;}
    if(restartButton.inside(p)){loadLevel(level);return;}
    if(compareButton.inside(p)){compare=!compare;dirty=true;return;}
    if(toggleButton.inside(p)){
        if(level==2)session.paused=!session.paused;
        if(level==0&&tool==2){cleanEdges=!cleanEdges;dirty=true;}
        return;
    }
    if(sceneRect.inside(p)){
        if(claimMode)claimAt(scenePoint(p));
        else{lastBrush={-1,-1};brushAt(p);dragging=!(level==1&&tool==2);}
    }
}
void ofApp::mouseDragged(int x,int y,int button){if(dragging&&screen==Screen::Play&&!help&&!notebook)brushAt(logical(x,y));}
void ofApp::mouseReleased(int,int,int){dragging=false;lastBrush={-1,-1};}
void ofApp::mouseScrolled(int,int,float,float sy){radius=std::clamp(radius+int(sy*8),28,138);}
void ofApp::windowResized(int width,int height){
    if(width<W||height<H)ofSetWindowShape(std::max(width,W),std::max(height,H));
}
void ofApp::keyPressed(int key){
    if(key==OF_KEY_ESC){if(help||notebook){help=false;notebook=false;}else screen=Screen::Home;dragging=false;return;}
    if(key=='h'||key=='H'){help=!help;notebook=false;return;}
    if(help||notebook){if(key=='n'||key=='N'){notebook=false;}else if(notebook&&(key=='s'||key=='S'))exportEvidence();return;}
    if(screen==Screen::Home){if(key==OF_KEY_RETURN)loadLevel(0);return;}
    if(screen==Screen::Result){if(key=='r'||key=='R')loadLevel(level);if(key==OF_KEY_RETURN){if(level<2)loadLevel(level+1);else screen=Screen::Home;}return;}
    if(key>='1'&&key<='3')selectTool(key-'1');
    else if(key=='e'||key=='E')captureEvidence();
    else if(key=='m'||key=='M')claimMode=!claimMode;
    else if(key=='n'||key=='N')notebook=true;
    else if(key=='r'||key=='R')loadLevel(level);
    else if(key=='[')radius=std::max(28,radius-12);
    else if(key==']')radius=std::min(138,radius+12);
    else if(key==' '&&level==2)session.paused=!session.paused;
    else if(key=='x'&&level==0&&tool==2){cleanEdges=!cleanEdges;dirty=true;}
    else if(key==OF_KEY_TAB){compare=!compare;dirty=true;}
}

// Deterministic smoke tour uses the same public input/action paths as a player.
// It writes screenshots in the working directory and never changes saved progress.
void ofApp::runQA(){
    ++qaFrame;
    if(qaFrame%30)return;
    auto brushPoint=[this](int x,int y){brushAt({sceneRect.x+x*sceneRect.width/900,sceneRect.y+y*sceneRect.height/560});};
    const auto capture=[](const std::string& name){
        const auto folder=ofFilePath::getCurrentExeDir()+"../../../../qa/";
        ofDirectory::createDirectory(folder,true,true);ofSaveScreen(folder+name);
    };
    switch(qaStage++){
        case 0:capture("qa-home.png");loadLevel(0);break;
        case 1:selectTool(1);brushPoint(780,278);captureEvidence();selectTool(2);brushPoint(780,278);captureEvidence();toastUntil=0;break;
        case 2:capture("qa-case1.png");claimAt({780,278});qaPassed&=session.solved;break;
        case 3:capture("qa-result.png");loadLevel(1);break;
        case 4:selectTool(2);scanAt({450,412});captureEvidence();toastUntil=0;break;
        case 5:capture("qa-case2.png");claimAt({450,412});qaPassed&=session.solved;break;
        case 6:loadLevel(2);break;
        case 7:selectTool(1);brushPoint(mirror::LevelFactory::mothCenter(3,motionTick).x,320);break;
        case 8:session.paused=true;brushPoint(mirror::LevelFactory::mothCenter(3,motionTick).x,320);captureEvidence();toastUntil=0;break;
        case 9:capture("qa-case3.png");claimAt(mirror::LevelFactory::mothCenter(3,motionTick));qaPassed&=session.solved;break;
        case 10:capture("qa-complete.png");ofLogNotice()<<"QA tour ended: "<<(qaPassed?"ALL CASES COMPLETED":"FAILED");ofExit(qaPassed?0:1);break;
    }
}
