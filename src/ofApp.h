#pragma once
#include "ofMain.h"
#include "GameModel.h"
#include "LevelFactory.h"
#include "VisionEngine.h"
#include "PixelGrid.h"
#include "PixelUI.h"

class ofApp : public ofBaseApp {
public:
    explicit ofApp(bool qa=false):qaMode(qa){}
    void setup() override;
    void update() override;
    void draw() override;
    void mousePressed(int x,int y,int button) override;
    void mouseDragged(int x,int y,int button) override;
    void mouseReleased(int x,int y,int button) override;
    void keyPressed(int key) override;
    void windowResized(int width,int height) override;
private:
    enum class Screen {Home,Play,Transition,Complete,Error};
    Screen screen=Screen::Home;
    mirror::LevelFactory factory;
    mirror::Campaign campaign;
    mirror::Session session;
    mirror::PointerGesture gesture;
    mirror::LevelData data;
    cv::Mat processed,flow,previous,display;
    ofTexture sceneTexture,referenceTexture;
    ofImage hero,moth;
    ofFbo canvas;
    const ofRectangle sceneRect{12,48,480,300};
    glm::vec2 lastBrush{-1,-1};
    std::vector<cv::Point2f> matchPolygon;
    int motionTick=0,qaStage=0,qaFrame=0;
    bool qaMode=false,qaPassed=true,dirty=false;
    float nextMotion=0,nextScan=0,toastUntil=0,transitionAt=0,hintUntil=0;
    std::string toastMessage,errorMessage;

    void startStory();
    void loadChapter();
    void advanceStory();
    void advanceFilm();
    void brushAt(glm::vec2 position);
    mirror::MatchEvidence scanAt(cv::Point point);
    void claimAt(cv::Point point);
    void notify(const std::string& message);
    void refreshTexture();
    void upload(const cv::Mat& image,ofTexture& texture);
    void drawHome();
    void drawPlay();
    void drawTransition();
    void drawComplete();
    void button(const std::string& text,const ofRectangle& rect,bool active=false,int scale=1);
    void centered(const std::string& text,int centerX,int y,ofColor color,int scale=1);
    glm::vec2 logical(float x,float y) const;
    bool over(const ofRectangle& rect) const;
    cv::Point scenePoint(glm::vec2 point) const;
    glm::vec2 boardPoint(cv::Point point) const;
    void runQA();
};
