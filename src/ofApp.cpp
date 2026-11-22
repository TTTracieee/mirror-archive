#include "ofApp.h"

namespace {
using namespace pixel;
constexpr int W=640,H=384;
const ofRectangle startButton{236,258,168,44},homeButton{580,8,48,26};
const ofRectangle photoArea{104,40,432,288},nextButton{459,343,165,30};
const ofRectangle cameraButton{486,243,140,28},referenceButton{486,288,140,28};
const ofRectangle castButton{176,318,130,30},aboutButton{334,318,130,30},infoBack{520,43,68,28},soundButton{536,354,88,23};
const ofRectangle letterArea{70,39,500,290};
const cv::Rect address{213,71,50,17};
const std::array<std::string,3> titles{"Restore the photo","Open the box","Guide the letter"};
ofRectangle inkArea(int i){return {float(145+i*126),137,90,108};}
void rect(float x,float y,float w,float h,ofColor c){ofSetColor(c);ofDrawRectangle(x,y,w,h);}
void line(int x,int y,int xx,int yy,ofColor c){ofSetColor(c);ofDrawLine(x,y,xx,yy);}
glm::vec2 mapPoint(cv::Point p){return {float(35+p.x*32),float(68+p.y*29)};}
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
        if(!lia.load("art/lia-portraits.png")||!mapArt.load("art/garden-map.png")||!buildings.load("art/village-buildings.png"))throw std::runtime_error("Missing story art");
        buildings.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        lia.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);mapArt.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        // Keep scene art on its logical pixel grid; text has a separate raster size.
        auto sample=[](ofImage& art,int width,int height){art.getPixels().resize(width,height,OF_INTERPOLATE_NEAREST_NEIGHBOR);art.update();art.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);};
        sample(hero,640,384);sample(lia,264,88);sample(mapArt,452,294);sample(buildings,186,62);
        auto trim=[](ofImage& art,int width){
            auto& pixels=art.getPixels();int left=pixels.getWidth(),top=pixels.getHeight(),right=-1,bottom=-1;
            for(int y=0;y<int(pixels.getHeight());++y)for(int x=0;x<int(pixels.getWidth());++x)if(pixels.getColor(x,y).a>16){left=std::min(left,x);top=std::min(top,y);right=std::max(right,x);bottom=std::max(bottom,y);}
            if(right<left)throw std::runtime_error("Empty prop art");art.crop(left,top,right-left+1,bottom-top+1);
            int height=int(std::round(width*art.getHeight()/art.getWidth()));art.getPixels().resize(width,height,OF_INTERPOLATE_NEAREST_NEIGHBOR);art.update();
            art.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
        };
        ofPixels chestSheet;if(!ofLoadImage(chestSheet,"art/keepsake-chests.png"))throw std::runtime_error("Missing chest art");
        for(int i=0;i<2;++i){ofPixels cell;chestSheet.cropTo(cell,i*chestSheet.getWidth()/2,0,chestSheet.getWidth()/2,chestSheet.getHeight());chests[i].setFromPixels(cell);trim(chests[i],400);}
        if(!envelopeArt.load("art/keepsake-envelope.png")||!paperArt.load("art/letter-paper.png")||!titleArt.load("art/archive-sign.png")||!moth.load("art/moth.png"))throw std::runtime_error("Missing prop art");
        trim(envelopeArt,144);trim(paperArt,500);trim(titleArt,478);trim(moth,28);
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
        repair.load(sourcePhoto);refreshPhoto();reader.load(ofToDataPath("digits/digits.png",true));
        for(int i=0;i<3;++i){writing[i]=cv::Mat::zeros(108,90,CV_8U);refreshInk(i);}
    }catch(const std::exception& e){ofLogError()<<e.what();errorMessage="MISSING RESOURCES";screen=Screen::Error;if(qaMode)ofExit(1);}
}
void ofApp::exit(){stopCamera();}
void ofApp::upload(const cv::Mat& image,ofTexture& texture){
    cv::Mat rgb;cv::cvtColor(image,rgb,cv::COLOR_BGR2RGB);ofPixels pixels;pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);
    texture.loadData(pixels);texture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
}
void ofApp::refreshPhoto(){upload(repair.image(),photoTexture);}
void ofApp::refreshInk(int i){
    cv::Mat display(writing[i].size(),CV_8UC3,cv::Scalar(184,239,255));
    display.setTo(cv::Scalar(32,35,63),writing[i]);upload(display,writingTextures[i]);
}
void ofApp::beginStory(){
    stopCamera();chapter=0;repair.reset();refreshPhoto();maze.reset();delivered=false;photoOpen=false;brushing=false;drawingCell=-1;
    recognized={-1,-1,-1};inkDue={0,0,0};notice.clear();moveDuration=0;brushRadius=19;filterIndex=0;intro=true;exhibitAt=-1;revisiting=false;lockAttempts=0;wrongHomeHint=false;trail.clear();
    for(int i=0;i<3;++i){writing[i].setTo(0);guesses[i]={};refreshInk(i);}
    outro=false;letterOpened=false;giftReturned=false;giftPhoto.clear();bellsPlayed=0;bellAt=-100;returnPhotoAt=liaLeftAt=-1;
    say(story::opening());
}
void ofApp::say(std::vector<Speech> lines,Screen next){
    dialogue=std::move(lines);speechIndex=0;afterDialogue=next;screen=Screen::Dialogue;dialogueAt=ofGetElapsedTimef();
    brushing=false;drawingCell=-1;palm.reset();lastPalmStamp=0;enterSpeech();
}
void ofApp::enterSpeech(){
    switch(dialogue[speechIndex].cue){
        case story::Cue::Doorbell:bellAt=ofGetElapsedTimef();++bellsPlayed;if(soundOn)doorbell.play();break;
        case story::Cue::ShowKeepsakes:exhibitAt=ofGetElapsedTimef();break;
        case story::Cue::ReturnPhoto:upload(repair.image(true),giftPhoto);giftReturned=true;returnPhotoAt=ofGetElapsedTimef();break;
        case story::Cue::LiaLeaves:liaLeftAt=ofGetElapsedTimef();break;
        case story::Cue::None:break;
    }
}
void ofApp::nextSpeech(){
    if(dialogue[speechIndex].cue==story::Cue::Doorbell&&ofGetElapsedTimef()-dialogueAt<.75f)return;
    if(++speechIndex>=dialogue.size()){
        screen=afterDialogue;intro=false;palm.reset();lastPalmStamp=0;
        if(screen==Screen::Letter){letterOpened=false;letterAt=ofGetElapsedTimef();}
        if(screen==Screen::Complete)outro=false;
    }else{dialogueAt=ofGetElapsedTimef();enterSpeech();}
}
void ofApp::openLetter(){if(!letterOpened){letterOpened=true;letterAt=ofGetElapsedTimef();}}
void ofApp::beginEnding(){outro=true;returnPhotoAt=liaLeftAt=-1;say(story::ending(),Screen::Complete);}
void ofApp::finishChapter(){
    photoOpen=false;
    if(chapter==0){
        if(revisiting){chapter=returnChapter;revisiting=false;return;}
        chapter=1;say({{"tonton","Let's try your code!\nWrite one number in each box.\nCheck the small windows, then press Open.",1},
            {"tonton","You can use more than one stroke.\nClear lets you try again.\nClick Photo to work on the photo."}});
    }else if(chapter==1){chapter=2;maze.reset();trail.clear();say({{"tonton","For little Tang... That's Lia's grandma!\nHer mom made her a little map.",2},
        {"tonton","Let's send the letter to her old home.\nStart camera. Put your hand\nin the middle of the camera view."},
        {"tonton","Move your hand to a side. Hold it there.\nWhen the ring is full, the letter will move.\nGo back to the middle to try again."}});
    }else {delivered=true;stopCamera();say({
        {"tonton","We found her home!\nThere is something on the back.",3},
        {"You","Let's turn it over, gently."},
        {"tonton","Click the folded corner\nto read the note.",1}},Screen::Letter);}
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
        if(screen!=Screen::Play||photoOpen||delivered||moveDuration>0){palm.reset();return;}
        auto direction=palm.update(hand.x,hand.y,hand.visible,dt,exits());
        if(direction!=homeward::Direction::None){lastWave=direction;waveAt=now;moveEnvelope(direction);}
    });
    if(hand.frames!=previewFrame&&!hand.preview.empty()){
        ofBuffer buffer(hand.preview.data(),hand.preview.size());ofPixels pixels;
        if(ofLoadImage(pixels,buffer)){cameraTexture.loadData(pixels);cameraTexture.setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);++cameraFrames;lastCameraFrame=now;}
        previewFrame=hand.frames;
    }
    if(!hand.running&&!hand.error.empty()){notice="Camera stopped. Try Camera again.";noticeUntil=now+6;palm.reset();}
    else if(now-lastCameraFrame>(hand.ready?6:35)){stopCamera();notice="Camera is not ready. Try again.";noticeUntil=now+6;}
}
void ofApp::moveEnvelope(homeward::Direction direction){
    if(screen!=Screen::Play||chapter!=2||photoOpen||delivered||moveDuration>0)return;
    moveFrom=maze.position;movePath=maze.route(maze.position,direction);
    if(!movePath.empty()){maze.position=movePath.back();moveAt=ofGetElapsedTimef();moveDuration=.16f+float(movePath.size())*.13f;trail.push_back(moveFrom);trail.insert(trail.end(),movePath.begin(),movePath.end());}
}
void ofApp::update(){
    float now=ofGetElapsedTimef();
    if(chapter==2&&(screen==Screen::Play||screen==Screen::Dialogue))updateCamera(now);
    if(moveDuration>0&&screen==Screen::Play&&now>=moveAt+moveDuration){moveDuration=0;
        if(maze.solved())finishChapter();
        else if(maze.house(maze.position)&&!wrongHomeHint){wrongHomeHint=true;say({{"tonton","This is not her home.\nLook for the red roof and the light."}});}
    }
    for(int i=0;i<3;++i)if(inkDue[i]>0&&now>=inkDue[i]&&drawingCell!=i){guesses[i]=reader.guess(writing[i]);recognized[i]=guesses[i].digit;inkDue[i]=0;}
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
    ofPushMatrix();ofTranslate(int(489+std::sin(t*.25f)*42),int(170+std::sin(t*.42f)*17));ofScale(1,.55f+.45f*std::abs(std::sin(t*7)));drawProp(moth,{-14,-10,28,20});ofPopMatrix();
    int bob=int(std::sin(t*.7f));ofPushMatrix();ofTranslate(0,bob);drawProp(titleArt,{81,23,478,163});
    centered("MIRROR ARCHIVE",320,101,light,4);ofPopMatrix();
    textInBox("Old things. New stories.",{172,193,296,28},light);
    button("START",startButton,true,3);button("Meet us",castButton);button("About",aboutButton);
    button(soundOn?"Sound on":"Sound off",soundButton);
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
        body("A small story about family and care.",72,91);
        label("How to play",72,121,wood);
        body("Mouse: brushes and writing.\nCamera: guide the letter with your hand.\nSpace: next line. Esc: back to the cover.",72,143);
        label("Credits & notice",72,211,wood);
        body("A fictional story. AI-assisted artwork.\nBuilt with openFrameworks, OpenCV and MediaPipe.\nFont: Departure Mono (OFL).\nYour camera stays on this computer.\nNo images are saved or uploaded.",72,233);
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
    ofSetColor(255);photoTexture.draw(photoArea);
    for(int x:{132,487}){rect(x,34,19,10,ofColor(183,159,111));rect(x+2,35,15,2,light);}
    const std::array<std::string,3> filters{"Clean","Contrast","Detail"};
    for(int i=0;i<3;++i)button(filters[i],{float(16+i*75),343,70,30},filterIndex==i);
    button("-",{246,343,24,30});button("+",{314,343,24,30});
    ofNoFill();ofSetColor(light);ofDrawCircle(292,358,brushRadius/4.f);ofFill();
    button("Undo",{352,343,59,30});
    button(revisiting?"Back to the box":"I found the password!",nextButton,true);
    if(screen==Screen::Play&&photoArea.inside(logical(ofGetMouseX(),ofGetMouseY()))){auto p=logical(ofGetMouseX(),ofGetMouseY());ofNoFill();ofSetColor(light);ofDrawCircle(int(p.x),int(p.y),brushRadius*.9f);ofFill();}
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

void ofApp::drawHouse(int x,int y,int kind){
    ofSetColor(255);float cell=buildings.getWidth()/3;
    buildings.drawSubsection(x-31,y-54,62,62,kind*cell,0,cell,buildings.getHeight());
}
void ofApp::drawMap(bool caption){
    ofSetColor(255);mapArt.draw(19,38,452,294);
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
    if(caption){paperFrame(176,43,146,23,paper);body("For little Tang",186,47,ink);}
    for(size_t i=1;i<trail.size();++i){auto a=mapPoint(trail[i-1]),b=mapPoint(trail[i]);if(cv::norm(trail[i]-trail[i-1])==1)line(a.x,a.y,b.x,b.y,ofColor(170,98,72));}
    // A bridge and a tiny pond are scenery; road topology remains explicit.
    rect(211,262,23,17,ofColor(103,158,163));for(int y=262;y<279;y+=4)rect(211,y,23,1,ofColor(161,190,167));
    for(int y=264;y<282;y+=4)rect(213,y,20,3,wood);
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
    paperFrame(481,41,148,190,ofColor(66,57,51));
    if(hand.running&&cameraTexture.isAllocated()){ofSetColor(255);cameraTexture.draw(487,69,136,102);rect(487,69,136,102,ofColor(25,29,33,60));}
    else {body("Show your hand",507,93,light);}
    body(hand.running?(hand.visible?(palm.ready()?"Choose a way":"Back to middle"):(cameraTexture.isAllocated()?"Show your hand":"Starting...")):"Hand guide",497,50,light);
    auto allowed=exits();
    auto arrow=[&](int x,int y,homeward::Direction d){auto v=homeward::step(d);ofSetColor(allowed[int(d)-1]?light:ofColor(130,121,103));
        ofDrawLine(x-v.x*5,y-v.y*5,x+v.x*5,y+v.y*5);ofDrawLine(x+v.x*5,y+v.y*5,x+v.y*3,y-v.x*3);ofDrawLine(x+v.x*5,y+v.y*5,x-v.y*3,y+v.x*3);};
    arrow(503,120,homeward::Direction::Left);arrow(607,120,homeward::Direction::Right);arrow(555,80,homeward::Direction::Up);arrow(555,160,homeward::Direction::Down);
    ofNoFill();ofSetColor(palm.ready()?gold:paper);ofDrawRectangle(539,108,32,24);ofFill();
    if(hand.running&&hand.visible){int x=487+hand.x*136,y=69+hand.y*102;rect(x-2,y-2,5,5,gold);
        int count=int(palm.progress()*16);for(int i=0;i<count;++i){float a=i*TWO_PI/16;rect(int(x+9*std::cos(a)),int(y+9*std::sin(a)),2,2,gold);}}
    // The same arrows also provide an optional mouse fallback, without a second pad.
    body("Hold for ring.",489,185,light);
    body("Back to middle.",489,202,light);
    button(hand.running?"Camera off":(cameraPending?"One moment...":"Start camera"),cameraButton,hand.running);
    paperFrame(496,280,120,53,paper);ofSetColor(255);photoTexture.draw(500,284,61,41);body("Photo",573,300);
    button("Start over",{21,343,91,30});
}

void ofApp::drawPlay(){
    drawDesk();if(chapter==0)drawPhoto();else if(chapter==1)drawLock();else drawMaze();
    if(ofGetElapsedTimef()<noticeUntil&&!notice.empty()){paperFrame(176,315,288,21,paper);body(notice,185,321);}
    if(photoOpen){rect(0,31,W,H-31,ofColor(32,23,29,220));paperFrame(99,35,442,298,paper);ofSetColor(255);photoTexture.draw(photoArea);button("Close",{558,45,64,26});}
}

void ofApp::drawDialogue(){
    if(intro)drawIntro();
    else if(outro)drawOutro();
    else if(chapter==2&&!delivered&&speechIndex==0&&dialogue[speechIndex].happy==2){
        drawDesk();drawChest({112,34,416,240},true);
        paperFrame(161,179,119,82,paper);ofSetColor(255);photoTexture.draw(166,184,109,72);
        drawEnvelope({345,171,114,76});
    }else drawPlay();
    rect(0,0,W,H,ofColor(28,21,27,50));
    const auto& speech=dialogue[speechIndex];
    paperFrame(18,278,604,95,paper);
    paperFrame(38,265,104,24,ofColor(114,72,49));textInBox(speech.speaker,{38,265,104,24},light);
    rasterText(speech.words,38,313,ink,true);
    if(speech.speaker!="You")rect(507,290,1,70,ofColor(208,179,130));
    if(speech.speaker=="tonton"){
        float age=ofGetElapsedTimef()-dialogueAt;int shake=age<.35&&speech.happy?int(std::sin(age*65)*2):0;
        pixel::tonton(562+shake,330,2,ofGetElapsedTimef(),speech.happy);
    }else if(speech.speaker=="Lia"){
        int pose=ofClamp(speech.happy,0,2);float cell=lia.getWidth()/3;
        ofSetColor(255);lia.getTexture().drawSubsection(520,282,88,88,cell*pose,0,cell,lia.getHeight());
    }
    int dot=int(ofGetElapsedTimef()*2)%3;for(int i=0;i<3;++i)rect(483+i*5,359,2,2,i==dot?wood:ofColor(210,175,120));
}

void ofApp::drawLetter(){
    drawDesk();float age=ofGetElapsedTimef()-letterAt;
    float flip=letterOpened&&age<.6f?std::max(.02f,float(std::abs(std::cos(age/.6f*PI)))):1;
    ofPushMatrix();ofTranslate(320,0);ofScale(flip,1);ofTranslate(-320,0);
    drawProp(paperArt,letterArea);
    if(!letterOpened||age<.3f){
        textInBox("For little Tang",{176,58,288,25},ink);
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
    ofSetColor(255);hero.draw(0,0,W,H);rect(0,0,W,H,ofColor(30,22,30,65));
    paperFrame(113,98,414,148,paper);
    rasterText("A light left on",199,140,ink,true);
    body("I put the tools away for tonight.\nThe next story can wait until morning.",150,174);
    button("Close the archive",{222,289,196,36},true);
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
        case Screen::Letter:drawLetter();break;case Screen::Complete:drawComplete();break;case Screen::Error:centered(errorMessage,320,180,light,2);break;}
    ofPopMatrix();canvas.end();
}
void ofApp::draw(){
    auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());renderCanvas(v.scale);
    ofBackground(shadow);ofSetColor(255);canvas.draw(v.x,v.y);
}
void ofApp::pointerDown(glm::vec2 p){
    if(screen==Screen::Home){
        if(startButton.inside(p))beginStory();
        else if(castButton.inside(p)){infoPage=0;screen=Screen::Info;}
        else if(aboutButton.inside(p)){infoPage=1;screen=Screen::Info;}
        else if(soundButton.inside(p)){soundOn=!soundOn;if(!soundOn)doorbell.stop();}
        return;
    }
    if(screen==Screen::Info){if(infoBack.inside(p))screen=Screen::Home;return;}
    if(screen==Screen::Dialogue){if(ofRectangle(18,266,604,107).inside(p))nextSpeech();return;}
    if(screen==Screen::Complete){if(ofRectangle(222,289,196,36).inside(p))screen=Screen::Home;return;}
    if(screen==Screen::Letter){
        if(!letterOpened&&letterArea.inside(p))openLetter();
        else if(letterOpened&&ofGetElapsedTimef()-letterAt>=.6f&&nextButton.inside(p))beginEnding();
        return;
    }
    if(screen!=Screen::Play)return;
    if(homeButton.inside(p)){stopCamera();moveDuration=0;screen=Screen::Home;brushing=false;drawingCell=-1;return;}
    if(photoOpen){photoOpen=false;palm.reset();return;}
    if(chapter==0){
        if(photoArea.inside(p)){repair.beginStroke();brushing=true;lastBrush={int((p.x-photoArea.x)*480/photoArea.width),int((p.y-photoArea.y)*320/photoArea.height)};pointerMove(p);}
        else if(ofRectangle(246,343,24,30).inside(p))brushRadius=std::max(7,brushRadius-5);
        else if(ofRectangle(314,343,24,30).inside(p))brushRadius=std::min(39,brushRadius+5);
        else if(ofRectangle(352,343,59,30).inside(p)){repair.undo();refreshPhoto();}
        else if(nextButton.inside(p))finishChapter();
        else for(int i=0;i<3;++i)if(ofRectangle(16+i*75,343,70,30).inside(p))filterIndex=i;
    }else if(chapter==1){
        if(ofRectangle(20,240,76,62).inside(p)){returnChapter=1;chapter=0;revisiting=true;return;}
        for(int i=0;i<3;++i){auto r=inkArea(i);
            if(ofRectangle(r.x+20,258,50,25).inside(p)){writing[i].setTo(0);recognized[i]=-1;guesses[i]={};inkDue[i]=0;refreshInk(i);return;}
            for(size_t j=0;j<guesses[i].candidates.size()&&guesses[i].candidates.size()>1;++j)if(ofRectangle(r.x+49+j*23,287,21,22).inside(p)){recognized[i]=guesses[i].candidates[j];return;}
            if(r.inside(p)){notice.clear();drawingCell=i;recognized[i]=-1;inkDue[i]=0;guesses[i]={};lastInk={int(p.x-r.x),int(p.y-r.y)};pointerMove(p);return;}
        }
        if(nextButton.inside(p)){
            for(int i=0;i<3;++i)if(inkDue[i]>0){guesses[i]=reader.guess(writing[i]);recognized[i]=guesses[i].digit;inkDue[i]=0;}
            if(recognized==std::array<int,3>{2,0,8})finishChapter();
            else{notice="The box is still locked.";noticeUntil=ofGetElapsedTimef()+3;
                if(++lockAttempts==2)say({{"tonton","Check the small number windows.\nIf they look right, check the photo."}});}
        }
    }else{
        if(delivered){if(nextButton.inside(p)){screen=Screen::Letter;letterOpened=false;letterAt=ofGetElapsedTimef();}return;}
        if(cameraButton.inside(p))toggleCamera();
        else if(ofRectangle(496,280,120,53).inside(p)){photoOpen=true;palm.reset();}
        else if(ofRectangle(21,343,91,30).inside(p)){maze.reset();trail.clear();moveFrom=maze.position;moveDuration=0;palm.reset();lastPalmStamp=0;}
        else if(ofRectangle(536,69,38,27).inside(p))moveEnvelope(homeward::Direction::Up);
        else if(ofRectangle(487,106,36,28).inside(p))moveEnvelope(homeward::Direction::Left);
        else if(ofRectangle(536,145,38,26).inside(p))moveEnvelope(homeward::Direction::Down);
        else if(ofRectangle(587,106,36,28).inside(p))moveEnvelope(homeward::Direction::Right);
    }
}
void ofApp::pointerMove(glm::vec2 p){
    if(screen!=Screen::Play||photoOpen)return;
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
    drawingCell=-1;brushing=false;
}
void ofApp::mousePressed(int x,int y,int b){if(b==0)pointerDown(logical(x,y));}
void ofApp::mouseDragged(int x,int y,int b){if(b==0)pointerMove(logical(x,y));}
void ofApp::mouseReleased(int x,int y,int b){if(b==0)pointerUp(logical(x,y));}
void ofApp::keyPressed(int key){
    if(key==OF_KEY_ESC){if(photoOpen){photoOpen=false;palm.reset();return;}stopCamera();moveDuration=0;brushing=false;drawingCell=-1;screen=Screen::Home;}
    else if(key==OF_KEY_RETURN||key==' '){if(screen==Screen::Home)beginStory();else if(screen==Screen::Dialogue)nextSpeech();else if(screen==Screen::Letter){if(!letterOpened)openLetter();else if(ofGetElapsedTimef()-letterAt>=.6f)beginEnding();}}
    else if(screen==Screen::Play&&chapter==2){
        if(key==OF_KEY_LEFT)moveEnvelope(homeward::Direction::Left);if(key==OF_KEY_RIGHT)moveEnvelope(homeward::Direction::Right);
        if(key==OF_KEY_UP)moveEnvelope(homeward::Direction::Up);if(key==OF_KEY_DOWN)moveEnvelope(homeward::Direction::Down);
    }
}
void ofApp::windowResized(int w,int h){if(w<W||h<H)ofSetWindowShape(std::max(w,W),std::max(h,H));}

void ofApp::runQA(){
    if(++qaFrame>(qaCamera?7500:6000)){ofLogError()<<"QA timed out at "<<qaStage;ofExit(1);return;}if(qaFrame%18)return;
    auto physical=[&](glm::vec2 p){auto v=mirror::pixelViewport(ofGetWidth(),ofGetHeight());return glm::ivec2(v.x+int(p.x*v.scale),v.y+int(p.y*v.scale));};
    auto click=[&](glm::vec2 p){auto q=physical(p);mousePressed(q.x,q.y,0);mouseReleased(q.x,q.y,0);};
    auto capture=[&](const std::string& name,int scale=2){if(hand.running||canvasHasCamera)return;renderCanvas(scale);auto folder=ofFilePath::getCurrentExeDir()+"../../../../qa/";ofDirectory::createDirectory(folder,true,true);ofPixels pixels;canvas.readToPixels(pixels);ofSaveImage(pixels,folder+name);};
    if(screen==Screen::Dialogue){
        auto cue=dialogue[speechIndex].cue;float age=ofGetElapsedTimef()-dialogueAt;
        if((cue==story::Cue::ShowKeepsakes&&age<.65f)||(cue==story::Cue::Doorbell&&age<1.f)||(cue==story::Cue::ReturnPhoto&&age<1.f))return;
        capture("qa-dialogue-"+std::to_string(chapter)+"-"+std::to_string(speechIndex)+"-"+std::to_string(dialogue[speechIndex].happy)+".png");
        if(cue==story::Cue::ShowKeepsakes){capture("qa-intro.png");capture("qa-type-3.png",3);capture("qa-type-4.png",4);}
        if(cue==story::Cue::Doorbell){capture("qa-doorbell.png");qaPassed&=bellsPlayed==1&&doorbell.isLoaded();}
        if(cue==story::Cue::ReturnPhoto){capture("qa-handover.png");qaPassed&=giftReturned&&giftPhoto.isAllocated();}
        if(cue==story::Cue::LiaLeaves)capture("qa-goodbye.png");
        click({400,350});return;
    }

    switch(qaStage){
        case 0:
            capture("qa-home.png");click(castButton.getCenter());qaPassed&=screen==Screen::Info;capture("qa-cast.png");click(infoBack.getCenter());
            click(aboutButton.getCenter());qaPassed&=screen==Screen::Info;capture("qa-about.png");click(infoBack.getCenter());
            click(soundButton.getCenter());qaPassed&=!soundOn;click(soundButton.getCenter());qaPassed&=soundOn;
            click(startButton.getCenter());++qaStage;break;
        case 1:{capture("qa-damaged.png");
            for(int tool=0;tool<3;++tool){click({float(46+tool*75),358});for(int y=50;y<327;y+=20){auto a=physical({110,float(y)});mousePressed(a.x,a.y,0);
                for(int x=110;x<534;x+=12){auto b=physical({float(x),float(y)});mouseDragged(b.x,b.y,0);}
                mouseReleased(a.x,a.y,0);}}
            ++qaStage;break;
        }
        case 2:capture("qa-case1.png");click(nextButton.getCenter());qaPassed&=chapter==1;++qaStage;break;
        case 3:{capture("qa-case2-blank.png");
            click(nextButton.getCenter());qaPassed&=chapter==1; // Empty lock cannot open.
            click({55,265});qaPassed&=chapter==0&&revisiting;click(nextButton.getCenter());qaPassed&=chapter==1&&!revisiting;
            // Draw separate mouse strokes, not injected recognized values.
            std::array<std::vector<glm::vec2>,3> strokes;
            strokes[0]={{14,26},{20,17},{35,12},{50,15},{59,24},{60,34},{55,44},{44,55},{30,67},{17,82},{63,82}};
            for(int j=0;j<=40;++j){float t=j*TWO_PI/40;strokes[1].push_back({45+24*std::cos(t),54+37*std::sin(t)});}
            for(int j=0;j<=64;++j){float t=j*TWO_PI/64;strokes[2].push_back({45+23*std::sin(2*t),54+36*std::cos(t)});}
            for(int i=0;i<3;++i){auto r=inkArea(i);auto q=physical({r.x+strokes[i][0].x,r.y+strokes[i][0].y});mousePressed(q.x,q.y,0);
                for(auto p:strokes[i]){q=physical({r.x+p.x,r.y+p.y});mouseDragged(q.x,q.y,0);}mouseReleased(q.x,q.y,0);}
            ++qaStage;break;
        }
        case 4:if(inkDue[0]>0||inkDue[1]>0||inkDue[2]>0)break;ofLogNotice()<<"QA handwriting: "<<recognized[0]<<recognized[1]<<recognized[2];qaPassed&=recognized==std::array<int,3>{2,0,8};capture("qa-case2.png");click(nextButton.getCenter());if(chapter!=2){ofLogError()<<"QA handwriting lock failed";ofExit(1);return;}++qaStage;break;
        case 5:
            if(qaMove==0){
                keyPressed(OF_KEY_RIGHT);qaPassed&=moveDuration>0;
                keyPressed(OF_KEY_ESC);qaPassed&=screen==Screen::Home&&moveDuration==0;
                click(startButton.getCenter());qaPassed&=chapter==0&&cv::countNonZero(repair.mask)==0&&recognized==std::array<int,3>{-1,-1,-1};
                qaMove=1;qaStage=1;break;
            }
            if(qaCamera&&!qaCameraChecked){
                if(qaCameraAt<0){qaCameraAt=ofGetElapsedTimef();click(cameraButton.getCenter());break;}
                if(cameraFrames>0&&qaFirstFrameAt<0)qaFirstFrameAt=ofGetElapsedTimef();
                if(qaFirstFrameAt<0&&ofGetElapsedTimef()-qaCameraAt<35)break;
                if(qaFirstFrameAt>=0&&ofGetElapsedTimef()-qaFirstFrameAt<5)break;
                bool connected=hand.running&&cameraTexture.isAllocated()&&cameraFrames>10;
                ofLogNotice()<<"QA palm camera: "<<(connected?"PASS":"FAIL")<<" ("<<cameraFrames<<" frames; none saved; startup "<<(qaFirstFrameAt<0?-1:qaFirstFrameAt-qaCameraAt)<<"s)";
                qaPassed&=connected;stopCamera();qaPassed&=!hand.running;
                maze.reset();moveFrom=maze.position;moveDuration=0;qaCameraChecked=true;break;
            }
            if(moveDuration>0)break;
            if(delivered){qaPassed&=screen==Screen::Letter;capture("qa-arrived.png");++qaStage;break;}
            capture("qa-case3.png");{auto route=maze.solution();if(route.empty()){ofExit(1);return;}
                int key=route[0]==homeward::Direction::Left?OF_KEY_LEFT:route[0]==homeward::Direction::Right?OF_KEY_RIGHT:route[0]==homeward::Direction::Up?OF_KEY_UP:OF_KEY_DOWN;keyPressed(key);}break;
        case 6:click({528,315});qaPassed&=letterOpened;++qaStage;break;
        case 7:if(ofGetElapsedTimef()-letterAt<.7)break;capture("qa-letter.png");click(nextButton.getCenter());++qaStage;break;
        case 8:qaPassed&=screen==Screen::Complete&&giftReturned&&liaLeftAt>=0;capture("qa-complete.png");ofLogNotice()<<"QA homeward story: "<<(qaPassed?"PASS":"FAIL");ofExit(qaPassed?0:1);break;
    }
}
