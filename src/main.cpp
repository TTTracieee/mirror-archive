#include "ofMain.h"
#include "ofApp.h"
#include <string>

int main(int argc,char** argv) {
    bool qa=false;
    for(int i=1;i<argc;++i) if(std::string(argv[i])=="--qa") qa=true;
    ofGLFWWindowSettings settings;
    settings.setSize(1280,836);
    settings.title="Mirror Archive | A game of seeing";
    settings.windowMode=OF_WINDOW;
    auto window=ofCreateWindow(settings);
    ofRunApp(window,std::make_shared<ofApp>(qa));
    ofRunMainLoop();
}
