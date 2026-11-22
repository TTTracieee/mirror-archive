#include "ofMain.h"
#include "ofApp.h"
#include <string>

int main(int argc,char** argv) {
    bool qa=false,cameraCheck=false;
    for(int i=1;i<argc;++i){
        if(std::string(argv[i])=="--qa")qa=true;
        if(std::string(argv[i])=="--qa-camera")qa=cameraCheck=true;
    }
    ofGLFWWindowSettings settings;
    settings.setSize(1280,768);
    settings.numSamples=0;
    settings.title="Mirror Archive";
    settings.windowMode=OF_WINDOW;
    auto window=ofCreateWindow(settings);
    if(auto native=std::dynamic_pointer_cast<ofAppGLFWWindow>(window)){
        // OF requests framebuffer pixels; keep the initial window 1280x768 points.
        const int density=native->getPixelScreenCoordScale();
        native->setWindowShape(1280*density,768*density);
    }
    ofRunApp(window,std::make_shared<ofApp>(qa,cameraCheck));
    return ofRunMainLoop();
}
