#include "ofApp.h"

namespace {
using namespace pixel;
constexpr int W=mirror::canvasWidth,H=mirror::canvasHeight;
const ofRectangle startButton{244,300,152,40},homeButton{601,9,28,26};
const ofRectangle nextButton{398,332,84,35},cameraButton{182,329,148,35};
const ofRectangle recordButton{30,329,132,35},rotateButton{362,332,120,35};
const ofRectangle recordArea{42,61,428,254};
const std::array<std::string,3> titles{"THE RAIN-SOAKED RECORD","THE PHOTO CABINET","THE FIRST TOWN PHOTO"};
const std::array<std::string,3> hints{"Save our\nfirst town\nphoto!\n\nClean up\nthe code.","Pinch and\ndrag.\n\nLinked\nwheels\nturn too!","Match the\noverlap.\n\nTurn a\nphoto if\nneeded."};
const std::array<std::string,3> cheers{"Found it!\n\nLet's open\nthe box!","It's here!\n\nLet's fix\nthe photo!","We did it!\n\nOur town,\ntogether."};
void rect(int x,int y,int w,int h,ofColor c){ofSetColor(c);ofDrawRectangle(x,y,w,h);}
}

void ofApp::setup(){
    ofSetFrameRate(60);ofSetVerticalSync(true);ofEnableAlphaBlending();ofSetEscapeQuitsApp(false);
    cv::setNumThreads(2);ofDisableAntiAliasing();ofDisableSmoothing();ofSetWindowTitle("Mirror Archive");
    const auto resources=ofFilePath::getCurrentExeDir()+"../Resources/data/";
    if(ofDirectory::doesDirectoryExist(resources))ofSetDataPathRoot(resources);
    ofFbo::Settings settings;settings.width=W;settings.height=H;settings.internalformat=GL_RGBA;
    settings.textureTarget=GL_TEXTURE_2D;settings.numSamples=0;canvas.allocate(settings);
    canvas.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    try{
        if(!hero.load("art/pixel-cottage.png"))throw std::runtime_error("Missing workshop");
        hero.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        auto photo=cv::imread(ofToDataPath("art/town-photo.png",true));
        if(photo.empty())throw std::runtime_error("Missing town photograph");
        makeRecord(photo);puzzle.load(photo);
        for(int i=0;i<5;++i)upload(puzzle.pieces[i].image,photoTextures[i]);
    }catch(const std::exception& e){
        ofLogError()<<e.what();errorMessage="MISSING ART. PLEASE REBUILD.";screen=Screen::Error;
        if(qaMode)ofExit(1);
    }
}
void ofApp::exit(){hand.stop();}
void ofApp::makeRecord(const cv::Mat& photo){
    cleanRecord=cv::Mat(380,640,CV_8UC3,cv::Scalar(177,218,239));
    auto box=[&](int x,int y,int w,int h,cv::Scalar color){cv::rectangle(cleanRecord,{x,y,w,h},color,cv::FILLED);};
    auto inkText=[&](const std::string& value,int x,int y,int size,cv::Scalar color){
        for(char c:value){auto rows=pixel::glyph(c);for(int r=0;r<7;++r)for(int bit=0;bit<5;++bit)
            if(rows[r]&(1<<(4-bit)))box(x+bit*size,y+r*size,size,size,color);x+=6*size;}
    };
    const cv::Scalar inkColor(49,70,78),faint(118,151,172);
    box(12,12,616,356,{103,151,181});box(16,16,608,348,{177,218,239});
    inkText("TOWN ARCHIVE",42,37,5,inkColor);box(40,85,560,3,faint);
    cv::Mat thumb;cv::resize(photo,thumb,{230,154},0,0,cv::INTER_NEAREST);
    box(38,107,242,166,inkColor);thumb.copyTo(cleanRecord(cv::Rect(44,113,230,154)));
    inkText("FIRST",315,116,3,inkColor);inkText("TOWN PHOTO",315,148,3,inkColor);
    inkText("CABINET CODE",315,206,3,inkColor);
    inkText("314",326,248,9,inkColor);
    box(40,299,245,2,faint);box(40,317,209,2,faint);box(40,335,180,2,faint);
    box(537,312,53,28,{110,135,161});box(542,317,43,18,{156,183,201});
    // Mild blur remains recoverable; median filtering removes impulsive rain damage.
    cv::Mat faded;cv::GaussianBlur(cleanRecord,faded,{3,3},.65);
    faded.convertTo(damagedRecord,CV_8U,.32,87);
    cv::RNG rng(4187);
    for(int i=0;i<640*380/4;++i){int x=rng.uniform(0,640),y=rng.uniform(0,380);
        const uchar value=rng.uniform(0,2)?230:24;damagedRecord.at<cv::Vec3b>(y,x)={value,value,value};}
    refreshRecord();
}
void ofApp::upload(const cv::Mat& image,ofTexture& texture){
    cv::Mat rgb;cv::cvtColor(image,rgb,cv::COLOR_BGR2RGB);
    ofPixels pixels;pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);
    texture.loadData(pixels);texture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
}
void ofApp::refreshRecord(){repairedRecord=restoration.apply(damagedRecord);upload(repairedRecord,recordTexture);}
void ofApp::startStory(){
    hand.stop();cameraPending=false;cancelDrag();campaign.start();restoration={};lock={};recordOpen=false;exhibitReady=false;
    refreshRecord();
    const std::array<cv::Point2f,5> starts{{{242,178},{112,122},{393,117},{102,226},{398,226}}};
    const std::array<float,5> angles{0,180,0,90,270};
    for(int i=0;i<5;++i){auto& piece=puzzle.pieces[i];piece.center=starts[i];piece.angle=angles[i];piece.scale=.5;
        piece.fixed=i==0;piece.pose=piece.currentPose();}
    puzzle.sparkle.clear();order={0,1,2,3,4};selected=-1;screen=Screen::Play;hintUntil=ofGetElapsedTimef()+8;
}
void ofApp::solveChapter(){
    cancelDrag();hand.stop();cameraPending=false;handDown=false;campaign.solve();screen=Screen::Transition;
    transitionAt=ofGetElapsedTimef()+(qaMode?1.5f:3.2f);
    if(campaign.chapter==2){upload(puzzle.composite(),exhibitTexture);exhibitReady=true;}
}
void ofApp::advanceStory(){
    if(!campaign.advance())return;
    screen=campaign.complete?Screen::Complete:Screen::Play;hintUntil=ofGetElapsedTimef()+7;
}
void ofApp::update(){
    const float now=ofGetElapsedTimef();
    if(screen==Screen::Transition&&now>=transitionAt)advanceStory();
    if(cameraPending&&screen==Screen::Play&&campaign.chapter==1){
        const auto permission=mirror::cameraAccess();
        if(permission==mirror::CameraAccess::Allowed){
            cameraPending=false;hand.start(ofFilePath::getCurrentExeDir(),ofToDataPath("hand",true));
            cameraStartedAt=lastHandAt=now;hintUntil=now+7;
        }else if(permission==mirror::CameraAccess::Blocked){cameraPending=false;hand.error="CAMERA BLOCKED";}
    }
    if(screen==Screen::Play&&campaign.chapter==1&&hand.running){
        hand.poll([&](){
            lastHandAt=now;handCursor={hand.x*W,hand.y*H};
            if(!hand.visible){if(handDown)cancelDrag();handDown=false;return;}
            if(hand.pinched&&!handDown)pointerDown(handCursor,true);
            else if(hand.pinched&&handDown)pointerMove(handCursor);
            else if(!hand.pinched&&handDown)pointerUp(handCursor);
            handDown=hand.pinched;
        });
        if(handDown&&now-lastHandAt>.7f){cancelDrag();handDown=false;hand.visible=false;}
        if(!hand.ready&&now-cameraStartedAt>25){hand.stop();hand.error="CAMERA UNAVAILABLE";}
    }
    if(qaMode)runQA();
}
glm::vec2 ofApp::logical(float x,float y)const{
    const auto view=mirror::pixelViewport(ofGetWidth(),ofGetHeight());return {view.localX(x),view.localY(y)};
}
bool ofApp::over(const ofRectangle& r)const{return r.inside(logical(ofGetMouseX(),ofGetMouseY()));}
void ofApp::centered(const std::string& value,int x,int y,ofColor c,int s){pixel::text(value,x-pixel::textWidth(value,s)/2,y,c,s);}
void ofApp::button(const std::string& value,const ofRectangle& r,bool active,int scale){
    pixel::panel(r.x,r.y,r.width,r.height,active?pixel::gold:(over(r)?pixel::light:pixel::paper));
    centered(value,r.getCenter().x,r.y+(r.height-7*scale)/2,pixel::ink,scale);
}
void ofApp::drawDesk(){
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(39,25,35,150));
    pixel::panel(9,45,484,278,ofColor(111,70,51));
    for(int y=54;y<317;y+=33){rect(15,y,472,2,ofColor(139,88,57));rect(15,y+2,472,1,ofColor(76,49,44));}
    for(int y=66;y<317;y+=66)for(int x=30+(y%5)*30;x<480;x+=130){rect(x,y,28,1,ofColor(159,100,62));rect(x+12,y+5,21,1,ofColor(87,55,43));}
}
void ofApp::drawHome(){
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(32,23,33,32));
    pixel::panel(140,35,360,69,ofColor(70,52,47));
    centered("MIRROR ARCHIVE",320,52,pixel::light,3);
    pixel::tonton(320,242,4,ofGetElapsedTimef());
    button("START",startButton,true,2);
}
void ofApp::drawRecord(){
    pixel::panel(36,55,440,266,pixel::paper);
    ofSetColor(255);recordTexture.draw(recordArea);
    for(int i=0;i<2;++i){
        int x=i==0?30:218;const float value=i==0?restoration.clean:restoration.clarity;
        pixel::text(i==0?"CLEAN":"CLEAR",x,332,pixel::light,2);
        rect(x,359,146,5,pixel::ink);rect(x+1,360,int(144*value),3,pixel::gold);
        pixel::panel(x+int(144*value)-5,352,13,18,pixel::light);
    }
    button("KEEP",nextButton,true,2);
}
void ofApp::drawLock(){
    // Brass cabinet: the linked gear axles make the coupling visible.
    pixel::panel(44,64,414,243,ofColor(126,72,43));
    pixel::panel(56,76,390,219,ofColor(168,106,57));
    for(int x=65;x<438;x+=48){rect(x,82,2,207,ofColor(132,78,45));rect(x+3,82,1,207,ofColor(193,129,68));}
    pixel::panel(139,85,222,33,pixel::paper);centered("PHOTO ARCHIVE",250,95,pixel::ink,2);
    pixel::panel(94,140,314,110,ofColor(65,48,42));
    for(int i=0;i<2;++i){
        const int x=192+i*92;
        rect(x-6,188,29,7,pixel::gold);
        for(int k=0;k<4;++k){rect(x-4+k*7,184,4,15,pixel::wood);rect(x-3+k*7,186,2,10,pixel::gold);}
        pixel::text(">",x+4,263,pixel::light,2);
    }
    for(int i=0;i<3;++i){
        const int x=111+i*92;
        pixel::panel(x,151,72,87,wheel==i?pixel::gold:pixel::paper);
        centered(std::to_string((lock.digits[i]+9)%10),x+36,158,pixel::wood,1);
        centered(std::to_string(lock.digits[i]),x+36,179,pixel::ink,4);
        centered(std::to_string((lock.digits[i]+1)%10),x+36,222,pixel::wood,1);
        rect(x+8,173,56,1,pixel::wood);rect(x+8,216,56,1,pixel::wood);
    }
    for(auto p:std::vector<glm::ivec2>{{69,87},{430,87},{69,282},{430,282}}){rect(p.x-3,p.y-3,6,6,pixel::edge);rect(p.x-2,p.y-2,4,4,pixel::gold);}
    button("RECORD",recordButton,false,2);
    button(hand.running||cameraPending?"CAM OFF":"CAMERA",cameraButton,hand.running||cameraPending,2);
    if(cameraPending)centered("ALLOW CAMERA",407,343,pixel::light,1);
    if(hand.running){
        const std::string status=hand.visible?"PINCH + DRAG":(hand.ready?"SHOW HAND":"STARTING...");
        centered(status,407,343,pixel::light,1);
        // A live landmark silhouette gives feedback without exposing a camera feed.
        if(hand.visible){
            const std::array<std::pair<int,int>,20> edges{{{0,1},{1,2},{2,3},{3,4},{0,5},{5,6},{6,7},{7,8},{5,9},{9,10},{10,11},{11,12},{9,13},{13,14},{14,15},{15,16},{13,17},{17,18},{18,19},{19,20}}};
            ofSetColor(pixel::light);ofSetLineWidth(1);
            for(auto [a,b]:edges)ofDrawLine(int(365+hand.joints[a][0]*70),int(251+hand.joints[a][1]*45),int(365+hand.joints[b][0]*70),int(251+hand.joints[b][1]*45));
        }
    }else if(!cameraPending&&!hand.error.empty())centered(hand.error,404,341,pixel::light,1);
    if(recordOpen){
        rect(16,47,475,275,ofColor(39,25,35,180));pixel::panel(35,58,438,261,pixel::paper);
        ofSetColor(255);recordTexture.draw(recordArea);
        button("REPAIR",{182,329,148,35},true,2);
    }
}
void ofApp::drawCompanion(int happy){
    const float now=ofGetElapsedTimef();
    if(happy||now<hintUntil){
        pixel::panel(499,62,133,154,pixel::paper);
        auto line=happy?cheers[campaign.chapter]:hints[campaign.chapter];
        if(!happy&&campaign.chapter==1&&!hand.running)line="Drag a\nwheel.\n\nNeighbors\nturn too!";
        pixel::text(line,508,77,pixel::ink,2);
        rect(555,216,11,5,pixel::ink);rect(557,213,7,6,pixel::paper);
    }
    pixel::tonton(565,260,3,now,happy);
    centered("tonton",565,320,pixel::light,2);
    if(!happy)button("?",{610,333,22,24},false,2);
}
void ofApp::drawPlay(){
    drawDesk();pixel::panel(9,5,584,35,pixel::paper);
    pixel::text(std::to_string(campaign.chapter+1)+"/3",21,17,pixel::wood,1);
    centered(titles[campaign.chapter],315,16,pixel::ink,2);
    button("<",homeButton,false,2);
    if(campaign.chapter==0)drawRecord();
    else if(campaign.chapter==1)drawLock();
    else {
        // Light table first, registered scans next, loose photos last.
        pixel::panel(14,50,474,272,ofColor(196,206,179));
        for(int y=57;y<317;y+=16)for(int x=21;x<482;x+=16)rect(x,y,1,1,ofColor(158,179,157));
        if(exhibitReady){ofSetColor(255);exhibitTexture.draw(0,0);}
        else for(const auto& p:puzzle.pieces)if(p.fixed){
            int i=int(&p-&puzzle.pieces[0]);ofPushMatrix();ofTranslate(std::round(p.center.x),std::round(p.center.y));ofRotateDeg(p.angle);
            ofSetColor(255);photoTextures[i].draw(-p.image.cols*p.scale/2,-p.image.rows*p.scale/2,p.image.cols*p.scale,p.image.rows*p.scale);ofPopMatrix();
        }
        // Loose scans draw above the already registered photo.
        for(int index:order){const auto& p=puzzle.pieces[index];if(p.fixed)continue;
            ofPushMatrix();ofTranslate(std::round(p.center.x),std::round(p.center.y));ofRotateDeg(p.angle);
            int w=int(p.image.cols*p.scale),h=int(p.image.rows*p.scale);
            rect(-w/2-1,-h/2,w+8,h+8,ofColor(39,25,35,90));
            rect(-w/2-4,-h/2-4,w+8,h+8,selected==index?pixel::gold:pixel::paper);
            ofSetColor(255);photoTextures[index].draw(-w/2,-h/2,w,h);ofPopMatrix();
        }
        centered(std::to_string(puzzle.count())+"/5",74,344,pixel::light,2);
        button("TURN",rotateButton,selected>0,2);
        if(ofGetElapsedTimef()<sparkleUntil)for(auto p:puzzle.sparkle)pixel::sprite(2,int(p.x),int(p.y),1,pixel::gold);
    }
    drawCompanion();
    if(hand.running&&hand.visible&&!recordOpen){
        pixel::sprite(hand.pinched?9:0,int(handCursor.x),int(handCursor.y),1,hand.pinched?pixel::gold:pixel::light);
    }
}
void ofApp::drawTransition(){
    drawPlay();
    rect(17,52,470,267,ofColor(39,25,35,185));
    pixel::panel(70,101,368,168,pixel::paper);
    if(campaign.chapter==0){
        pixel::sprite(9,250,145,3,pixel::green);centered("RECORD SAVED",254,207,pixel::ink,2);
    }else if(campaign.chapter==1){
        for(int i=0;i<3;++i){pixel::panel(145+i*40,124+i*10,126,88,pixel::light);
            ofSetColor(255);photoTextures[i+1].draw(151+i*40,130+i*10,114,76);}
        centered("PHOTO FOUND",254,237,pixel::ink,2);
    }else{
        centered("READY TO EXHIBIT",254,210,pixel::ink,2);
        pixel::sprite(9,250,149,3,pixel::green);
    }
    rect(495,45,145,339,ofColor(39,25,35,235));drawCompanion(campaign.chapter+1);
}
void ofApp::drawComplete(){
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(39,25,35,110));
    pixel::panel(130,46,380,260,pixel::wood);
    // Crop the assembled image's bounding area; all pixels come from warped scans.
    ofSetColor(255);exhibitTexture.drawSubsection(140,56,360,240,82,68,320,213);
    pixel::panel(156,14,328,34,pixel::paper);centered("BACK WHERE WE BELONG",320,25,pixel::ink,2);
    pixel::tonton(550,294,4,ofGetElapsedTimef(),3);
    button("HOME",{244,333,152,35},true,2);
    const float t=ofGetElapsedTimef();
    for(int i=0;i<28;++i){int x=(i*89+23)%630,y=int(t*27+i*41)%320;rect(x,y,3,5,i%2?pixel::gold:pixel::teal);}
}
void ofApp::draw(){
    canvas.begin();ofClear(pixel::shadow);ofSetColor(255);
    switch(screen){
        case Screen::Home:drawHome();break;
        case Screen::Play:drawPlay();break;
        case Screen::Transition:drawTransition();break;
        case Screen::Complete:drawComplete();break;
        case Screen::Error:centered(errorMessage,320,180,pixel::light,2);break;
    }
    canvas.end();ofBackground(pixel::shadow);
    auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());ofSetColor(255);
    canvas.draw(v.x,v.y,W*v.scale,H*v.scale);
}
void ofApp::cancelDrag(){dragging=slider=wheel=-1;wheelSteps=0;}
void ofApp::pointerDown(glm::vec2 p,bool camera){
    if(screen==Screen::Home){if(startButton.inside(p))startStory();return;}
    if(screen==Screen::Complete){if(ofRectangle(244,333,152,35).inside(p))screen=Screen::Home;return;}
    if(screen!=Screen::Play)return;
    if(camera){
        if(campaign.chapter!=1||recordOpen)return;
    }else{
        if(homeButton.inside(p)){hand.stop();cameraPending=false;cancelDrag();screen=Screen::Home;return;}
        if(ofRectangle(499,222,135,146).inside(p)){hintUntil=ofGetElapsedTimef()+7;return;}
    }
    if(campaign.chapter==0){
        for(int i=0;i<2;++i)if(ofRectangle(i==0?24:212,350,158,29).inside(p)){slider=i;pointerMove(p);return;}
        if(nextButton.inside(p)){solveChapter();return;}
    }else if(campaign.chapter==1){
        if(!camera&&recordButton.inside(p)){cancelDrag();recordOpen=!recordOpen;return;}
        if(recordOpen){
            if(cameraButton.inside(p)){hand.stop();cameraPending=false;campaign.chapter=0;campaign.solved=false;recordOpen=false;hintUntil=ofGetElapsedTimef()+5;}
            return;
        }
        if(!camera&&cameraButton.inside(p)){
            cancelDrag();handDown=false;
            if(hand.running||cameraPending){hand.stop();cameraPending=false;}
            else {hand.error.clear();cameraPending=true;if(mirror::cameraAccess()==mirror::CameraAccess::Unknown)mirror::requestCameraAccess();}
            return;
        }
        for(int i=0;i<3;++i)if(ofRectangle(111+i*92,151,72,87).inside(p)){wheel=i;wheelStart=p.x;wheelSteps=0;return;}
    }else{
        if(rotateButton.inside(p)){rotateSelected();return;}
        for(auto it=order.rbegin();it!=order.rend();++it)if(!puzzle.pieces[*it].fixed&&puzzle.pieces[*it].contains({p.x,p.y})){
            dragging=selected=*it;auto center=puzzle.pieces[*it].center;dragOffset={center.x-p.x,center.y-p.y};
            order.erase(std::find(order.begin(),order.end(),dragging));order.push_back(dragging);return;
        }
    }
}
void ofApp::pointerMove(glm::vec2 p){
    if(screen!=Screen::Play)return;
    if(slider>=0){float value=ofClamp((p.x-(slider==0?30:218))/144.f,0,1);
        if(slider==0)restoration.clean=value;else restoration.clarity=value;refreshRecord();}
    if(wheel>=0){const int steps=int(std::round((p.x-wheelStart)/24.f));
        lock.turn(wheel,steps-wheelSteps);wheelSteps=steps;}
    if(dragging>=0){
        auto& piece=puzzle.pieces[dragging];float angle=piece.angle*DEG_TO_RAD;
        float hw=(std::abs(std::cos(angle))*piece.image.cols+std::abs(std::sin(angle))*piece.image.rows)*piece.scale/2;
        float hh=(std::abs(std::sin(angle))*piece.image.cols+std::abs(std::cos(angle))*piece.image.rows)*piece.scale/2;
        piece.center={ofClamp(p.x+dragOffset.x,20+hw,482-hw),ofClamp(p.y+dragOffset.y,56+hh,316-hh)};
    }
}
void ofApp::acceptSnap(int index){
    if(puzzle.snap(index)){sparkleUntil=ofGetElapsedTimef()+.8f;selected=-1;
        if(puzzle.count()==5)solveChapter();}
}
void ofApp::pointerUp(glm::vec2 p){
    if(screen!=Screen::Play){cancelDrag();return;}
    pointerMove(p);int piece=dragging;bool turned=wheel>=0;cancelDrag();
    if(piece>=0)acceptSnap(piece);
    if(turned&&lock.solved())solveChapter();
}
void ofApp::rotateSelected(){
    if(selected<1||puzzle.pieces[selected].fixed)return;
    auto& p=puzzle.pieces[selected];p.angle=std::fmod(p.angle+90,360.f);
    float angle=p.angle*DEG_TO_RAD;
    float hw=(std::abs(std::cos(angle))*p.image.cols+std::abs(std::sin(angle))*p.image.rows)*p.scale/2;
    float hh=(std::abs(std::sin(angle))*p.image.cols+std::abs(std::cos(angle))*p.image.rows)*p.scale/2;
    p.center.x=ofClamp(p.center.x,20+hw,482-hw);p.center.y=ofClamp(p.center.y,56+hh,316-hh);
    acceptSnap(selected);
}
void ofApp::mousePressed(int x,int y,int b){if(b==0){if(handDown)cancelDrag();handDown=false;pointerDown(logical(x,y));}}
void ofApp::mouseDragged(int x,int y,int b){if(b==0)pointerMove(logical(x,y));}
void ofApp::mouseReleased(int x,int y,int b){if(b==0)pointerUp(logical(x,y));}
void ofApp::keyPressed(int key){
    if(key==OF_KEY_ESC){
        if(recordOpen){recordOpen=false;return;}
        hand.stop();cameraPending=false;cancelDrag();if(screen!=Screen::Error)screen=Screen::Home;
    }else if(key==OF_KEY_RETURN&&screen==Screen::Home)startStory();
    else if((key=='r'||key=='R')&&screen==Screen::Play&&campaign.chapter==2)rotateSelected();
}
void ofApp::windowResized(int width,int height){
    if(width<W||height<H)ofSetWindowShape(std::max(width,W),std::max(height,H));
}

void ofApp::runQA(){
    if(++qaFrame>2400){ofLogError()<<"QA timed out at "<<qaStage;ofExit(1);return;}
    if(qaFrame%24)return;
    const auto physical=[&](glm::vec2 p){auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());return glm::ivec2(v.x+int(p.x*v.scale),v.y+int(p.y*v.scale));};
    const auto click=[&](glm::vec2 p){auto q=physical(p);mousePressed(q.x,q.y,0);mouseReleased(q.x,q.y,0);};
    const auto drag=[&](glm::vec2 a,glm::vec2 b){auto p=physical(a),q=physical(b);mousePressed(p.x,p.y,0);mouseDragged(q.x,q.y,0);mouseReleased(q.x,q.y,0);};
    const auto capture=[&](const std::string& name){const auto folder=ofFilePath::getCurrentExeDir()+"../../../../qa/";
        ofDirectory::createDirectory(folder,true,true);ofSaveScreen(folder+name);};
    switch(qaStage){
        case 0:capture("qa-home.png");click(startButton.getCenter());++qaStage;break;
        case 1:capture("qa-damaged.png");drag({30,360},{174,360});drag({218,360},{350,360});++qaStage;break;
        case 2:capture("qa-case1.png");click(nextButton.getCenter());qaPassed&=campaign.solved;++qaStage;break;
        case 3:if(screen!=Screen::Play||campaign.chapter!=1)break;capture("qa-case2.png");
            drag({147,190},{195,190});drag({239,190},{215,190});qaPassed&=!campaign.solved;++qaStage;break;
        case 4:click(recordButton.getCenter());++qaStage;break;
        case 5:capture("qa-record.png");click(recordButton.getCenter());drag({331,190},{283,190});qaPassed&=campaign.solved;++qaStage;break;
        case 6:if(screen!=Screen::Play||campaign.chapter!=2)break;capture("qa-case3.png");++qaStage;break;
        case 7:{
            if(qaPiece==5){qaPassed&=campaign.solved;++qaStage;break;}
            auto& piece=puzzle.pieces[qaPiece];
            // Select through an uncovered point, then rotate through the real UI.
            glm::vec2 grab;bool found=false;
            for(int y=60;y<315&&!found;y+=4)for(int x=22;x<482&&!found;x+=4){
                int top=-1;for(auto it=order.rbegin();it!=order.rend();++it)
                    if(!puzzle.pieces[*it].fixed&&puzzle.pieces[*it].contains({float(x),float(y)})){top=*it;break;}
                if(top==qaPiece){grab={x,y};found=true;}
            }
            if(!found){qaPassed=false;ofExit(1);break;}
            click(grab);
            for(int turns=0;turns<4&&std::abs(piece.angle)>.1f;++turns)click(rotateButton.getCenter());
            auto expected=puzzle.expectedPose(qaPiece);
            if(expected.empty()){qaPassed=false;ofExit(1);break;}
            auto target=mirror::mapped(expected,{piece.image.cols/2.f,piece.image.rows/2.f});
            // Fresh hit point after rotation; move the selected scan via public mouse events.
            glm::vec2 from{piece.center.x,piece.center.y};
            drag(from,{target.x+3,target.y-2});
            qaPassed&=piece.fixed;++qaPiece;break;
        }
        case 8:if(screen!=Screen::Complete)break;capture("qa-complete.png");
            ofLogNotice()<<"QA story: "<<(qaPassed?"ALL THREE CHAPTERS COMPLETED":"FAILED");ofExit(qaPassed?0:1);break;
    }
}
