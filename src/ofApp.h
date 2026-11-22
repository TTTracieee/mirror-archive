#pragma once
#include "ofMain.h"
#include "GameModel.h"
#include "ArchiveGame.h"
#include "HandInput.h"
#include "CameraAccess.h"
#include "PixelGrid.h"
#include "Tonton.h"

class ofApp : public ofBaseApp {
public:
    explicit ofApp(bool qa=false):qaMode(qa){}
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
    enum class Screen {Home,Play,Transition,Complete,Error};
    Screen screen=Screen::Home;
    mirror::Campaign campaign;
    mirror::Restoration restoration;
    mirror::CombinationLock lock;
    mirror::PhotoPuzzle puzzle;
    mirror::HandInput hand;
    cv::Mat cleanRecord,damagedRecord,repairedRecord;
    ofTexture recordTexture,exhibitTexture;
    std::array<ofTexture,5> photoTextures;
    ofImage hero;
    ofFbo canvas;
    std::vector<int> order{0,1,2,3,4};
    glm::vec2 dragOffset{},handCursor{};
    int dragging=-1,selected=-1,slider=-1,wheel=-1,wheelSteps=0;
    float wheelStart=0,transitionAt=0,hintUntil=0,sparkleUntil=0,lastHandAt=0,cameraStartedAt=0;
    bool qaMode=false,qaPassed=true,handDown=false,recordOpen=false,exhibitReady=false,cameraPending=false;
    int qaStage=0,qaFrame=0,qaPiece=1;
    std::string errorMessage;
    void makeRecord(const cv::Mat& photo);
    void refreshRecord();
    void startStory();
    void solveChapter();
    void advanceStory();
    void pointerDown(glm::vec2 point,bool camera=false);
    void pointerMove(glm::vec2 point);
    void pointerUp(glm::vec2 point);
    void cancelDrag();
    void rotateSelected();
    void acceptSnap(int index);
    void upload(const cv::Mat& image,ofTexture& texture);
    void drawHome();
    void drawPlay();
    void drawRecord();
    void drawLock();
    void drawCompanion(int happy=0);
    void drawTransition();
    void drawComplete();
    void drawDesk();
    void button(const std::string& text,const ofRectangle& rect,bool active=false,int scale=1);
    void centered(const std::string& text,int centerX,int y,ofColor color,int scale=1);
    glm::vec2 logical(float x,float y) const;
    bool over(const ofRectangle& rect) const;
    void runQA();
};
