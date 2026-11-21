#pragma once
#include "ofMain.h"
#include "GameModel.h"
#include "LevelFactory.h"
#include "VisionEngine.h"
#include "PixelGrid.h"
#include "PixelUI.h"
#include <array>

class ofApp : public ofBaseApp {
public:
    explicit ofApp(bool qa = false) : qaMode(qa) {}
    void setup() override;
    void update() override;
    void draw() override;
    void mousePressed(int x,int y,int button) override;
    void mouseDragged(int x,int y,int button) override;
    void mouseReleased(int x,int y,int button) override;
    void mouseScrolled(int x,int y,float sx,float sy) override;
    void keyPressed(int key) override;
    void windowResized(int width,int height) override;
    void exit() override;
private:
    enum class Screen { Home, Play, Result };
    Screen screen=Screen::Home;
    mirror::Session session;
    mirror::LevelData data;
    std::array<cv::Mat,4> processed;
    cv::Mat flow,previous,display;
    ofTexture sceneTexture,referenceTexture;
    ofImage hero;
    ofFbo canvas;
    ofRectangle sceneRect{14,55,450,280};
    std::array<int,3> bestStars{};
    int level=0,tool=0,radius=72,motionTick=0;
    int qaStage=0,qaFrame=0;
    bool qaMode=false,qaPassed=true,dragging=false,claimMode=false,help=false,notebook=false;
    bool cleanEdges=true,dirty=true,compare=false;
    glm::vec2 lastBrush{-1,-1};
    cv::Point lastPoint{450,280};
    float nextMotion=0,toastUntil=0;
    std::string toastMessage,scanMessage;
    std::vector<std::string> notes;
    std::vector<ofImage> snapshots;
    std::string savePath;

    void loadLevel(int index);
    void selectTool(int index);
    void brushAt(glm::vec2 position);
    void scanAt(cv::Point point);
    void claimAt(cv::Point point);
    void captureEvidence();
    void exportEvidence();
    void useHint();
    void notify(const std::string& message);
    void refreshTexture();
    void upload(const cv::Mat& image,ofTexture& texture);
    void saveProgress();
    void drawHome();
    void drawPlay();
    void drawResult();
    void drawModal();
    void drawIcon(int icon,float x,float y,float size,ofColor color);
    void label(const std::string& text,float x,float y,ofColor color,int style=0);
    void panel(float x,float y,float w,float h,ofColor color,float radius=16);
    void button(const std::string& text,const ofRectangle& rect,bool active=false);
    glm::vec2 logical(float x,float y) const;
    bool over(const ofRectangle& rect) const;
    cv::Point scenePoint(glm::vec2 point) const;
    void runQA();
};
