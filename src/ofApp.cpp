#include "ofApp.h"

namespace {
using namespace pixel;
constexpr int W=640,H=384;
const ofRectangle startButton{236,258,168,44},homeButton{580,8,48,26};
const ofRectangle photoArea{128,44,384,256},nextButton{459,343,165,30};
const ofRectangle cameraButton{489,224,132,28},referenceButton{489,278,132,52};
const ofRectangle strengthTrack{365,346,75,24},beforeButton{127,346,80,24};
const ofRectangle detailPhotoButton{65,325,172,28},detailBackButton{458,325,112,28};
const ofRectangle undoButton{52,346,54,24},smallerBrush{338,314,24,24},largerBrush{416,314,24,24};
const ofRectangle copyButton{160,315,150,36},goHomeButton{330,315,150,36};
ofRectangle filterButton(int i){return {float(52+i*75),314,70,24};}
const ofRectangle cameraPreview{489,77,132,99};
ofRectangle directionButton(int index){
    const std::array<ofRectangle,4> areas{{{489,111,30,30},{591,111,30,30},{540,77,30,26},{540,150,30,26}}};
    return areas[index];
}
const ofRectangle castButton{176,318,130,30},aboutButton{334,318,130,30},infoBack{520,43,68,28},soundButton{594,350,30,26};
const ofRectangle envelopeArea{180,65,280,185},openEnvelopeButton{236,306,168,32};
const ofRectangle letterArea{70,39,500,290};
const cv::Rect address{213,71,50,17};
const std::array<std::string,3> titles{"Restore the photo","Open the box","Guide the letter"};
ofRectangle inkArea(int i){return {float(145+i*126),137,90,108};}
void rect(float x,float y,float w,float h,ofColor c){ofSetColor(c);ofDrawRectangle(x,y,w,h);}
void line(int x,int y,int xx,int yy,ofColor c){ofSetColor(c);ofDrawLine(x,y,xx,yy);}
glm::vec2 mapPoint(cv::Point p){return {float(35+p.x*32),float(91+p.y*27)};}
void paperFrame(int x,int y,int w,int h,ofColor fill=ofColor(247,229,191)){
    rect(x+3,y+4,w,h,ofColor(36,25,28,100));rect(x,y,w,h,ofColor(103,70,47));
    rect(x+2,y+2,w-4,h-4,fill);rect(x+4,y+4,w-8,1,ofColor(255,244,218));
    for(int px:{x+4,x+w-6})for(int py:{y+5,y+h-7})rect(px,py,2,2,ofColor(192,147,87));
}
void tree(int x,int y,int seed){
    rect(x-2,y+6,4,11,ofColor(113,89,51));rect(x,y+6,2,10,ofColor(158,117,64));
    const std::array<ofColor,5> greens{ofColor(76,101,58),ofColor(95,118,62),ofColor(117,136,70),ofColor(139,151,80),ofColor(161,164,94)};
    for(int row=-7;row<=6;++row)for(int col=-6;col<=6;++col){
        if(col*col*1.25+row*row>42+(row%3)*2)continue;
        int shade=std::clamp(2+(col+row<-3?1:0)+(col+row>5?-1:0)+((col*13+row*7+seed+200)%5==0?1:0),0,4);
        rect(x+col*2,y+row*2,2,2,greens[shade]);
    }
}

}

void ofApp::setup(){
    ofSetFrameRate(60);ofSetVerticalSync(true);ofEnableAlphaBlending();ofSetEscapeQuitsApp(false);
    cv::setNumThreads(2);ofDisableAntiAliasing();ofDisableSmoothing();ofSetWindowTitle("Mirror Archive - A Light Left On");
    const auto resources=ofFilePath::getCurrentExeDir()+"../Resources/data/";
    if(ofDirectory::doesDirectoryExist(resources))ofSetDataPathRoot(resources);
    try{
        if(!hero.load("art/pixel-cottage.png"))throw std::runtime_error("Missing workshop art");
        hero.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        if(!lia.load("art/lia-portraits.png")||!mapArt.load("art/garden-map.png"))throw std::runtime_error("Missing story art");
        lia.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);mapArt.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        // Keep scene art on its logical pixel grid; text has a separate raster size.
        auto sample=[](ofImage& art,int width,int height){art.getPixels().resize(width,height,OF_INTERPOLATE_NEAREST_NEIGHBOR);art.update();art.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);};
        sample(hero,640,384);sample(lia,264,88);sample(mapArt,452,294);
        auto trim=[](ofImage& art,int width){
            auto& pixels=art.getPixels();int left=pixels.getWidth(),top=pixels.getHeight(),right=-1,bottom=-1;
            for(int y=0;y<int(pixels.getHeight());++y)for(int x=0;x<int(pixels.getWidth());++x)if(pixels.getColor(x,y).a>16){left=std::min(left,x);top=std::min(top,y);right=std::max(right,x);bottom=std::max(bottom,y);}
            if(right<left)throw std::runtime_error("Empty prop art");art.crop(left,top,right-left+1,bottom-top+1);
            int height=int(std::round(width*art.getHeight()/art.getWidth()));art.getPixels().resize(width,height,OF_INTERPOLATE_NEAREST_NEIGHBOR);art.update();
            art.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        };
        ofPixels houseSheet;if(!ofLoadImage(houseSheet,"art/village-buildings.png"))throw std::runtime_error("Missing village houses");
        for(int i=0;i<3;++i){ofPixels cell;houseSheet.cropTo(cell,i*houseSheet.getWidth()/3,0,houseSheet.getWidth()/3,houseSheet.getHeight());houses[i].setFromPixels(cell);trim(houses[i],200);}
        ofPixels chestSheet;if(!ofLoadImage(chestSheet,"art/keepsake-chests.png"))throw std::runtime_error("Missing chest art");
        for(int i=0;i<2;++i){ofPixels cell;chestSheet.cropTo(cell,i*chestSheet.getWidth()/2,0,chestSheet.getWidth()/2,chestSheet.getHeight());chests[i].setFromPixels(cell);trim(chests[i],400);}
        if(!envelopeArt.load("art/keepsake-envelope.png")||!paperArt.load("art/letter-paper.png")||!titleArt.load("art/archive-sign.png"))throw std::runtime_error("Missing prop art");
        trim(envelopeArt,144);trim(paperArt,500);trim(titleArt,478);
        if(!doorbell.load("audio/doorbell.wav"))throw std::runtime_error("Missing doorbell sound");doorbell.setVolume(qaMode?.08f:.3f);
        fontFor(11);fontFor(22);fontFor(33);
        auto photo=cv::imread(ofToDataPath("art/family-home.png",true));
        if(photo.empty())throw std::runtime_error("Missing family photograph");
        cv::resize(photo,sourcePhoto,{480,320},0,0,cv::INTER_NEAREST);
        // The address is part of the observation before degradation, not a UI overlay.
        cv::rectangle(sourcePhoto,address,{161,216,246},cv::FILLED);
        int x=221;for(char c:std::string("208")){auto glyph=pixel::glyph(c);
            for(int y=0;y<7;++y)for(int bit=0;bit<5;++bit)if(glyph[y]&(1<<(4-bit)))
                cv::rectangle(sourcePhoto,{x+bit*2,73+y*2,2,2},{34,44,54},cv::FILLED);
            x+=12;
        }
        repair.load(sourcePhoto,{address.x-10,address.y-10,address.width+20,address.height+20});upload(repair.damaged,damagedTexture);refreshPhoto();reader.load(ofToDataPath("digits/digits.png",true));
        for(int i=0;i<3;++i){writing[i]=cv::Mat::zeros(108,90,CV_8U);refreshInk(i);}
    }catch(const std::exception& e){ofLogError()<<e.what();errorMessage="MISSING RESOURCES";screen=Screen::Error;if(qaMode)ofExit(1);}
}
void ofApp::exit(){stopCamera();ofShowCursor();}
void ofApp::upload(const cv::Mat& image,ofTexture& texture){
    cv::Mat rgb;cv::cvtColor(image,rgb,cv::COLOR_BGR2RGB);ofPixels pixels;pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);
    texture.loadData(pixels);texture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
}
void ofApp::refreshPhoto(){upload(repair.image(),photoTexture);}
void ofApp::refreshInk(int i){
    cv::Mat display(writing[i].size(),CV_8UC3,cv::Scalar(184,239,255));
    display.setTo(cv::Scalar(32,35,63),writing[i]);upload(display,writingTextures[i]);
}
void ofApp::openEnvelope(){
    if(screen!=Screen::Envelope||envelopeOpened)return;
    envelopeOpened=true;say(story::foundMap());
}
bool ofApp::saveCopy(const std::string& path){
    if(returnedPhoto.empty())return false;
    try{if(!cv::imwrite(path,returnedPhoto))throw std::runtime_error("Could not write photo");
        copySaved=true;notice="Your photo is saved.";noticeUntil=ofGetElapsedTimef()+4;return true;
    }catch(const std::exception& e){ofLogError()<<"Photo export failed: "<<e.what();notice="Could not save. Try another folder.";noticeUntil=ofGetElapsedTimef()+5;return false;}
}
void ofApp::beginStory(){
    cameraTaught=cameraTipPending=envelopeOpened=false;copySaved=false;
    stopCamera();resumeCamera=false;adjustingStrength=comparing=false;inspectedHouse=-1;chapter=0;repair.reset();refreshPhoto();maze.reset();delivered=false;photoOpen=false;brushing=false;drawingCell=-1;
    recognized={-1,-1,-1};inkDue={0,0,0};notice.clear();moveDuration=0;brushRadius=19;filterIndex=0;intro=true;exhibitAt=-1;revisiting=false;lockAttempts=0;wrongHomeHint=false;trail.clear();
    for(int i=0;i<3;++i){writing[i].setTo(0);guesses[i]={};refreshInk(i);}
    outro=false;letterOpened=false;giftReturned=false;giftPhoto.clear();returnedPhoto.release();bellsPlayed=0;bellAt=-100;returnPhotoAt=liaLeftAt=-1;
    say(story::opening());
}
void ofApp::say(std::vector<Speech> lines,Screen next){
    dialogue=std::move(lines);speechIndex=0;afterDialogue=next;screen=Screen::Dialogue;dialogueAt=ofGetElapsedTimef();
    brushing=adjustingStrength=comparing=false;drawingCell=-1;palm.reset();lastPalmStamp=0;enterSpeech();
}
void ofApp::enterSpeech(){
    switch(dialogue[speechIndex].cue){
        case story::Cue::Doorbell:bellAt=ofGetElapsedTimef();++bellsPlayed;if(soundOn)doorbell.play();break;
        case story::Cue::ShowKeepsakes:exhibitAt=ofGetElapsedTimef();break;
        case story::Cue::ReturnPhoto:returnedPhoto=repair.image();upload(returnedPhoto,giftPhoto);giftReturned=true;returnPhotoAt=ofGetElapsedTimef();break;
        case story::Cue::LiaLeaves:liaLeftAt=ofGetElapsedTimef();break;
        case story::Cue::None:break;
    }
}
void ofApp::nextSpeech(){
    if(dialogue[speechIndex].focus!=story::Focus::None&&ofGetElapsedTimef()-dialogueAt<.35f)return;
    if(dialogue[speechIndex].cue==story::Cue::Doorbell&&ofGetElapsedTimef()-dialogueAt<.75f)return;
    if(++speechIndex>=dialogue.size()){
        screen=afterDialogue;intro=false;palm.reset();lastPalmStamp=0;
        if(screen==Screen::Letter){letterOpened=false;letterAt=ofGetElapsedTimef();}
        if(screen==Screen::Complete)outro=false;
    }else{dialogueAt=ofGetElapsedTimef();enterSpeech();}
}
void ofApp::openLetter(){if(!letterOpened){letterOpened=true;letterAt=ofGetElapsedTimef();}}
void ofApp::beginEnding(){outro=true;returnPhotoAt=liaLeftAt=-1;say(story::ending(repair.peopleRestored()),Screen::Complete);}
void ofApp::finishChapter(){
    cameraTipPending=false;photoOpen=false;
    if(chapter==0){
        if(revisiting){chapter=returnChapter;revisiting=false;if(resumeCamera){resumeCamera=false;toggleCamera();}return;}
        chapter=1;say(story::foundNumber());
    }else if(chapter==1){chapter=2;maze.reset();trail.clear();say(story::foundEnvelope(),Screen::Envelope);
    }else {delivered=true;stopCamera();say(story::reachedHome(),Screen::Letter);}

}
void ofApp::revisitPhoto(int from){
    returnChapter=from;resumeCamera=from==2&&(hand.running||cameraPending);
    stopCamera();inspectedHouse=-1;photoOpen=false;brushing=adjustingStrength=comparing=false;
    chapter=0;revisiting=true;
}
void ofApp::toggleCamera(){
    if(hand.running||cameraPending){stopCamera();return;}
    cameraPending=true;notice.clear();
    if(mirror::cameraAccess()==mirror::CameraAccess::Unknown)mirror::requestCameraAccess();
}
void ofApp::stopCamera(){cameraPending=false;hand.stop();cameraTexture.clear();palm.reset();lastPalmStamp=0;previewFrame=0;}
std::array<bool,4> ofApp::exits()const{
    std::array<bool,4> allowed{};for(int i=0;i<4;++i)allowed[i]=maze.road(maze.position+homeward::step(homeward::directions[i]));return allowed;
}
void ofApp::updateCamera(float now){
    if(cameraPending){
        auto permission=mirror::cameraAccess();
        if(permission==mirror::CameraAccess::Blocked){cameraPending=false;notice="Camera access is off.";noticeUntil=now+5;}
        else if(permission==mirror::CameraAccess::Allowed){
            cameraPending=false;hand.start(ofFilePath::getCurrentExeDir(),ofToDataPath("hand",true));lastCameraFrame=now;previewFrame=0;palm.reset();lastPalmStamp=0;
            if(!hand.error.empty()){notice=hand.error;noticeUntil=now+6;}
        }
    }
    if(!hand.running)return;
    hand.poll([&](){
        lastCameraFrame=now;double dt=lastPalmStamp>0?hand.stamp-lastPalmStamp:0;lastPalmStamp=hand.stamp;
        if(screen!=Screen::Play||photoOpen||inspectedHouse>=0||delivered||moveDuration>0){palm.reset();return;}
        auto direction=palm.update(hand.x,hand.y,hand.visible,dt,exits());
        if(direction!=homeward::Direction::None){lastWave=direction;waveAt=now;moveEnvelope(direction);}
    });
    if(hand.frames!=previewFrame&&!hand.preview.empty()){
        ofBuffer buffer(hand.preview.data(),hand.preview.size());ofPixels pixels;
        if(ofLoadImage(pixels,buffer)){cameraTexture.loadData(pixels);cameraTexture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);++cameraFrames;lastCameraFrame=now;
            if(!cameraTaught)cameraTipPending=true;}
        previewFrame=hand.frames;
    }
    if(!hand.running&&!hand.error.empty()){notice="Camera stopped. Try Camera again.";noticeUntil=now+6;palm.reset();}
    else if(now-lastCameraFrame>(hand.ready?6:35)){stopCamera();notice="Camera is not ready. Try again.";noticeUntil=now+6;}
}
void ofApp::moveEnvelope(homeward::Direction direction){
    if(screen!=Screen::Play||chapter!=2||photoOpen||inspectedHouse>=0||delivered||moveDuration>0)return;
    moveFrom=maze.position;movePath=maze.route(maze.position,direction);
    if(!movePath.empty()){maze.position=movePath.back();moveAt=ofGetElapsedTimef();moveDuration=.16f+float(movePath.size())*.13f;trail.push_back(moveFrom);trail.insert(trail.end(),movePath.begin(),movePath.end());palm.reset();lastPalmStamp=0;}
}
void ofApp::update(){
    float now=ofGetElapsedTimef();
    if(chapter==2&&(screen==Screen::Play||screen==Screen::Dialogue))updateCamera(now);
    if(moveDuration>0&&screen==Screen::Play&&now>=moveAt+moveDuration){moveDuration=0;
        if(maze.solved())finishChapter();
        else if(maze.house(maze.position)&&!wrongHomeHint){wrongHomeHint=true;say(story::wrongHouse());}
    }
    for(int i=0;i<3;++i)if(inkDue[i]>0&&now>=inkDue[i]&&drawingCell!=i){guesses[i]=reader.guess(writing[i]);recognized[i]=guesses[i].digit;inkDue[i]=0;}
    if(screen==Screen::Play&&chapter==2&&moveDuration<=0&&inspectedHouse<0&&cameraTipPending){
        cameraTaught=true;cameraTipPending=false;
        say(story::cameraGuide());
    }
    if(qaMode)runQA();
}
glm::vec2 ofApp::logical(float x,float y)const{auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());return {v.localX(x),v.localY(y)};}
bool ofApp::over(const ofRectangle& r)const{return r.inside(logical(ofGetMouseX(),ofGetMouseY()));}
void ofApp::centered(const std::string& s,int x,int y,ofColor c,int scale){pixel::text(s,x-pixel::textWidth(s,scale)/2,y,c,scale);}
ofTrueTypeFont& ofApp::fontFor(int pixels){
    auto found=fonts.find(pixels);if(found!=fonts.end())return found->second;
    auto& font=fonts[pixels];ofTrueTypeFontSettings settings("fonts/DepartureMono-Regular.otf",pixels);
    settings.antialiased=false;settings.dpi=72;
    if(!font.load(settings))throw std::runtime_error("Missing story font");
    const_cast<ofTexture&>(font.getFontTexture()).setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);return font;
}
float ofApp::bodyWidth(const std::string& text){return fontFor(11*drawScale).stringWidth(text)/drawScale;}
void ofApp::rasterText(const std::string& text,float x,float baseline,ofColor color,bool speech){
    // The font is drawn directly into display pixels at an exact 11px multiple.
    // It never passes through the 640px art raster or a fractional glyph scale.
    int multiple=speech?std::max(1,drawScale*3/2):drawScale;
    auto& font=fontFor(11*multiple);font.setLineHeight((speech?20:18)*drawScale);
    ofPushMatrix();ofScale(1.f/drawScale,1.f/drawScale);ofSetColor(color);
    font.drawString(text,std::round(x*drawScale),std::round(baseline*drawScale));ofPopMatrix();
}
void ofApp::body(const std::string& text,int x,int y,ofColor color){rasterText(text,x,y+10,color);}
void ofApp::label(const std::string& text,int x,int y,ofColor color){body(text,x,y,color);}
void ofApp::textInBox(const std::string& text,const ofRectangle& area,ofColor color){
    auto box=fontFor(11*drawScale).getStringBoundingBox(text,0,0);
    float x=std::round(area.getCenter().x*drawScale-box.x-box.width/2)/drawScale;
    float baseline=std::round(area.getCenter().y*drawScale-box.y-box.height/2)/drawScale;
    rasterText(text,x,baseline,color);
}
void ofApp::drawProp(ofImage& art,const ofRectangle& area){
    float scale=std::min(area.width/art.getWidth(),area.height/art.getHeight());
    int width=std::round(art.getWidth()*scale),height=std::round(art.getHeight()*scale);
    ofSetColor(255);art.draw(std::round(area.getCenter().x-width/2.f),std::round(area.getCenter().y-height/2.f),width,height);
}
void ofApp::drawEnvelope(const ofRectangle& area){drawProp(envelopeArt,area);}
void ofApp::drawChest(const ofRectangle& area,bool opened){drawProp(chests[opened?1:0],area);}
void ofApp::button(const std::string& text,const ofRectangle& r,bool active,int scale){
    paperFrame(r.x,r.y,r.width,r.height,active?ofColor(238,191,102):(over(r)?light:ofColor(239,218,177)));
    if(scale>=3)centered(text,r.getCenter().x,r.y+(r.height-7*scale)/2,ink,scale);
    else textInBox(text,r,ink);
}
void ofApp::drawIntro(){
    float t=exhibitAt<0?0:ofClamp((ofGetElapsedTimef()-exhibitAt)/.55f,0,1);
    float ease=t*t*(3-2*t),zoom=1+.06f*ease;
    ofPushMatrix();ofTranslate(320,160);ofScale(zoom,zoom);ofSetColor(255);hero.draw(-320,-160,W,H);ofPopMatrix();
    rect(0,0,W,H,ofColor(32,22,27,45+int(ease*35)));
    float bellAge=ofGetElapsedTimef()-bellAt;
    if(bellAge>=0&&bellAge<1.5f){
        int shake=int(std::sin(bellAge*36)*2);paperFrame(233+shake,86,174,35,paper);
        textInBox("Ding-dong!",{float(233+shake),86,174,35},ink);
        ofNoFill();ofSetColor(gold,180*(1-bellAge/1.5f));ofDrawCircle(600,170,14+bellAge*20);ofFill();
    }
    if(exhibitAt<0)return;
    ofPushMatrix();ofTranslate(320,168);ofScale(.94f+.06f*ease);ofTranslate(-320,-168);
    drawChest({35,64,272,188});
    paperFrame(337,63,254,184,paper);ofSetColor(255);photoTexture.draw(346,72,236,157);
    rect(346,232,236,1,ofColor(202,174,126));ofPopMatrix();
}

void ofApp::drawHome(){
    float t=ofGetElapsedTimef();ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(31,24,37,36));
    // Ambient motion stays behind the controls and never moves their hit targets.
    rect(277,190,78,66,ofColor(105,221,198,8+int(7*(1+std::sin(t*1.3f)))));
    for(int i=0;i<23;++i){int x=40+(i*73)%560,y=150+int(std::fmod(t*(3+i%4)+i*23,184.f));rect(x+int(std::sin(t*.6f+i)*6),y,1+i%2,1,ofColor(255,222,142,50+int(50*(1+std::sin(t+i)))));}
    int bob=int(std::sin(t*.7f));ofPushMatrix();ofTranslate(0,bob);drawProp(titleArt,{81,23,478,163});
    centered("MIRROR ARCHIVE",320,101,light,4);ofPopMatrix();
    // Only the artwork floats; the hit area covers its full travel.
    auto floatingStart=startButton;floatingStart.y+=std::round(std::sin(t*2.f)*3);
    button("START",floatingStart,true,3);button("Meet us",castButton);button("About",aboutButton);
    if(over(soundButton))rect(soundButton.x,soundButton.y,soundButton.width,soundButton.height,ofColor(49,37,35,160));
    auto soundColor=over(soundButton)?light:ofColor(190,165,126);
    rect(599,359,4,7,soundColor);ofSetColor(soundColor);ofDrawTriangle(603,359,608,355,608,370);
    if(soundOn){line(612,358,614,360,soundColor);line(614,360,614,365,soundColor);line(614,365,612,367,soundColor);}
    else {line(612,359,619,366,soundColor);line(619,359,612,366,soundColor);}
}
void ofApp::drawInfo(){
    drawHome();rect(0,0,W,H,ofColor(24,20,27,190));paperFrame(48,29,544,327,paper);
    button("Back",infoBack);
    if(infoPage==0){
        rasterText("Meet the archive",72,68,ink,true);
        const std::array<std::string,3> names{"You","tonton","Lia"};
        const std::array<std::string,3> descriptions{
            "You opened this\narchive to bring\nold stories back.",
            "Your helper.\nI keep the tools\nand cheer you on!",
            "Today's visitor.\nShe brings a box\nand an old photo."};
        for(int i=0;i<3;++i){int x=66+i*173;paperFrame(x,92,162,230,ofColor(255,235,195));
            if(i==0)drawEnvelope({float(x+43),111,76,64});
            else if(i==1)pixel::tonton(x+81,156,2,ofGetElapsedTimef(),1);
            else {ofSetColor(255);lia.drawSubsection(x+45,103,72,72,0,0,88,88);}
            textInBox(names[i],{float(x+15),190,132,22},ink);body(descriptions[i],x+13,231,ink);
        }
    }else{
        rasterText("About this game",72,68,ink,true);
        body("CityUHK CS4187 Mini Project.",72,91);
        label("How to play",72,121,wood);
        body("Mouse: brushes and writing.\nCamera: guide the letter with your hand.\nSpace: next line. Esc: back to the cover.",72,143);
        label("Credits & notice",72,211,wood);
        body("A fictional story about family and care.\nBuilt with openFrameworks, OpenCV and MediaPipe.\nFont: Departure Mono (OFL).\nYour camera stays on this computer.\nCamera images are never saved.",72,233);
    }
}
void ofApp::drawDesk(){
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(30,24,29,107));
    // A single work surface, not a stack of heavy panels.
    rect(14,31,612,304,ofColor(108,70,51,240));
    for(int y=34;y<334;y+=48){rect(14,y,612,1,ofColor(64,43,35));rect(14,y+1,612,1,ofColor(157,104,65));
        for(int x=24+(y%5)*9;x<610;x+=91)rect(x,y+23,37,1,ofColor(135,87,54));}
    paperFrame(202,5,236,25,paper);body(titles[chapter],int(320-bodyWidth(titles[chapter])/2),12);
    button("Home",homeButton);
}

void ofApp::drawPhoto(){
    paperFrame(photoArea.x-5,photoArea.y-5,photoArea.width+10,photoArea.height+10,paper);
    ofSetColor(255);(comparing?damagedTexture:photoTexture).draw(photoArea);
    for(int x:{150,471}){rect(x,38,19,10,ofColor(183,159,111));rect(x+2,39,15,2,light);}
    paperFrame(40,308,584,69,ofColor(107,72,51));
    const std::array<std::string,3> filters{"Clean","Contrast","Detail"};
    for(int i=0;i<3;++i)button(filters[i],filterButton(i),filterIndex==i);
    textInBox("Size",{280,314,49,24},light);button("-",smallerBrush);button("+",largerBrush);
    ofNoFill();ofSetColor(light);ofDrawCircle(389,326,brushRadius/4.f);ofFill();
    button("Undo",undoButton);
    float strength=repair.strength(homeward::Filter(filterIndex));
    textInBox("Strength",{280,346,72,24},light);
    rect(strengthTrack.x,strengthTrack.y+10,strengthTrack.width,4,ofColor(180,148,100));
    rect(strengthTrack.x,strengthTrack.y+10,strengthTrack.width*strength,4,gold);
    paperFrame(int(strengthTrack.x+strengthTrack.width*strength)-3,int(strengthTrack.y)+5,7,14,gold);
    button("Original",beforeButton,comparing);
    button(revisiting?(returnChapter==2?"Back to the map":"Back to the box"):"I found the password!",nextButton,true);
    if(!comparing&&screen==Screen::Play&&photoArea.inside(logical(ofGetMouseX(),ofGetMouseY()))){auto p=logical(ofGetMouseX(),ofGetMouseY());ofNoFill();ofSetColor(light);ofDrawCircle(int(p.x),int(p.y),brushRadius*photoArea.width/480);ofFill();}
}

void ofApp::drawLock(){
    drawChest({83,37,474,288});
    rect(132,103,376,207,ofColor(65,41,30,120));
    paperFrame(248,73,144,35,ofColor(80,53,40));label("Our first home",264,83,light);
    for(int i=0;i<3;++i){auto r=inkArea(i);paperFrame(r.x-5,r.y-5,r.width+10,r.height+10,light);ofSetColor(255);writingTextures[i].draw(r);
        paperFrame(r.x+25,111,40,23,ofColor(77,64,50));pixel::text(recognized[i]<0?"-":std::to_string(recognized[i]),r.x+40,116,light,2);
        button("Clear",{r.x+20,258,50,25});
        if(guesses[i].candidates.size()>1){body("Or",r.x+26,293,light);
            for(size_t j=0;j<guesses[i].candidates.size();++j)button(std::to_string(guesses[i].candidates[j]),{r.x+49+float(j)*23,287,21,22},recognized[i]==guesses[i].candidates[j]);}
    }
    paperFrame(20,240,76,62,paper);ofSetColor(255);photoTexture.draw(24,244,68,40);body("Photo",37,286);
    button("Open",nextButton,true);
}

void ofApp::drawHouse(int x,int y,int kind){drawProp(houses[kind],{float(x-31),float(y-54),62,62});}
void ofApp::drawHouseDetail(){
    rect(0,31,W,H-31,ofColor(32,23,29,220));paperFrame(47,55,546,308,paper);
    textInBox("Your photo",{66,65,240,24},ink);textInBox("House",{347,65,200,24},ink);
    paperFrame(62,92,248,218,wood);ofSetColor(255);
    photoTexture.drawSubsection(66,96,240,210,120,70,240,210);
    drawProp(houses[inspectedHouse],{337,96,220,210});
    button("Work on photo",detailPhotoButton);button("Back",detailBackButton);
}
void ofApp::drawMap(bool caption){
    paperFrame(19,38,452,294,paper);ofSetColor(255);mapArt.draw(23,61,444,267);
    for(int y=0;y<9;++y)for(int x=0;x<13;++x){
        auto p=mapPoint({x,y});
        if(maze.road({x,y})){
            rect(p.x-9,p.y-9,19,19,ofColor(183,140,92));rect(p.x-7,p.y-7,15,15,ofColor(243,222,170));
            for(auto d:homeward::directions)if(maze.road(cv::Point(x,y)+homeward::step(d))){
                auto q=mapPoint(cv::Point(x,y)+homeward::step(d));
                rect(std::min(p.x,q.x)-7,std::min(p.y,q.y)-7,std::abs(q.x-p.x)+15,std::abs(q.y-p.y)+15,ofColor(243,222,170));
            }
            rect(p.x-3,p.y+4,4,1,ofColor(202,178,124));rect(p.x+3,p.y-3,2,1,ofColor(202,178,124));
        }else if(y>1&&y<8&&x>0&&x<12){
            if((x+y)%4==0){tree(p.x,p.y,17+x);}
            else if((x+y)%3==0){for(int n=0;n<3;++n){int xx=p.x-8+n*7;rect(xx,p.y-5,5,8,ofColor(94,123,65));rect(xx+1,p.y-7,3,4,light);rect(xx+2,p.y-6,1,2,gold);}}
            else if(y%2==0){for(int xx=-10;xx<12;xx+=10){rect(p.x+xx,p.y-4,3,10,wood);}rect(p.x-10,p.y,22,2,ofColor(173,132,76));}
        }
    }
    for(int y=1;y<8;++y)for(int x=1;x<12;++x)if(maze.road({x,y})){
        auto p=mapPoint({x,y});for(int i=0;i<5;++i){int dx=(x*7+y*3+i*5)%11-5,dy=(x*3+y*7+i*3)%11-5;rect(p.x+dx,p.y+dy,2,1,ofColor(218,195,145));}
    }
    if(caption)textInBox(story::dedication,{23,41,444,19},ink);
    for(size_t i=1;i<trail.size();++i){auto a=mapPoint(trail[i-1]),b=mapPoint(trail[i]);if(cv::norm(trail[i]-trail[i-1])==1)line(a.x,a.y,b.x,b.y,ofColor(170,98,72));}
    // A bridge and a tiny pond are scenery; road topology remains explicit.
    rect(211,271,23,17,ofColor(103,158,163));for(int y=271;y<288;y+=4)rect(211,y,23,1,ofColor(161,190,167));
    for(int y=273;y<291;y+=4)rect(213,y,20,3,wood);
    auto h=mapPoint(maze.home);drawHouse(h.x,h.y,0);
    for(int i=0;i<2;++i){auto p=mapPoint(maze.decoys[i]);drawHouse(p.x,p.y,i+1);}
    auto p=mapPoint(maze.position);
    if(moveDuration>0&&!movePath.empty()){
        float phase=ofClamp((ofGetElapsedTimef()-moveAt)/moveDuration,0,1)*movePath.size();
        int segment=std::min(int(movePath.size()-1),int(phase));float t=std::min(1.f,phase-segment);
        auto a=mapPoint(segment==0?moveFrom:movePath[segment-1]),b=mapPoint(movePath[segment]);p=a+(b-a)*t;
    }
    drawEnvelope({p.x-16,p.y-12,32,24});
}
void ofApp::drawMaze(){
    drawMap();
    if(delivered){button("Read the note",nextButton,true);return;}
    paperFrame(481,38,148,224,ofColor(66,57,51));
    textInBox("Camera",{489,44,132,23},light);
    paperFrame(487,75,136,103,ofColor(42,38,39));
    if(hand.running&&cameraTexture.isAllocated()){ofSetColor(255);cameraTexture.draw(cameraPreview);rect(489,77,132,99,ofColor(25,29,33,45));}
    else{
        // A compact hand silhouette shows where to begin without a second block of text.
        rect(548,119,16,22,ofColor(132,116,96));
        for(int i=0;i<4;++i)rect(548+i*4,107+(i%3)*2,3,17,ofColor(157,138,112));
        rect(543,126,6,10,ofColor(157,138,112));
    }
    auto allowed=exits();
    for(int i=0;i<4;++i){auto r=directionButton(i);auto v=homeward::step(homeward::directions[i]);auto p=r.getCenter();
        ofSetColor(allowed[i]?light:ofColor(110,104,91));
        ofDrawLine(p.x-v.x*5,p.y-v.y*5,p.x+v.x*5,p.y+v.y*5);
        ofDrawLine(p.x+v.x*5,p.y+v.y*5,p.x+v.y*3,p.y-v.x*3);ofDrawLine(p.x+v.x*5,p.y+v.y*5,p.x-v.y*3,p.y+v.x*3);
    }
    ofNoFill();ofSetColor(palm.ready()?gold:paper);ofDrawRectangle(539,114,32,24);ofFill();
    if(hand.running&&hand.visible){int x=cameraPreview.x+hand.x*cameraPreview.width,y=cameraPreview.y+hand.y*cameraPreview.height;rect(x-2,y-2,5,5,gold);
        int count=int(palm.progress()*16);for(int i=0;i<count;++i){float a=i*TWO_PI/16;rect(int(x+9*std::cos(a)),int(y+9*std::sin(a)),2,2,gold);}}
    std::string status=!hand.running?"Ready when you are":(!hand.ready?"Starting...":(!hand.visible?"Show your hand":(moveDuration>0?"On the way...":(palm.ready()?"Choose a way":"Back to middle"))));
    textInBox(status,{486,187,138,24},light);
    button(hand.running?"Camera off":(cameraPending?"One moment...":"Start camera"),cameraButton,hand.running);
    paperFrame(referenceButton.x,referenceButton.y,referenceButton.width,referenceButton.height,paper);
    ofSetColor(255);photoTexture.draw(493,282,61,41);textInBox("Photo",{558,282,59,41},ink);
    button("Start over",{21,343,91,30});
}

void ofApp::drawPlay(){
    drawDesk();if(chapter==0)drawPhoto();else if(chapter==1)drawLock();else drawMaze();
    if(ofGetElapsedTimef()<noticeUntil&&!notice.empty()){paperFrame(176,315,288,21,paper);body(notice,185,321);}
    if(inspectedHouse>=0)drawHouseDetail();
}

void ofApp::drawSealedEnvelope(){
    drawDesk();drawChest({112,34,416,240},true);drawEnvelope(envelopeArea);
    if(screen==Screen::Envelope)button("Open envelope",openEnvelopeButton,true);
}

ofRectangle ofApp::dialogueBox()const{
    auto focus=dialogue[speechIndex].focus;
    return {18,focus!=story::Focus::None&&focus!=story::Focus::Ready?48.f:278.f,604,95};
}
ofRectangle ofApp::focusArea()const{
    switch(dialogue[speechIndex].focus){
        case story::Focus::Clean:return filterButton(0);
        case story::Focus::Contrast:return filterButton(1);
        case story::Focus::Detail:return filterButton(2);
        case story::Focus::Size:return {278,311,166,28};
        case story::Focus::Strength:return {278,343,166,31};
        case story::Focus::Original:return beforeButton;
        default:return {};
    }
}
void ofApp::drawDialogue(){
    const auto& speech=dialogue[speechIndex];
    if(speech.focus!=story::Focus::None)drawPlay();
    else if(intro)drawIntro();
    else if(outro)drawOutro();
    else if(chapter==2&&!delivered&&!envelopeOpened)drawSealedEnvelope();
    else drawPlay();
    auto target=focusArea();auto shade=ofColor(28,21,27,90);
    if(target.width>0){
        target.x-=4;target.y-=4;target.width+=8;target.height+=8;
        rect(0,0,W,target.y,shade);rect(0,target.y,target.x,target.height,shade);
        rect(target.getRight(),target.y,W-target.getRight(),target.height,shade);
        rect(0,target.getBottom(),W,H-target.getBottom(),shade);
        // Crisp pixel brackets leave the highlighted control unobscured.
        auto glow=ofColor(255,221,137,210+int(35*std::sin(ofGetElapsedTimef()*4)));
        for(int side:{0,1})for(int edge:{0,1}){
            int x=side?target.getRight()-7:target.x,y=edge?target.getBottom()-2:target.y;
            rect(x,y,7,2,glow);rect(side?target.getRight()-2:target.x,edge?target.getBottom()-7:target.y,2,7,glow);
        }
    }else rect(0,0,W,H,ofColor(28,21,27,50));
    auto box=dialogueBox();int y=box.y;
    paperFrame(box.x,y,box.width,box.height,paper);
    if(!speech.narration){paperFrame(38,y-13,104,24,ofColor(114,72,49));textInBox(speech.speaker,{38,float(y-13),104,24},light);}
    float age=ofGetElapsedTimef()-dialogueAt;
    if(speech.focus==story::Focus::None||age>=.3f)rasterText(speech.words,38,y+35,ink,true);
    if(!speech.narration&&speech.speaker!="You")rect(507,y+12,1,70,ofColor(208,179,130));
    if(speech.speaker=="tonton"){
        int shake=age<.35&&speech.happy?int(std::sin(age*65)*2):0;
        pixel::tonton(562+shake,y+52,2,ofGetElapsedTimef(),speech.happy);
    }else if(speech.speaker=="Lia"){
        int pose=ofClamp(speech.happy,0,2);float cell=lia.getWidth()/3;
        ofSetColor(255);lia.getTexture().drawSubsection(520,y+4,88,88,cell*pose,0,cell,lia.getHeight());
    }
    int dot=int(ofGetElapsedTimef()*2)%3;for(int i=0;i<3;++i)rect(483+i*5,y+81,2,2,i==dot?wood:ofColor(210,175,120));
}

void ofApp::drawLetter(){
    drawDesk();float age=ofGetElapsedTimef()-letterAt;
    float flip=letterOpened&&age<.6f?std::max(.02f,float(std::abs(std::cos(age/.6f*PI)))):1;
    ofPushMatrix();ofTranslate(320,0);ofScale(flip,1);ofTranslate(-320,0);
    drawProp(paperArt,letterArea);
    if(!letterOpened||age<.3f){
        textInBox(story::dedication,{176,58,288,25},ink);
        ofPushMatrix();ofTranslate(136,56);ofScale(.75f,.75f);drawMap(false);ofPopMatrix();
        int lift=2+int((1+std::sin(ofGetElapsedTimef()*3))*2);
        float fit=std::min(letterArea.width/paperArt.getWidth(),letterArea.height/paperArt.getHeight());
        int right=letterArea.getCenter().x+paperArt.getWidth()*fit/2-4,bottom=letterArea.getBottom()-5;
        ofSetColor(190,149,93);ofDrawTriangle(right-26,bottom,right,bottom-26,right,bottom);
        ofSetColor(over(letterArea)?light:ofColor(255,241,204));ofDrawTriangle(right-26,bottom-lift,right-26,bottom-26-lift,right,bottom-26-lift);
    }else rasterText(story::letter,126,91,ink,true);
    ofPopMatrix();
    if(letterOpened&&age>=.6f)button("Bring it to Lia",nextButton,true);
}
void ofApp::drawOutro(){
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(30,22,30,35));
    if(liaLeftAt>=0)return;
    if(returnPhotoAt<0){drawProp(paperArt,{190,79,263,170});drawEnvelope({390,168,89,59});return;}
    float t=ofClamp((ofGetElapsedTimef()-returnPhotoAt)/.9f,0,1),ease=t*t*(3-2*t);
    ofRectangle photo(110+ease*120,60+ease*15,270,180);
    paperFrame(photo.x-6,photo.y-6,photo.width+12,photo.height+12,paper);
    ofSetColor(255);giftPhoto.draw(photo);drawEnvelope({photo.x+210,photo.y+125,90,60});
}
void ofApp::drawComplete(){
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(30,22,30,60));
    rect(0,226,W,65,ofColor(35,25,30,195));
    rasterText("I leave a light on by the door.",171,251,light,true);
    rasterText("Then tonton and I head home.",186,275,light,true);
    button(copySaved?"Save again":"Keep a copy",copyButton);button("Go home",goHomeButton,true);
    if(ofGetElapsedTimef()<noticeUntil&&!notice.empty())textInBox(notice,{130,354,380,24},light);
}
void ofApp::renderCanvas(int scale){
    drawScale=scale;
    if(!canvas.isAllocated()||canvas.getWidth()!=W*scale||canvas.getHeight()!=H*scale){
        ofFbo::Settings settings;settings.width=W*scale;settings.height=H*scale;settings.internalformat=GL_RGBA;
        settings.textureTarget=GL_TEXTURE_2D;settings.numSamples=0;canvas.allocate(settings);
        canvas.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
    }
    canvasHasCamera=hand.running&&cameraTexture.isAllocated();
    canvas.begin();ofClear(shadow);ofSetColor(255);ofPushMatrix();ofScale(scale,scale);
    switch(screen){case Screen::Home:drawHome();break;case Screen::Info:drawInfo();break;case Screen::Play:drawPlay();break;case Screen::Dialogue:drawDialogue();break;
        case Screen::Envelope:drawSealedEnvelope();break;case Screen::Letter:drawLetter();break;case Screen::Complete:drawComplete();break;case Screen::Error:centered(errorMessage,320,180,light,2);break;}
    ofPopMatrix();canvas.end();
}
void ofApp::drawMagnifier(glm::vec2 p){
    int x=std::floor(p.x),y=std::floor(p.y);
    // Pixel steps, with the lens centre as the click point.
    rect(x-4,y-6,8,2,shadow);rect(x-6,y-4,2,8,shadow);rect(x+4,y-4,2,8,shadow);rect(x-4,y+4,8,2,shadow);
    rect(x-3,y-5,6,1,gold);rect(x-5,y-3,1,6,gold);rect(x+3,y-3,1,6,gold);rect(x-3,y+3,6,1,gold);
    rect(x+4,y+4,3,3,shadow);rect(x+6,y+6,3,3,shadow);rect(x+8,y+8,3,3,shadow);
    rect(x+4,y+4,2,2,wood);rect(x+6,y+6,2,2,gold);rect(x+8,y+8,2,2,gold);
    rect(x-2,y-2,2,1,light);rect(x-3,y-1,1,2,light);
}
void ofApp::draw(){
    auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());renderCanvas(v.scale);
    ofBackground(shadow);ofSetColor(255);canvas.draw(v.x,v.y);
    auto p=logical(ofGetMouseX(),ofGetMouseY());bool magnifier=(screen==Screen::Home&&ofRectangle(0,0,W,H).inside(p))||(screen==Screen::Dialogue&&dialogueBox().inside(p));
    if(magnifier){ofHideCursor();ofPushMatrix();ofTranslate(v.x,v.y);ofScale(v.scale,v.scale);drawMagnifier(p);ofPopMatrix();}else ofShowCursor();
}
void ofApp::pointerDown(glm::vec2 p){
    if(screen==Screen::Home){
        if(ofRectangle(startButton.x,startButton.y-3,startButton.width,startButton.height+6).inside(p))beginStory();
        else if(castButton.inside(p)){infoPage=0;screen=Screen::Info;}
        else if(aboutButton.inside(p)){infoPage=1;screen=Screen::Info;}
        else if(soundButton.inside(p)){soundOn=!soundOn;if(!soundOn)doorbell.stop();}
        return;
    }
    if(screen==Screen::Info){if(infoBack.inside(p))screen=Screen::Home;return;}
    if(screen==Screen::Dialogue){if(dialogueBox().inside(p))nextSpeech();return;}
    if(screen==Screen::Envelope){
        if(homeButton.inside(p)){screen=Screen::Home;return;}
        if(envelopeArea.inside(p)||openEnvelopeButton.inside(p))openEnvelope();return;
    }
    if(screen==Screen::Complete){
        if(goHomeButton.inside(p))screen=Screen::Home;
        else if(copyButton.inside(p)){
            ofShowCursor();auto file=ofSystemSaveDialog("Mirror Archive - My photo.png","Keep your restored photo");
            if(file.bSuccess){auto path=file.getPath();if(ofToLower(ofFilePath::getFileExt(path))!="png")path+=".png";saveCopy(path);}
        }return;
    }
    if(screen==Screen::Letter){
        if(!letterOpened&&letterArea.inside(p))openLetter();
        else if(letterOpened&&ofGetElapsedTimef()-letterAt>=.6f&&nextButton.inside(p))beginEnding();
        return;
    }
    if(screen!=Screen::Play)return;
    if(homeButton.inside(p)){stopCamera();moveDuration=0;screen=Screen::Home;brushing=false;drawingCell=-1;return;}
    if(inspectedHouse>=0){
        if(detailPhotoButton.inside(p))revisitPhoto(2);
        else if(detailBackButton.inside(p)){inspectedHouse=-1;palm.reset();lastPalmStamp=0;}
        return;
    }
    if(chapter==0){
        if(beforeButton.inside(p)){comparing=true;return;}
        if(ofRectangle(strengthTrack.x-5,strengthTrack.y,strengthTrack.width+10,strengthTrack.height).inside(p)){repair.beginStroke();adjustingStrength=true;pointerMove(p);return;}
        if(photoArea.inside(p)){repair.beginStroke();brushing=true;lastBrush={int((p.x-photoArea.x)*480/photoArea.width),int((p.y-photoArea.y)*320/photoArea.height)};pointerMove(p);}
        else if(smallerBrush.inside(p))brushRadius=std::max(7,brushRadius-5);
        else if(largerBrush.inside(p))brushRadius=std::min(39,brushRadius+5);
        else if(undoButton.inside(p)){repair.undo();refreshPhoto();}
        else if(nextButton.inside(p))finishChapter();
        else for(int i=0;i<3;++i)if(filterButton(i).inside(p))filterIndex=i;
    }else if(chapter==1){
        if(ofRectangle(20,240,76,62).inside(p)){revisitPhoto(1);return;}
        for(int i=0;i<3;++i){auto r=inkArea(i);
            if(ofRectangle(r.x+20,258,50,25).inside(p)){writing[i].setTo(0);recognized[i]=-1;guesses[i]={};inkDue[i]=0;refreshInk(i);return;}
            for(size_t j=0;j<guesses[i].candidates.size()&&guesses[i].candidates.size()>1;++j)if(ofRectangle(r.x+49+j*23,287,21,22).inside(p)){recognized[i]=guesses[i].candidates[j];return;}
            if(r.inside(p)){notice.clear();drawingCell=i;recognized[i]=-1;inkDue[i]=0;guesses[i]={};lastInk={int(p.x-r.x),int(p.y-r.y)};pointerMove(p);return;}
        }
        if(nextButton.inside(p)){
            for(int i=0;i<3;++i)if(inkDue[i]>0){guesses[i]=reader.guess(writing[i]);recognized[i]=guesses[i].digit;inkDue[i]=0;}
            if(recognized==std::array<int,3>{2,0,8})finishChapter();
            else{notice="The box is still locked.";noticeUntil=ofGetElapsedTimef()+3;
                if(++lockAttempts==2)say(story::tryPasswordAgain());}
        }
    }else{
        if(delivered){if(nextButton.inside(p)){screen=Screen::Letter;letterOpened=false;letterAt=ofGetElapsedTimef();}return;}
        if(cameraButton.inside(p))toggleCamera();
        else if(referenceButton.inside(p)&&moveDuration<=0){revisitPhoto(2);}
        else if(ofRectangle(21,343,91,30).inside(p)){maze.reset();trail.clear();moveFrom=maze.position;moveDuration=0;palm.reset();lastPalmStamp=0;}
        else{
            for(int i=0;i<4;++i)if(directionButton(i).inside(p)){moveEnvelope(homeward::directions[i]);return;}
            if(moveDuration<=0)for(int i=0;i<3;++i){auto q=mapPoint(i==0?maze.home:maze.decoys[i-1]);
                if(ofRectangle(q.x-31,q.y-54,62,62).inside(p)){inspectedHouse=i;palm.reset();lastPalmStamp=0;return;}}
        }
    }
}
void ofApp::pointerMove(glm::vec2 p){
    if(screen!=Screen::Play||photoOpen||inspectedHouse>=0||comparing)return;
    if(adjustingStrength){repair.setStrength(homeward::Filter(filterIndex),(p.x-strengthTrack.x)/strengthTrack.width);refreshPhoto();return;}
    if(brushing&&photoArea.inside(p)){
        cv::Point q{int((p.x-photoArea.x)*480/photoArea.width),int((p.y-photoArea.y)*320/photoArea.height)};
        repair.stroke(lastBrush,q,brushRadius,homeward::Filter(filterIndex));lastBrush=q;refreshPhoto();
    }
    if(drawingCell>=0){auto r=inkArea(drawingCell);cv::Point q{int(ofClamp(p.x-r.x,0,r.width-1)),int(ofClamp(p.y-r.y,0,r.height-1))};
        cv::line(writing[drawingCell],lastInk,q,cv::Scalar(255),5,cv::LINE_AA);cv::circle(writing[drawingCell],q,2,cv::Scalar(255),cv::FILLED);
        lastInk=q;refreshInk(drawingCell);
    }
}
void ofApp::pointerUp(glm::vec2 p){
    if(screen==Screen::Play){pointerMove(p);if(drawingCell>=0)inkDue[drawingCell]=ofGetElapsedTimef()+.45f;}
    drawingCell=-1;brushing=adjustingStrength=comparing=false;
}
void ofApp::mousePressed(int x,int y,int b){if(b==0)pointerDown(logical(x,y));}
void ofApp::mouseDragged(int x,int y,int b){if(b==0)pointerMove(logical(x,y));}
void ofApp::mouseReleased(int x,int y,int b){if(b==0)pointerUp(logical(x,y));}
void ofApp::keyPressed(int key){
    if(key==OF_KEY_ESC){if(inspectedHouse>=0){inspectedHouse=-1;palm.reset();lastPalmStamp=0;return;}if(photoOpen){photoOpen=false;palm.reset();return;}stopCamera();moveDuration=0;brushing=false;drawingCell=-1;screen=Screen::Home;}
    else if(key==OF_KEY_RETURN||key==' '){if(screen==Screen::Play&&chapter==0&&key==' '&&!brushing&&!adjustingStrength)comparing=true;else if(screen==Screen::Home)beginStory();else if(screen==Screen::Dialogue)nextSpeech();else if(screen==Screen::Envelope)openEnvelope();else if(screen==Screen::Letter){if(!letterOpened)openLetter();else if(ofGetElapsedTimef()-letterAt>=.6f)beginEnding();}}
    else if(screen==Screen::Play&&chapter==2){
        if(key==OF_KEY_LEFT)moveEnvelope(homeward::Direction::Left);if(key==OF_KEY_RIGHT)moveEnvelope(homeward::Direction::Right);
        if(key==OF_KEY_UP)moveEnvelope(homeward::Direction::Up);if(key==OF_KEY_DOWN)moveEnvelope(homeward::Direction::Down);
    }
}
void ofApp::keyReleased(int key){if(key==' ')comparing=false;}
void ofApp::windowResized(int w,int h){if(w<W||h<H)ofSetWindowShape(std::max(w,W),std::max(h,H));}

void ofApp::runQA(){
    auto checkQA=[&](bool passed,int line){if(!passed)ofLogError()<<"QA check failed at ofApp.cpp:"<<line<<" stage "<<qaStage;qaPassed&=passed;};
    if(++qaFrame>(qaCamera?7500:6000)){ofLogError()<<"QA timed out at "<<qaStage;ofExit(1);return;}if(qaFrame%18)return;
    auto physical=[&](glm::vec2 p){auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());return glm::ivec2(v.x+int(p.x*v.scale),v.y+int(p.y*v.scale));};
    auto click=[&](glm::vec2 p){auto q=physical(p);mousePressed(q.x,q.y,0);mouseReleased(q.x,q.y,0);};
    auto capture=[&](const std::string& name,int scale=2){if(hand.running||canvasHasCamera)return;renderCanvas(scale);auto folder=ofFilePath::getCurrentExeDir()+"../../../../qa/";ofDirectory::createDirectory(folder,true,true);ofPixels pixels;canvas.readToPixels(pixels);ofSaveImage(pixels,folder+name);};
    if(screen==Screen::Dialogue){
        auto cue=dialogue[speechIndex].cue;float age=ofGetElapsedTimef()-dialogueAt;
        if(dialogue[speechIndex].focus!=story::Focus::None&&age<.5f)return;
        if((cue==story::Cue::ShowKeepsakes&&age<.65f)||(cue==story::Cue::Doorbell&&age<1.f)||(cue==story::Cue::ReturnPhoto&&age<1.f))return;
        capture("qa-dialogue-"+std::to_string(chapter)+"-"+std::to_string(speechIndex)+"-"+std::to_string(dialogue[speechIndex].happy)+".png");
        if(intro&&speechIndex==0){capture("qa-narration.png");checkQA(bool(dialogue[speechIndex].narration),__LINE__);}
        if(cue==story::Cue::ShowKeepsakes){capture("qa-intro.png");capture("qa-type-3.png",3);capture("qa-type-4.png",4);}
        if(cue==story::Cue::Doorbell){capture("qa-doorbell.png");checkQA(bool(bellsPlayed==1&&doorbell.isLoaded()),__LINE__);}
        if(cue==story::Cue::ReturnPhoto){capture("qa-handover.png");checkQA(bool(giftReturned&&giftPhoto.isAllocated()&&!returnedPhoto.empty()&&cv::norm(returnedPhoto,repair.image(),cv::NORM_INF)==0&&cv::countNonZero(repair.mask)<int(repair.mask.total())),__LINE__);}
        if(cue==story::Cue::LiaLeaves)capture("qa-goodbye.png");
        if(outro&&speechIndex>0&&dialogue[speechIndex-1].cue==story::Cue::ReturnPhoto)capture("qa-lia-response.png");
        if(!intro&&!outro&&chapter==1&&dialogue[speechIndex].words.find("password from the photo")!=std::string::npos)capture("qa-photo-to-box.png");
        if(intro&&dialogue[speechIndex].words.find("inside this box")!=std::string::npos)capture("qa-lia-request.png");
        if(!intro&&!outro&&chapter==2&&!delivered&&!envelopeOpened){checkQA(afterDialogue==Screen::Envelope,__LINE__);capture("qa-open-box.png");}
        if(!intro&&!outro&&chapter==2&&dialogue[speechIndex].words.find("Click a map house")!=std::string::npos&&!delivered)capture("qa-photo-guide.png");
        if(!intro&&!outro&&chapter==2&&dialogue[speechIndex].words.find("Show your hand")!=std::string::npos&&!delivered)capture("qa-camera-guide.png");
        if(delivered&&!outro&&speechIndex==0)capture("qa-map-success.png");
        if(delivered&&!outro&&speechIndex==2)capture("qa-turn-guide.png");
        if(dialogue[speechIndex].focus!=story::Focus::None){
            auto target=focusArea();auto box=dialogueBox();
            checkQA(target.width==0||box.getBottom()<target.y,__LINE__);
            capture("qa-tool-guide-"+std::to_string(int(dialogue[speechIndex].focus))+".png");
        }
        if(chapter==1&&speechIndex==0)capture("qa-photo-success.png");
        click(dialogueBox().getCenter());return;
    }

    if(screen==Screen::Envelope){
        checkQA(!envelopeOpened&&!letterOpened&&!delivered&&!hand.running,__LINE__);capture("qa-sealed-envelope.png");
        auto position=maze.position;keyPressed(OF_KEY_RIGHT);checkQA(maze.position==position,__LINE__);
        click(openEnvelopeButton.getCenter());checkQA(envelopeOpened&&screen==Screen::Dialogue&&!letterOpened,__LINE__);
        capture("qa-map-found.png");return;
    }
    switch(qaStage){
        case 0:
            capture("qa-home.png");click(castButton.getCenter());checkQA(bool(screen==Screen::Info),__LINE__);capture("qa-cast.png");click(infoBack.getCenter());
            click(aboutButton.getCenter());checkQA(bool(screen==Screen::Info),__LINE__);capture("qa-about.png");click(infoBack.getCenter());
            click(soundButton.getCenter());checkQA(bool(!soundOn),__LINE__);click(soundButton.getCenter());checkQA(bool(soundOn),__LINE__);
            click(startButton.getCenter());++qaStage;break;
        case 1:{capture("qa-damaged.png");
            for(int tool=0;tool<3;++tool){click(filterButton(tool).getCenter());for(int y=100;y<298;y+=22){auto a=physical({176,float(y)});mousePressed(a.x,a.y,0);
                for(int x=176;x<445;x+=24){auto b=physical({float(x),float(y)});mouseDragged(b.x,b.y,0);}
                mouseReleased(a.x,a.y,0);}}
            checkQA(screen==Screen::Play,__LINE__);++qaStage;break;
        }
        case 2:{
            auto painted=repair.image();auto mask=repair.mask.clone();float original=repair.strength(homeward::Filter(filterIndex));
            click({strengthTrack.x,strengthTrack.getCenter().y});checkQA(bool(repair.strength(homeward::Filter(filterIndex))==0&&cv::norm(painted,repair.image(),cv::NORM_INF)>0),__LINE__);
            click(undoButton.getCenter());checkQA(bool(repair.strength(homeward::Filter(filterIndex))==original&&cv::norm(painted,repair.image(),cv::NORM_INF)==0),__LINE__);
            auto q=physical(beforeButton.getCenter());mousePressed(q.x,q.y,0);checkQA(bool(comparing),__LINE__);capture("qa-before.png");
            q=physical(photoArea.getCenter());mouseReleased(q.x,q.y,0);checkQA(bool(!comparing&&cv::norm(mask,repair.mask,cv::NORM_INF)==0),__LINE__);
            keyPressed(' ');checkQA(comparing,__LINE__);keyReleased(' ');checkQA(!comparing&&cv::norm(painted,repair.image(),cv::NORM_INF)==0,__LINE__);
            int oldRadius=brushRadius;click(largerBrush.getCenter());checkQA(brushRadius>oldRadius,__LINE__);click(smallerBrush.getCenter());checkQA(brushRadius==oldRadius,__LINE__);
            capture("qa-case1.png");click(nextButton.getCenter());checkQA(bool(chapter==1),__LINE__);++qaStage;break;
        }
        case 3:{capture("qa-case2-blank.png");
            click(nextButton.getCenter());checkQA(bool(chapter==1),__LINE__); // Empty lock cannot open.
            click({55,265});checkQA(bool(chapter==0&&revisiting),__LINE__);click(nextButton.getCenter());checkQA(bool(chapter==1&&!revisiting),__LINE__);
            // Draw separate mouse strokes, not injected recognized values.
            std::array<std::vector<glm::vec2>,3> strokes;
            strokes[0]={{14,26},{20,17},{35,12},{50,15},{59,24},{60,34},{55,44},{44,55},{30,67},{17,82},{63,82}};
            for(int j=0;j<=40;++j){float t=j*TWO_PI/40;strokes[1].push_back({45+24*std::cos(t),54+37*std::sin(t)});}
            for(int j=0;j<=64;++j){float t=j*TWO_PI/64;strokes[2].push_back({45+23*std::sin(2*t),54+36*std::cos(t)});}
            for(int i=0;i<3;++i){auto r=inkArea(i);auto q=physical({r.x+strokes[i][0].x,r.y+strokes[i][0].y});mousePressed(q.x,q.y,0);
                for(auto p:strokes[i]){q=physical({r.x+p.x,r.y+p.y});mouseDragged(q.x,q.y,0);}mouseReleased(q.x,q.y,0);}
            ++qaStage;break;
        }
        case 4:if(inkDue[0]>0||inkDue[1]>0||inkDue[2]>0)break;ofLogNotice()<<"QA handwriting: "<<recognized[0]<<recognized[1]<<recognized[2];checkQA(bool(recognized==std::array<int,3>{2,0,8}),__LINE__);capture("qa-case2.png");click(nextButton.getCenter());if(chapter!=2){ofLogError()<<"QA handwriting lock failed";ofExit(1);return;}++qaStage;break;
        case 5:
            if(qaMove==0){
                auto expected=maze.destination(maze.position,homeward::Direction::Right);keyPressed(OF_KEY_RIGHT);checkQA(bool(moveDuration>0&&maze.position==expected),__LINE__);
                keyPressed(OF_KEY_RIGHT);checkQA(bool(maze.position==expected),__LINE__);
                keyPressed(OF_KEY_ESC);checkQA(bool(screen==Screen::Home&&moveDuration==0),__LINE__);
                click(startButton.getCenter());checkQA(bool(chapter==0&&cv::countNonZero(repair.mask)==0&&recognized==std::array<int,3>{-1,-1,-1}),__LINE__);
                qaMove=1;qaStage=1;break;
            }
            if(!qaCluesChecked&&moveDuration<=0){
                auto position=maze.position;auto oldTrail=trail;auto picture=repair.image();
                for(int i=0;i<3;++i){auto q=mapPoint(i==0?maze.home:maze.decoys[i-1]);click({q.x,q.y-25});checkQA(bool(inspectedHouse==i&&maze.position==position),__LINE__);capture("qa-house-"+std::to_string(i)+".png");click(detailBackButton.getCenter());}
                click(referenceButton.getCenter());checkQA(bool(chapter==0&&revisiting&&returnChapter==2),__LINE__);
                auto a=physical({160,280}),b=physical({180,280});mousePressed(a.x,a.y,0);mouseDragged(b.x,b.y,0);mouseReleased(b.x,b.y,0);
                capture("qa-map-photo.png");click(nextButton.getCenter());checkQA(bool(chapter==2&&!revisiting&&maze.position==position&&trail==oldTrail&&cv::norm(picture,repair.image(),cv::NORM_INF)>0),__LINE__);
                auto q=mapPoint(maze.decoys[0]);click({q.x,q.y-25});click(detailPhotoButton.getCenter());checkQA(bool(chapter==0&&returnChapter==2),__LINE__);click(nextButton.getCenter());checkQA(bool(chapter==2&&maze.position==position),__LINE__);
                qaCluesChecked=true;
            }
            if(qaCamera&&!qaCameraChecked){
                if(qaCameraAt<0){qaCameraAt=ofGetElapsedTimef();click(cameraButton.getCenter());break;}
                if(cameraFrames>0&&qaFirstFrameAt<0)qaFirstFrameAt=ofGetElapsedTimef();
                if(qaFirstFrameAt<0&&ofGetElapsedTimef()-qaCameraAt<35)break;
                if(qaFirstFrameAt>=0&&ofGetElapsedTimef()-qaFirstFrameAt<5)break;
                bool connected=hand.running&&cameraTexture.isAllocated()&&cameraFrames>10;
                ofLogNotice()<<"QA palm camera: "<<(connected?"PASS":"FAIL")<<" ("<<cameraFrames<<" frames; none saved; startup "<<(qaFirstFrameAt<0?-1:qaFirstFrameAt-qaCameraAt)<<"s)";
                checkQA(bool(connected),__LINE__);
                if(connected&&!qaCameraResumed){
                    auto position=maze.position;click(referenceButton.getCenter());checkQA(bool(chapter==0&&!hand.running&&resumeCamera),__LINE__);
                    click(nextButton.getCenter());checkQA(bool(chapter==2&&cameraPending&&maze.position==position),__LINE__);
                    qaCameraResumed=true;qaCameraAt=ofGetElapsedTimef();qaFirstFrameAt=-1;cameraFrames=0;break;
                }
                stopCamera();checkQA(bool(!hand.running),__LINE__);
                maze.reset();moveFrom=maze.position;moveDuration=0;qaCameraChecked=true;break;
            }
            if(moveDuration>0)break;
            if(delivered){checkQA(bool(screen==Screen::Letter),__LINE__);capture("qa-arrived.png");++qaStage;break;}
            capture("qa-case3.png");{auto route=maze.solution();if(route.empty()){ofExit(1);return;}
                int key=route[0]==homeward::Direction::Left?OF_KEY_LEFT:route[0]==homeward::Direction::Right?OF_KEY_RIGHT:route[0]==homeward::Direction::Up?OF_KEY_UP:OF_KEY_DOWN;keyPressed(key);}break;
        case 6:click({528,315});checkQA(bool(letterOpened),__LINE__);++qaStage;break;
        case 7:if(ofGetElapsedTimef()-letterAt<.7)break;capture("qa-letter.png");click(nextButton.getCenter());++qaStage;break;
        case 8:{
            auto path=ofFilePath::getCurrentExeDir()+"../../../../qa/player-photo.png";
            checkQA(saveCopy(path),__LINE__);auto exported=cv::imread(path);checkQA(!exported.empty()&&cv::norm(exported,returnedPhoto,cv::NORM_INF)==0,__LINE__);
            checkQA(envelopeOpened&&(!qaCamera||cameraTaught),__LINE__);
            checkQA(bool(screen==Screen::Complete&&giftReturned&&liaLeftAt>=0),__LINE__);capture("qa-complete.png");ofLogNotice()<<"QA homeward story: "<<(qaPassed?"PASS":"FAIL");ofExit(qaPassed?0:1);break;}
    }
}
