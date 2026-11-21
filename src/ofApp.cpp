#include "ofApp.h"

namespace {
using namespace pixel;
constexpr int W=mirror::canvasWidth,H=mirror::canvasHeight;
const std::array<std::string,3> titles={"THE LOST KEY","THE KEEPER'S MAP","THE LAST FILM"};
const std::array<std::string,3> discoveries={"KEY RESTORED","MAP IDENTIFIED","FILM REPAIRED"};
const std::array<std::string,3> story={
    "THE KEY OPENS THE KEEPER'S STUDY.",
    "HIS MAP LEADS TO THE GREENHOUSE FILM.",
    "THE KEEPER'S LAST LIGHT IS FOUND."
};
const ofRectangle startButton{244,296,152,40},homeButton{602,9,28,26};
const ofRectangle hintButton{515,302,104,29},pauseButton{515,263,104,29};
}

void ofApp::setup(){
    ofSetFrameRate(60);ofSetVerticalSync(true);ofEnableAlphaBlending();ofSetEscapeQuitsApp(false);
    cv::setNumThreads(2);ofDisableAntiAliasing();ofDisableSmoothing();
    ofSetWindowTitle("Mirror Archive");
    const auto resources=ofFilePath::getCurrentExeDir()+"../Resources/data/";
    if(ofDirectory::doesDirectoryExist(resources))ofSetDataPathRoot(resources);
    ofFbo::Settings settings;settings.width=W;settings.height=H;
    settings.internalformat=GL_RGBA;settings.textureTarget=GL_TEXTURE_2D;settings.numSamples=0;
    canvas.allocate(settings);canvas.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    try{
        factory.load(ofToDataPath("art",true));
        if(!hero.load("art/pixel-cottage.png")||!moth.load("art/moth.png"))
            throw std::runtime_error("Missing menu or moth art");
        hero.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        moth.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    }catch(const std::exception& error){
        ofLogError()<<error.what();errorMessage="ART FILES MISSING. PLEASE REBUILD.";screen=Screen::Error;
        if(qaMode)ofExit(1);
    }
}

void ofApp::startStory(){campaign.start();loadChapter();}
void ofApp::loadChapter(){
    screen=Screen::Play;gesture.cancel();lastBrush={-1,-1};motionTick=0;
    session.reset({mirror::LevelFactory::width,mirror::LevelFactory::height});
    matchPolygon.clear();toastUntil=hintUntil=nextScan=0;
    if(campaign.chapter==0){
        data=factory.archive();processed=mirror::VisionEngine::repairLens(data.observed);
        upload(data.reference,referenceTexture);
    }else if(campaign.chapter==1){
        data=factory.study();processed=mirror::VisionEngine::keypoints(data.observed);
        upload(data.reference,referenceTexture);
    }else{
        data={};data.observed=factory.motionFrame(0);advanceFilm();
    }
    nextMotion=ofGetElapsedTimef()+1.f/24;dirty=true;refreshTexture();
    if(campaign.chapter==0)notify("RESTORE THE KEEPER'S LOST RECORD.");
}
void ofApp::advanceStory(){
    if(!campaign.advance())return;
    if(campaign.complete){screen=Screen::Complete;return;}
    loadChapter();
}
void ofApp::advanceFilm(){
    previous=data.observed;data.observed=factory.motionFrame(++motionTick);
    flow=mirror::VisionEngine::flow(previous,data.observed);
    processed=mirror::VisionEngine::motionLens(previous,data.observed,flow);
    data.answer=mirror::LevelFactory::motionAnswer(motionTick);dirty=true;
}
void ofApp::update(){
    const float now=ofGetElapsedTimef();
    if(screen==Screen::Transition&&now>=transitionAt)advanceStory();
    if(screen==Screen::Play){
        if(campaign.chapter==2&&!session.paused&&now>=nextMotion){
            nextMotion=now+1.f/24;advanceFilm();
        }
        if(dirty)refreshTexture();
    }
    if(qaMode)runQA();
}

void ofApp::upload(const cv::Mat& image,ofTexture& texture){
    cv::Mat rgb;cv::cvtColor(image,rgb,cv::COLOR_BGR2RGB);
    ofPixels pixels;pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);
    texture.loadData(pixels);texture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
}
void ofApp::refreshTexture(){
    display=mirror::VisionEngine::blend(data.observed,processed,session.mask);
    upload(display,sceneTexture);dirty=false;
}
void ofApp::notify(const std::string& message){toastMessage=message;toastUntil=ofGetElapsedTimef()+2.5f;}
glm::vec2 ofApp::logical(float x,float y)const{
    const auto view=mirror::pixelViewport(ofGetWidth(),ofGetHeight());
    return {view.localX(x),view.localY(y)};
}
bool ofApp::over(const ofRectangle& rect)const{return rect.inside(logical(ofGetMouseX(),ofGetMouseY()));}
cv::Point ofApp::scenePoint(glm::vec2 point)const{
    return {std::clamp(int((point.x-sceneRect.x)*2),0,mirror::LevelFactory::width-1),
            std::clamp(int((point.y-sceneRect.y)*2),0,mirror::LevelFactory::height-1)};
}
glm::vec2 ofApp::boardPoint(cv::Point point)const{return {sceneRect.x+point.x/2.f,sceneRect.y+point.y/2.f};}

void ofApp::brushAt(glm::vec2 position){
    if(!sceneRect.inside(position))return;
    auto point=scenePoint(position);glm::vec2 current(point.x,point.y);
    if(lastBrush.x<0)lastBrush=current;
    const int steps=std::max(1,int(glm::distance(lastBrush,current)/20));
    for(int i=1;i<=steps;++i){
        auto pointOnStroke=glm::mix(lastBrush,current,float(i)/steps);
        session.brush({int(pointOnStroke.x),int(pointOnStroke.y)},82);
    }
    lastBrush=current;dirty=true;
    if(campaign.chapter==1&&ofGetElapsedTimef()>=nextScan){
        scanAt(point);nextScan=ofGetElapsedTimef()+.4f;
    }
}
mirror::MatchEvidence ofApp::scanAt(cv::Point point){
    auto result=mirror::VisionEngine::match(data.reference,data.observed,
                                           {point.x-150,point.y-140,300,280});
    if(result.verified){
        matchPolygon=result.polygon;
        processed=result.view;dirty=true;
        const auto region=cv::Rect(point.x-150,point.y-140,300,280)&cv::Rect(0,0,960,600);
        session.mask(region).setTo(255);
    }
    return result;
}
void ofApp::claimAt(cv::Point point){
    bool correct=false;
    if(campaign.chapter==0)correct=data.answer.contains(point);
    else if(campaign.chapter==1){
        const auto match=scanAt(point);
        correct=match.verified&&cv::pointPolygonTest(match.polygon,point,false)>=0;
    }else correct=data.answer.contains(point)&&mirror::VisionEngine::movingLeft(flow,point);
    if(!correct){notify("NOT THIS ONE");return;}
    campaign.solve();gesture.cancel();hintUntil=toastUntil=0;screen=Screen::Transition;
    transitionAt=ofGetElapsedTimef()+(qaMode?1.1f:3.6f);
    if(campaign.chapter==0)upload(processed,sceneTexture);
    else upload(data.observed,sceneTexture);
}

void ofApp::centered(const std::string& text,int centerX,int y,ofColor color,int scale){
    pixel::text(text,centerX-pixel::textWidth(text,scale)/2,y,color,scale);
}
void ofApp::button(const std::string& text,const ofRectangle& rect,bool active,int scale){
    pixel::panel(rect.x,rect.y,rect.width,rect.height,active?green:over(rect)?light:paper);
    centered(text,rect.getCenter().x,rect.y+(rect.height-7*scale)/2,active?light:ink,scale);
}
void ofApp::draw(){
    ofBackground(shadow);
    canvas.begin();ofClear(shadow);ofPushStyle();ofFill();ofDisableAntiAliasing();ofDisableSmoothing();
    if(screen==Screen::Home)drawHome();
    else if(screen==Screen::Complete)drawComplete();
    else if(screen==Screen::Error)centered(errorMessage,320,180,light);
    else{
        drawPlay();
        if(screen==Screen::Transition)drawTransition();
        else if(ofGetElapsedTimef()<toastUntil){
            const int width=pixel::textWidth(toastMessage)+20;
            pixel::panel(252-width/2,310,width,25,paper);
            centered(toastMessage,252,319,ink);
        }
    }
    ofPopStyle();canvas.end();
    const auto view=mirror::pixelViewport(ofGetWidth(),ofGetHeight());
    ofSetColor(255);canvas.draw(view.x,view.y,W*view.scale,H*view.scale);
}
void ofApp::drawHome(){
    ofSetColor(255);hero.draw(0,0,W,H);
    pixel::panel(171,36,298,74,wood);
    centered("MIRROR ARCHIVE",320,53,shadow,3);
    centered("MIRROR ARCHIVE",318,51,light,3);
    ofSetColor(gold);ofDrawRectangle(246,88,148,2);
    button("START",startButton,true,2);
}
void ofApp::drawPlay(){
    ofSetColor(edge);ofDrawRectangle(0,0,W,41);
    pixel::sprite(5,22,20,1,gold);
    pixel::text("ARCHIVE RESTORER",38,9,light);
    pixel::text(titles[campaign.chapter],38,24,gold);
    for(int i=0;i<3;++i){
        const int x=506+i*29;
        if(i<2){ofSetColor(wood);ofDrawRectangle(x+14,20,15,2);}
        pixel::panel(x,11,23,21,i<=campaign.chapter?green:wood);
        if(i<campaign.chapter)pixel::sprite(9,x+11,21,1,light);
        else centered(ofToString(i+1),x+11,18,light);
    }
    button("",homeButton);pixel::sprite(6,616,21,1,green);
    pixel::panel(6,42,492,312,wood);
    ofSetColor(255);sceneTexture.draw(sceneRect);
    pixel::panel(505,42,124,312,paper);
    centered(campaign.chapter==1?"MATCH THIS":"FIND",567,64,ink);
    ofSetColor(edge);ofDrawRectangle(517,87,100,98);
    ofSetColor(light);ofDrawRectangle(519,89,96,94);
    ofSetColor(255);
    if(campaign.chapter==0)referenceTexture.draw(520,104,94,63);
    else if(campaign.chapter==1)referenceTexture.draw(520,95,94,81);
    else moth.draw(520,106,94,64);
    ofSetColor(wood);ofDrawRectangle(562,84,10,5);
    if(campaign.chapter==0){
        centered("THE TEAL KEY",567,200,ink);
        pixel::sprite(1,567,234,1,teal);
    }else if(campaign.chapter==1){
        centered("LIGHTHOUSE SEAL",567,200,ink);
        if(!matchPolygon.empty()){pixel::sprite(9,567,231,2,green);centered("MATCH FOUND",567,255,green);}
    }else{
        centered("FLYING LEFT",567,200,ink);
        pixel::sprite(4,567,232,2,gold);
        button(session.paused?"RESUME":"PAUSE",pauseButton,session.paused);
    }
    button("HINT",hintButton);
    centered("DRAG TO RESTORE  /  CLICK TO CHOOSE",252,365,light);
    centered(ofToString(campaign.chapter+1)+" / 3",567,365,paper);

    if(screen!=Screen::Play)return;
    if(ofGetElapsedTimef()<hintUntil){
        const auto center=boardPoint((data.answer.tl()+data.answer.br())/2);
        const int x=std::clamp(int(center.x)-60,14,370),y=std::clamp(int(center.y)-42,50,262);
        ofSetColor(gold);
        for(int dx:{0,116})for(int dy:{0,80}){
            ofDrawRectangle(x+dx,y+dy,4,4);
            ofDrawRectangle(x+(dx==0?0:108),y+dy,12,2);
            ofDrawRectangle(x+dx,y+(dy==0?0:72),2,12);
        }
    }
    auto mouse=logical(ofGetMouseX(),ofGetMouseY());
    if(sceneRect.inside(mouse)){
        const int x=int(mouse.x),y=int(mouse.y);
        ofSetColor(light);
        const auto dot=[&](int px,int py){if(sceneRect.inside(px,py))ofDrawRectangle(px,py,1,1);};
        if(gesture.dragged){
            for(int yy=-41;yy<=41;++yy){
                int xx=int(std::sqrt(float(41*41-yy*yy)));dot(x-xx,y+yy);dot(x+xx,y+yy);
            }
        }else for(int i=-4;i<=4;++i){dot(x+i,y);dot(x,y+i);}
    }
}
void ofApp::drawTransition(){
    ofSetColor(shadow,130);ofDrawRectangle(0,0,W,H);
    pixel::panel(148,125,344,134,paper);
    pixel::sprite(9,320,145,2,green);
    centered(discoveries[campaign.chapter],320,172,ink,2);
    centered(story[campaign.chapter],320,208,ink);
    const float remaining=std::clamp((transitionAt-ofGetElapsedTimef())/(qaMode?1.1f:3.6f),0.f,1.f);
    ofSetColor(wood);ofDrawRectangle(176,237,288,3);
    ofSetColor(green);ofDrawRectangle(176,237,int(288*(1-remaining)),3);
}
void ofApp::drawComplete(){
    ofSetColor(255);hero.draw(0,0,W,H);
    ofSetColor(shadow,145);ofDrawRectangle(0,0,W,H);
    pixel::panel(126,76,388,234,paper);
    centered("ARCHIVE RESTORED",320,99,ink,2);
    ofSetColor(255);
    sceneTexture.drawSubsection(249,127,142,83,754,40,188,110);
    centered("THE KEEPER'S LAST LIGHT",320,226,ink);
    centered("SAVED FROM THE STORM.",320,243,wood);
    button("HOME",{244,269,152,29},true);
}

void ofApp::mousePressed(int x,int y,int buttonCode){
    if(buttonCode!=OF_MOUSE_BUTTON_LEFT)return;
    const auto p=logical(x,y);
    if(screen==Screen::Home){if(startButton.inside(p))startStory();return;}
    if(screen==Screen::Complete){if(ofRectangle(244,269,152,29).inside(p))screen=Screen::Home;return;}
    if(screen!=Screen::Play)return;
    if(homeButton.inside(p)){gesture.cancel();screen=Screen::Home;return;}
    if(hintButton.inside(p)){hintUntil=ofGetElapsedTimef()+6;return;}
    if(campaign.chapter==2&&pauseButton.inside(p)){session.paused=!session.paused;return;}
    if(sceneRect.inside(p)){
        gesture.begin({p.x,p.y});lastBrush={-1,-1};
    }
}
void ofApp::mouseDragged(int x,int y,int buttonCode){
    if(buttonCode!=OF_MOUSE_BUTTON_LEFT||screen!=Screen::Play)return;
    auto p=logical(x,y);
    const bool firstStroke=!gesture.dragged;
    if(gesture.move({p.x,p.y})){
        if(firstStroke)brushAt({gesture.start.x,gesture.start.y});
        if(sceneRect.inside(p))brushAt(p);else lastBrush={-1,-1};
    }
}
void ofApp::mouseReleased(int x,int y,int buttonCode){
    if(buttonCode!=OF_MOUSE_BUTTON_LEFT||screen!=Screen::Play)return;
    const auto p=logical(x,y);
    const bool stroke=gesture.pressed&&gesture.dragged;
    const bool choose=gesture.release({p.x,p.y});lastBrush={-1,-1};
    if(!sceneRect.inside(p))return;
    if(choose)claimAt(scenePoint(p));
    else if(stroke&&campaign.chapter==1)scanAt(scenePoint(p));
}
void ofApp::keyPressed(int key){
    if(key==OF_KEY_ESC){gesture.cancel();if(screen!=Screen::Error)screen=Screen::Home;return;}
    if(screen==Screen::Home&&key==OF_KEY_RETURN){startStory();return;}
    if(screen==Screen::Complete&&key==OF_KEY_RETURN){screen=Screen::Home;return;}
    if(screen==Screen::Play&&campaign.chapter==2&&key==' ')session.paused=!session.paused;
}
void ofApp::windowResized(int width,int height){
    if(width<W||height<H)ofSetWindowShape(std::max(width,W),std::max(height,H));
}

// Screenshots and inputs follow the same story progression as normal play.
// QA never jumps to a chosen chapter and never writes player data.
void ofApp::runQA(){
    if(++qaFrame>2400){ofLogError()<<"QA timed out at stage "<<qaStage;ofExit(1);return;}
    if(qaFrame%20)return;
    const auto physical=[&](glm::vec2 p){
        auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());
        return glm::ivec2(v.x+int(p.x*v.scale),v.y+int(p.y*v.scale));
    };
    const auto click=[&](glm::vec2 p){
        auto q=physical(p);mousePressed(q.x,q.y,0);mouseReleased(q.x,q.y,0);
    };
    const auto stroke=[&](cv::Point a,cv::Point b){
        auto p=physical(boardPoint(a)),q=physical(boardPoint(b));
        mousePressed(p.x,p.y,0);mouseDragged(q.x,q.y,0);mouseReleased(q.x,q.y,0);
    };
    const auto capture=[](const std::string& name){
        const auto folder=ofFilePath::getCurrentExeDir()+"../../../../qa/";
        ofDirectory::createDirectory(folder,true,true);ofSaveScreen(folder+name);
    };
    switch(qaStage){
        case 0:capture("qa-home.png");click(startButton.getCenter());++qaStage;break;
        case 1:
            click(boardPoint({100,150}));qaPassed&=!campaign.solved;
            stroke({150,150},{390,250});stroke({650,420},{800,480});
            qaPassed&=!campaign.solved;toastUntil=0;++qaStage;break;
        case 2:
            capture("qa-case1.png");click(boardPoint({784,479}));
            qaPassed&=campaign.solved;++qaStage;break;
        case 3:capture("qa-result.png");++qaStage;break;
        case 4:
            if(screen!=Screen::Play||campaign.chapter!=1)break;
            stroke({360,280},{400,294});click(boardPoint({400,294}));
            qaPassed&=!campaign.solved;
            stroke({150,460},{210,449});qaPassed&=!matchPolygon.empty();
            toastUntil=0;++qaStage;break;
        case 5:
            capture("qa-case2.png");click(boardPoint({210,449}));
            qaPassed&=campaign.solved;++qaStage;break;
        case 6:
            if(screen!=Screen::Play||campaign.chapter!=2)break;
            stroke({180,100},{700,250});++qaStage;break;
        case 7:{
            keyPressed(' ');auto point=mirror::LevelFactory::mothCenter(3,motionTick);
            stroke(point-cv::Point(35,0),point);++qaStage;break;
        }
        case 8:
            capture("qa-case3.png");click(boardPoint(mirror::LevelFactory::mothCenter(3,motionTick)));
            qaPassed&=campaign.solved;++qaStage;break;
        case 9:
            if(screen!=Screen::Complete)break;
            capture("qa-complete.png");
            ofLogNotice()<<"QA story ended: "<<(qaPassed?"ALL THREE CHAPTERS COMPLETED":"FAILED");
            ofExit(qaPassed?0:1);break;
    }
}
