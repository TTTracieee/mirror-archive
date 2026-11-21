#include "ofApp.h"
#include <filesystem>
#include <sstream>

namespace {
const ofColor paper(243,239,229),ink(29,61,54),muted(113,124,111),gold(173,134,69);
const ofColor white(253,251,245),line(218,218,201),teal(78,132,117),rust(170,89,65);
constexpr float W=1440,H=940;
const std::array<std::string,3> titles={"The drowned constellation","An original among echoes","The moth that remembers"};
const std::array<std::string,3> chapters={"01 / THE LOST SEAL","02 / THE FORGER'S GALLERY","03 / THE NIGHT MIGRATION"};
const std::array<std::string,3> descriptions={
    "Salt, time and a faded photograph. Find the turquoise seal with seven points.",
    "Six convincing engravings. Only one shares the reference's microscopic details.",
    "Six identical moths. One travels against the current. Read its movement, not its face."};
const std::array<std::array<std::string,3>,3> toolNames={{{"Restore","Chroma","Contour"},{"Inspect","Keypoints","Authenticate"},{"Afterimage","Flow field","Counterflow"}}};
const std::array<std::array<std::string,3>,3> toolHints={{{"Median + intensity window","HSV + open / close","Denoise + Canny"},{"Magnify the etched details","SIFT scale + orientation","SIFT + ratio test + RANSAC"},{"Frame difference + threshold","Farneback optical flow","Filter measured leftward flow"}}};
const std::array<std::string,3> missions={
    "The inventory describes a turquoise, seven-point star. Several seals share its color. Several share its shape. Only one shares both.",
    "The original was rotated and resized before it disappeared. Compare local features with this reference. A convincing outline is not enough.",
    "The archive's moths drift east. One remembers the way home and flies west. Freeze time when you are ready to mark it."};
}

void ofApp::setup(){
    ofSetFrameRate(60);ofSetVerticalSync(true);ofEnableAlphaBlending();ofSetEscapeQuitsApp(false);
    cv::setNumThreads(2);
    ofSetCircleResolution(64);ofSetWindowTitle("Mirror Archive | A game of seeing");
    const std::string sans="/System/Library/Fonts/Avenir Next.ttc";
    const std::string serif="/System/Library/Fonts/Supplemental/Baskerville.ttc";
    const auto loadSans=[&](ofTrueTypeFont& font,int size,int index){
        ofTrueTypeFontSettings settings(sans,size);settings.index=index;settings.addRange(ofUnicode::Latin1Supplement);
        font.load(settings);
    };
    loadSans(bodyFont,15,7);loadSans(smallFont,12,5);loadSans(tinyFont,10,5);
    headingFont.load(serif,30,true,true);titleFont.load(serif,46,true,true);largeFont.load(serif,61,true,true);
    if(!bodyFont.isLoaded() || !headingFont.isLoaded())ofLogError()<<"Required macOS system fonts could not be loaded.";
    const auto resources=ofFilePath::getCurrentExeDir()+"../Resources/data/";
    if(ofDirectory::doesDirectoryExist(resources))ofSetDataPathRoot(resources);
    if(!hero.load("art/archive.png"))ofLogError()<<"Archive illustration missing from application resources.";
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
    scanMessage="No region examined yet.";lastPoint={450,280};
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
    notify(level==1?"Brush to inspect. Use Authenticate on a promising plate.":"Drag a lens over the photograph. Press E to keep a clue.");
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
}
void ofApp::refreshTexture(){
    int layer=level==0 && tool==2 && !cleanEdges?3:tool;
    display=mirror::VisionEngine::blend(data.observed,processed[layer],session.masks[layer]);
    upload(compare?data.observed:display,sceneTexture);dirty=false;
}
void ofApp::selectTool(int index){tool=std::clamp(index,0,2);claimMode=false;dirty=true;lastBrush={-1,-1};}
void ofApp::notify(const std::string& message){toastMessage=message;toastUntil=ofGetElapsedTimef()+4.5f;}
glm::vec2 ofApp::logical(float x,float y)const{
    float scale=std::min(ofGetWidth()/W,ofGetHeight()/H);
    return {(x-(ofGetWidth()-W*scale)/2)/scale,(y-(ofGetHeight()-H*scale)/2)/scale};
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
            notify("Not enough energy for this area. Use your existing clues or restart (R).");break;
        }
    }
    lastBrush=p;dirty=true;
}
void ofApp::scanAt(cv::Point point){
    if(!session.spend(9)){notify("Authentication needs 9 energy. Restart with R to try a new route.");return;}
    ++session.scans;
    const cv::Rect region(point.x-130,point.y-130,260,260);
    const auto result=mirror::VisionEngine::match(data.reference,data.observed,region);
    const cv::Rect clipped=region & cv::Rect(0,0,900,560);
    result.view(clipped).copyTo(processed[2](clipped));
    session.masks[2](clipped).setTo(255);
    scanMessage=ofToString(result.ratioMatches)+" ratio matches / "+ofToString(result.inliers)+" inliers";
    session.hasMatch|=result.verified;
    if(result.verified){session.evidence[2]=true;notify("Authentic detail found. Geometry agrees. Mark the original.");}
    else notify("No reliable geometric agreement. Try a different region.");
    dirty=true;
}
void ofApp::captureEvidence(){
    if(cv::countNonZero(session.masks[tool])<500){notify("Explore a region before collecting its evidence.");return;}
    if(level==2 && tool>0){
        cv::Mat motionMask;std::vector<cv::Mat> axes;cv::split(flow,axes);
        cv::compare(cv::abs(axes[0]),.55,motionMask,cv::CMP_GT);
        cv::bitwise_and(motionMask,session.masks[tool],motionMask);
        if(cv::countNonZero(motionMask)<30){notify("No movement measured here yet. Unfreeze and explore a moth.");return;}
    }
    if(level==1 && tool==2 && !session.hasMatch){notify("Collect a verified match before sealing this evidence.");return;}
    if(notes.size()>=6){notify("Notebook full. Your six clues are safe for this case.");return;}
    session.evidence[tool]=true;
    refreshTexture();
    cv::Mat rgb;cv::cvtColor(display,rgb,cv::COLOR_BGR2RGB);ofPixels pixels;
    pixels.setFromPixels(rgb.data,rgb.cols,rgb.rows,OF_PIXELS_RGB);ofImage snapshot;snapshot.setFromPixels(pixels);
    snapshots.push_back(snapshot);
    notes.push_back(toolNames[level][tool]+" / "+(level==1&&tool==2?scanMessage:toolHints[level][tool]));
    notify("Evidence added to your notebook (N). Energy unchanged.");
}
void ofApp::exportEvidence(){
    if(snapshots.empty()){notify("Keep a clue with E before exporting.");return;}
    const std::string folder=ofFilePath::getEnclosingDirectory(savePath)+"evidence/"+ofGetTimestampString("%Y%m%d-%H%M%S")+"/";
    try{
        std::filesystem::create_directories(folder);
        if(!std::filesystem::is_directory(folder))throw std::runtime_error("Cannot create export directory");
        for(size_t i=0;i<snapshots.size();++i)
            if(!snapshots[i].save(folder+"clue-"+ofToString(i+1)+".png"))throw std::runtime_error("Cannot save clue image");
        ofJson metadata;metadata["case"]=level+1;metadata["energy"]=session.energy;metadata["notes"]=notes;
        if(!ofSavePrettyJson(folder+"evidence.json",metadata))throw std::runtime_error("Cannot save evidence metadata");
        notebook=false;
        notify("Clues exported to Application Support / MirrorArchive / evidence.");
    }catch(const std::exception& e){notebook=false;notify(std::string("Export failed: ")+e.what());}
}
void ofApp::claimAt(cv::Point point){
    if(!session.ready(level)){notify(level==0?"Keep a Chroma clue and a Contour clue with E first.":level==1?"Authenticate the original before making your claim.":"Keep a measured Flow field or Counterflow clue with E first.");return;}
    if(!data.answer.contains(point)){
        ++session.mistakes;session.energy=std::max(0.f,session.energy-12);claimMode=false;
        notify("The evidence does not agree. -12 energy. Look again.");return;
    }
    session.solved=true;screen=Screen::Result;
    bestStars[level]=std::max(bestStars[level],session.stars());saveProgress();
}
void ofApp::useHint(){
    if(!session.spend(12)){notify("A hint needs 12 energy.");return;}
    ++session.hints;
    const std::array<std::string,3> hints={"Compare turquoise seals 01, 06, 08 and 11. Count the star tips.","The original is in the lower row. Match its small etched marks.","Inspect the fourth horizontal flight path. Gold arrows point west."};
    notify(hints[level]);toastUntil=ofGetElapsedTimef()+9;
}
void ofApp::saveProgress(){
    if(qaMode)return;
    try{ofJson j;j["bestStars"]=bestStars;if(!ofSavePrettyJson(savePath,j))throw std::runtime_error("Cannot write progress file");}
    catch(const std::exception& e){ofLogWarning()<<"Cannot save progress: "<<e.what();notify("Case solved. Progress could not be saved.");}
}
void ofApp::exit(){saveProgress();}

void ofApp::panel(float x,float y,float w,float h,ofColor color,float radius){ofSetColor(color);ofDrawRectRounded(x,y,w,h,radius);}
void ofApp::label(const std::string& text,float x,float y,ofColor color,int style){
    ofSetColor(color);
    auto& font=style==1?smallFont:style==2?headingFont:style==3?titleFont:style==4?tinyFont:style==5?largeFont:bodyFont;
    font.drawString(text,x,y);
}
void ofApp::paragraph(const std::string& text,float x,float y,float maxWidth,ofColor color,int style){
    auto& font=style==1?smallFont:bodyFont;
    std::istringstream words(text);std::string word,row;
    while(words>>word){
        auto next=row.empty()?word:row+" "+word;
        if(font.stringWidth(next)>maxWidth && !row.empty()){label(row,x,y,color,style);y+=style==1?21:26;row=word;}else row=next;
    }
    if(!row.empty())label(row,x,y,color,style);
}
void ofApp::button(const std::string& text,const ofRectangle& rect,bool active){
    bool hover=over(rect);panel(rect.x,rect.y,rect.width,rect.height,active?ink:hover?ofColor(226,232,218):white,10);
    label(text,rect.x+(rect.width-bodyFont.stringWidth(text))/2,rect.y+rect.height/2+6,active?paper:ink);
}
void ofApp::drawIcon(int icon,float x,float y,float size,ofColor color){
    ofPushStyle();ofSetColor(color);ofNoFill();ofSetLineWidth(1.7);
    if(icon==0){ofDrawCircle(x,y,size*.34);ofDrawLine(x+size*.25,y+size*.25,x+size*.48,y+size*.48);ofDrawLine(x-size*.18,y,x+size*.18,y);ofDrawLine(x,y-size*.18,x,y+size*.18);}
    else if(icon==1){for(int i=0;i<3;++i){float a=i*TWO_PI/3-HALF_PI;ofDrawCircle(x+cos(a)*size*.17,y+sin(a)*size*.17,size*.24);}}
    else if(icon==2){ofBeginShape();for(int i=0;i<14;++i){float a=i*PI/7-HALF_PI;float r=size*(i%2?.19:.43);ofVertex(x+cos(a)*r,y+sin(a)*r);}ofEndShape(true);}
    else{ofDrawRectangle(x-size*.32,y-size*.38,size*.64,size*.76);ofDrawLine(x-size*.16,y-size*.15,x+size*.16,y-size*.15);ofDrawLine(x-size*.16,y+size*.05,x+size*.16,y+size*.05);}
    ofPopStyle();
}

void ofApp::draw(){
    ofBackground(paper);ofPushMatrix();float scale=std::min(ofGetWidth()/W,ofGetHeight()/H);
    ofTranslate((ofGetWidth()-W*scale)/2,(ofGetHeight()-H*scale)/2);ofScale(scale,scale);
    if(screen==Screen::Home)drawHome();else if(screen==Screen::Play)drawPlay();else drawResult();
    if(help||notebook)drawModal();
    if(screen==Screen::Play && !help && !notebook && ofGetElapsedTimef()<toastUntil){
        float width=std::min(1020.f,smallFont.stringWidth(toastMessage)+40);
        float x=sceneRect.x+(sceneRect.width-width)/2;
        panel(x,254,width,40,ink,20);label(toastMessage,x+20,280,paper,1);
    }
    ofPopMatrix();
}
void ofApp::drawHome(){
    ofSetColor(255);if(hero.isAllocated())hero.draw(0,0,W,H);
    // Quiet opaque paper under text makes the illustrated menu readable at every size.
    panel(45,44,550,534,ofColor(246,242,232,234),24);
    drawIcon(2,88,88,35,ink);label("MIRROR ARCHIVE",118,95,ink,1);
    label("A GAME OF SEEING",81,160,gold,4);
    label("Every picture",77,248,ink,5);label("keeps a secret.",77,318,ink,5);
    paragraph("Three mysteries. A handful of light. Choose what to reveal, collect your evidence, and find what the eye alone can miss.",82,374,428,muted);
    button("Enter the archive  >",{81,478,280,57},true);
    label("MOUSE ONLY  /  NO CAMERA NEEDED",82,562,muted,4);
    panel(45,614,1350,280,ofColor(248,245,237,248),24);
    label("THE CASE FILES",73,650,gold,1);label("Choose any case. Return for a better route.",1040,650,muted,1);
    for(int i=0;i<3;++i){
        float x=73+i*444;
        ofRectangle rect(x,674,418,192);panel(x,674,418,192,over(rect)?ofColor(226,233,219):white,14);
        drawIcon(i,x+36,712,27,teal);label("0"+ofToString(i+1),x+361,716,gold,2);
        const std::array<std::string,3> shortTitles={"The lost seal","The forger's gallery","The night migration"};
        label(shortTitles[i],x+22,759,ink,2);
        paragraph(i==0?"Separate color from shape.":i==1?"Trust detail over resemblance.":"Make the invisible movement visible.",x+23,792,365,muted,1);
        label(bestStars[i]?ofToString(bestStars[i])+" / 3 SEALS EARNED":"UNOPENED CASE",x+23,840,gold,4);
        label("OPEN  >",x+320,840,ink,4);
    }
    label("An interactive image-processing mystery",57,922,muted,4);
    label("H  HOW TO PLAY",1250,922,muted,4);
}

void ofApp::drawPlay(){
    drawIcon(2,73,51,31,ink);label("MIRROR ARCHIVE",99,58,ink,1);
    label(chapters[level],502,58,gold,1);
    button("Notebook "+ofToString(notes.size()),{1028,29,167,44});button("?",{1208,29,47,44});button("Archive",{1268,29,116,44});
    ofSetColor(line);ofDrawLine(56,91,1384,91);
    label(titles[level],56,146,ink,2);
    label(claimMode?"MARK MODE  /  Click the object your evidence supports.":descriptions[level],57,177,muted,1);
    label("EXPLORATION ENERGY",1130,125,muted,4);
    label(ofToString(int(std::ceil(session.energy)))+" / 100",1272,151,ink);
    panel(1130,142,125,7,line,3);panel(1130,142,125*session.energy/100,7,session.energy<20?rust:teal,3);
    panel(51,193,1045,654,ofColor(214,211,194),15);
    ofSetColor(255);sceneTexture.draw(sceneRect);
    // Status chips stay outside the pixels used by every vision algorithm.
    panel(73,214,level==2?209:174,30,ofColor(29,61,54,229),15);
    label(compare?"ORIGINAL OBSERVATION":level==2?(session.paused?"TIME FROZEN / SPACE":"LIVE ARCHIVE / SPACE"):"OBSERVATION PLATE",85,235,paper,4);
    if(level==0 && tool==2){panel(75,789,327,32,ofColor(29,61,54,235),16);label(cleanEdges?"DENOISE ON / X to compare raw Canny":"RAW CANNY / X to remove the salt noise",89,811,paper,4);}
    if(level==1 && tool==2){panel(75,789,437,32,ofColor(29,61,54,235),16);label(scanMessage,89,811,paper,1);}
    auto mouse=logical(ofGetMouseX(),ofGetMouseY());
    if(sceneRect.inside(mouse)&&!help&&!notebook){
        ofPushStyle();ofNoFill();ofSetLineWidth(1.4);ofSetColor(white);
        float r=(level==1&&tool==2?130:radius)*sceneRect.width/900;
        if(claimMode){ofDrawCircle(mouse,15);ofDrawLine(mouse.x-24,mouse.y,mouse.x+24,mouse.y);ofDrawLine(mouse.x,mouse.y-24,mouse.x,mouse.y+24);}
        else if(level==1&&tool==2)ofDrawRectRounded(mouse.x-r,mouse.y-r,r*2,r*2,8);
        else ofDrawCircle(mouse,r);
        ofPopStyle();
    }
    panel(1117,198,267,644,white,16);
    label("THE CURATOR'S NOTE",1138,233,gold,4);
    paragraph(missions[level],1138,268,224,ink,1);
    float evidenceY=level==1?611:491;
    if(level==0){drawIcon(2,1250,427,62,teal);label("COLOR + SHAPE",1188,473,muted,4);}
    if(level==1){ofSetColor(255);referenceTexture.draw(1165,398,171,171);label("REFERENCE / ORIGINAL",1165,594,muted,4);}
    if(level==2){drawIcon(0,1249,421,57,teal);label("GOLD <   /   > TEAL",1179,473,muted,4);}
    label("EVIDENCE REQUIRED",1138,evidenceY,gold,4);
    if(level==0){
        label(std::string(session.evidence[1]?"[+]":"[ ]")+" Color isolated",1138,evidenceY+30,session.evidence[1]?teal:muted,1);
        label(std::string(session.evidence[2]?"[+]":"[ ]")+" Outline inspected",1138,evidenceY+57,session.evidence[2]?teal:muted,1);
    }else label(std::string(session.ready(level)?"[+] ":"[ ] ")+(level==1?"Geometry verified":"Motion measured"),1138,evidenceY+30,session.ready(level)?teal:muted,1);
    if(level!=1){paragraph("Drag to explore. E keeps a clue. Mark the object when the evidence agrees.",1138,606,222,muted,1);}
    button("Keep evidence  [E]",{1138,704,225,43});button("Hint  -12",{1138,760,105,40});button("Restart",{1254,760,109,40});
    label("1-3 LENSES   [ ] SIZE   TAB BEFORE",1138,827,muted,4);
    for(int i=0;i<3;++i){
        float x=56+i*282;bool active=tool==i&&!claimMode;
        panel(x,862,270,62,active?ink:ofColor(230,230,217),12);
        drawIcon(i,x+31,893,27,active?paper:teal);
        label(ofToString(i+1)+"  "+toolNames[level][i],x+57,886,active?paper:ink,1);
        label(toolHints[level][i],x+57,908,active?ofColor(177,199,181):muted,4);
    }
    button(claimMode?"Click your answer":"Mark the object  [M]",{918,862,240,62},true);
    label(level==1&&tool==2?"9 ENERGY / SCAN":"NEW AREA ONLY",1180,885,muted,4);
    label(level==1&&tool==2?"Click to authenticate":"BRUSH  "+ofToString(radius)+" px",1180,912,ink,1);
    if(level==1&&tool==0&&cv::countNonZero(session.masks[0])>0){
        panel(74,598,226,225,ink,12);ofSetColor(255);
        sceneTexture.drawSubsection(84,608,206,181,std::clamp(lastPoint.x-45,0,810),std::clamp(lastPoint.y-40,0,480),90,80);
        label("INSPECTION LOUPE / 2.3x",88,811,paper,4);
    }
}

void ofApp::drawResult(){
    if(hero.isAllocated()){ofSetColor(255,255,255,70);hero.draw(0,0,W,H);}
    panel(298,115,844,718,ofColor(253,251,245,248),24);
    label("CASE "+ofToString(level+1)+" / RESTORED TO THE ARCHIVE",477,173,gold,1);
    drawIcon(2,720,240,78,teal);
    label("A secret, brought to light.",397,335,ink,3);
    paragraph(level==0?"You separated color from geometry. Neither clue was enough alone.":level==1?"The engraving survived rotation and scale. Its local features still told the truth.":"Identical appearances hid different intentions. Movement revealed the impostor.",425,389,590,muted);
    for(int i=0;i<3;++i)drawIcon(2,655+i*65,491,42,i<session.stars()?gold:line);
    label(ofToString(int(std::ceil(session.energy)))+" energy left",425,567,ink);
    label(ofToString(session.mistakes)+" false claims",650,567,ink);
    label(ofToString(session.hints)+" hints used",865,567,ink);
    paragraph("Three seals: finish with 50+ energy, no false claims and no hints. Explore less. Observe more.",425,616,590,muted,1);
    button(level<2?"Open the next case  >":"Back to the archive",{425,695,359,57},true);
    button("Replay case",{803,695,211,57});
    label("R REPLAY   /   ENTER CONTINUE   /   ESC ARCHIVE",502,793,muted,4);
}
void ofApp::drawModal(){
    ofSetColor(20,40,32,170);ofDrawRectangle(0,0,W,H);
    panel(250,110,940,730,paper,23);button("Close  [Esc]",{1000,135,160,45});
    label(notebook?"Your evidence notebook":"The art of looking",291,178,ink,3);
    if(notebook){
        if(notes.empty())paragraph("A blank page, for now. Explore with a lens, then press E to pin the processed observation here.",295,250,680,muted);
        for(size_t i=0;i<notes.size();++i){float x=290+(i%3)*294,y=219+(i/3)*278;
            panel(x,y,276,253,white,12);ofSetColor(255);snapshots[i].draw(x+10,y+10,256,159);
            paragraph(notes[i],x+14,y+194,245,ink,1);
        }
        button("Export clues as PNG + JSON  [S]",{754,775,390,43});
    }else{
        const std::array<std::string,6> tips={
            "01   EXPLORE / Drag the mouse to brush a local lens across the observation. Keys 1, 2 and 3 switch lenses. Scroll or [ ] changes brush size.",
            "02   KEEP CLUES / E pins a snapshot to your notebook. N opens it. Case 01 needs both a Chroma and a Contour clue. Case 02 needs a verified match. Case 03 needs measured motion.",
            "03   MAKE A CLAIM / M selects the marking tool. Click the object you believe is correct. A false claim costs 12 energy. You can always restart with R.",
            "04   EXPERIMENT / Tab toggles the original observation. X switches Canny denoising in case 01. In case 02, Authenticate scans a square region for 9 energy.",
            "05   STOP TIME / Space freezes the moth sequence in case 03. Measured arrows remain visible so you can inspect and mark precisely.",
            "06   SPEND WISELY / New lens area uses energy; returning to an explored area is free. Keep 50 energy, avoid hints and false claims to earn all three seals."};
        for(int i=0;i<6;++i)paragraph(tips[i],294,237+i*87,843,ink,1);
        label("All effects are computed from the observation pixels. No camera, props or internet required.",294,797,teal,1);
    }
}

void ofApp::mousePressed(int x,int y,int buttonCode){
    if(buttonCode!=OF_MOUSE_BUTTON_LEFT)return;
    auto p=logical(x,y);
    if(help||notebook){
        if(ofRectangle(1000,135,160,45).inside(p)){help=false;notebook=false;}
        else if(notebook&&ofRectangle(754,775,390,43).inside(p))exportEvidence();
        return;
    }
    if(screen==Screen::Home){
        if(ofRectangle(81,478,280,57).inside(p)){loadLevel(0);return;}
        for(int i=0;i<3;++i)if(ofRectangle(73+i*444,674,418,192).inside(p)){loadLevel(i);return;}
        if(ofRectangle(1220,896,180,40).inside(p))help=true;
        return;
    }
    if(screen==Screen::Result){
        if(ofRectangle(425,695,359,57).inside(p)){if(level<2)loadLevel(level+1);else screen=Screen::Home;}
        else if(ofRectangle(803,695,211,57).inside(p))loadLevel(level);
        return;
    }
    if(ofRectangle(1028,29,167,44).inside(p)){notebook=true;return;}
    if(ofRectangle(1208,29,47,44).inside(p)){help=true;return;}
    if(ofRectangle(1268,29,116,44).inside(p)){screen=Screen::Home;return;}
    for(int i=0;i<3;++i)if(ofRectangle(56+i*282,862,270,62).inside(p)){selectTool(i);return;}
    if(ofRectangle(918,862,240,62).inside(p)){claimMode=!claimMode;return;}
    if(ofRectangle(1138,704,225,43).inside(p)){captureEvidence();return;}
    if(ofRectangle(1138,760,105,40).inside(p)){useHint();return;}
    if(ofRectangle(1254,760,109,40).inside(p)){loadLevel(level);return;}
    if(sceneRect.inside(p)){
        if(claimMode)claimAt(scenePoint(p));
        else{lastBrush={-1,-1};brushAt(p);dragging=!(level==1&&tool==2);}
    }
}
void ofApp::mouseDragged(int x,int y,int button){if(dragging&&screen==Screen::Play&&!help&&!notebook)brushAt(logical(x,y));}
void ofApp::mouseReleased(int,int,int){dragging=false;lastBrush={-1,-1};}
void ofApp::mouseScrolled(int,int,float,float sy){radius=std::clamp(radius+int(sy*8),28,138);}
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
