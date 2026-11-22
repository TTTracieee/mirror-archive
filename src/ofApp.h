#pragma once
#include "ofMain.h"
#include "HomewardGame.h"
#include "CameraAccess.h"
#include "PixelGrid.h"
#include "Tonton.h"
#include "HandInput.h"
#include <map>

class ofApp : public ofBaseApp {
public:
    explicit ofApp(bool qa=false,bool cameraCheck=false):qaMode(qa),qaCamera(cameraCheck){}
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void mousePressed(int x,int y,int button) override;
    void mouseDragged(int x,int y,int button) override;
    void mouseReleased(int x,int y,int button) override;
    void keyPressed(int key) override;
    void windowResized(int width,int height) override;
private:
    enum class Screen {Home,Play,Dialogue,Letter,Complete,Error};
    struct Speech {std::string speaker,words;int happy=0;};
    Screen screen=Screen::Home,afterDialogue=Screen::Play;
    std::vector<Speech> dialogue;
    size_t speechIndex=0;
    int chapter=0,drawingCell=-1,brushRadius=19,filterIndex=0,lockAttempts=0;
    bool intro=false,revisiting=false,wrongHomeHint=false;
    int returnChapter=1;
    std::array<float,3> inkDue{0,0,0};
    std::array<homeward::DigitGuess,3> guesses;
    bool brushing=false,photoOpen=false,cameraPending=false,delivered=false;
    bool qaMode=false,qaPassed=true,qaCamera=false,qaCameraChecked=false;
    int cameraFrames=0;
    float qaCameraAt=-1,qaFirstFrameAt=-1;
    bool canvasHasCamera=false;
    int qaFrame=0,qaStage=0,qaMove=0;
    float exhibitAt=-1;
    float dialogueAt=0,noticeUntil=0,moveAt=0,moveDuration=0,letterAt=0,lastCameraFrame=0;
    std::string notice,errorMessage;
    homeward::BrushRepair repair;
    homeward::DigitReader reader;
    homeward::Maze maze;
    homeward::PalmGuide palm;
    mirror::HandInput hand;
    double lastPalmStamp=0;
    unsigned long previewFrame=0;
    std::vector<cv::Point> movePath,trail;
    std::array<cv::Mat,3> writing;
    std::array<int,3> recognized{-1,-1,-1};
    std::array<ofTexture,3> writingTextures;
    cv::Mat sourcePhoto;
    cv::Point lastBrush{},lastInk{},moveFrom{1,7};
    std::map<int,ofTrueTypeFont> fonts;
    int drawScale=1;
    ofTexture photoTexture,cameraTexture;
    ofImage hero,lia,mapArt,buildings;
    ofFbo canvas;
    homeward::Direction lastWave=homeward::Direction::None;
    float waveAt=0;
    void beginStory();
    void say(std::vector<Speech> lines,Screen next=Screen::Play);
    void nextSpeech();
    void finishChapter();
    void refreshPhoto();
    void refreshInk(int cell);
    void toggleCamera();
    void stopCamera();
    void updateCamera(float now);
    void moveEnvelope(homeward::Direction direction);
    void pointerDown(glm::vec2 point);
    void pointerMove(glm::vec2 point);
    void pointerUp(glm::vec2 point);
    void upload(const cv::Mat& image,ofTexture& texture);
    void drawHome();
    void drawIntro();
    void renderCanvas(int scale);
    ofTrueTypeFont& fontFor(int pixels);
    float bodyWidth(const std::string& text);
    void rasterText(const std::string& text,float x,float baseline,ofColor color,bool speech=false);
    void body(const std::string& text,int x,int y,ofColor color=pixel::ink);
    void label(const std::string& text,int x,int y,ofColor color=pixel::ink);
    std::array<bool,4> exits() const;
    void drawDesk();
    void drawPlay();
    void drawPhoto();
    void drawLock();
    void drawMaze();
    void drawHouse(int x,int y,int kind);
    void drawDialogue();
    void drawLetter();
    void drawComplete();
    void button(const std::string& text,const ofRectangle& rect,bool active=false,int scale=2);
    void centered(const std::string& text,int centerX,int y,ofColor color,int scale=1);
    glm::vec2 logical(float x,float y) const;
    bool over(const ofRectangle& rect) const;
    void runQA();
};
