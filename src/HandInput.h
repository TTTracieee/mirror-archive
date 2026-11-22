#pragma once
#include <array>
#include <filesystem>
#include <sstream>
#include <string>
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <cmath>
extern char** environ;

namespace mirror {
class HandInput {
    pid_t child=-1;
    int output=-1;
    std::string buffer;
public:
    bool running=false,ready=false,visible=false,pinched=false;
    float x=0,y=0;
    std::array<std::array<float,2>,21> joints{};
    std::string error;
    std::vector<char> preview;
    unsigned long frames=0;
    double stamp=0;
    ~HandInput(){stop();}
    void stop(){
        if(child>0){kill(child,SIGTERM);waitpid(child,nullptr,0);child=-1;}
        if(output>=0){close(output);output=-1;}
        running=ready=visible=pinched=false;buffer.clear();preview.clear();frames=0;stamp=0;
    }
    bool start(const std::string& executableDirectory,const std::string& resources){
        stop();error.clear();
        std::filesystem::path dir(executableDirectory),python;
        if(const char* overridePath=std::getenv("MIRROR_PYTHON"))python=overridePath;
        else for(int i=0;i<10&&!dir.empty();++i,dir=dir.parent_path())
            if(std::filesystem::exists(dir/".venv/bin/python")){python=dir/".venv/bin/python";break;}
        if(python.empty()||!std::filesystem::exists(python)){error="SETUP CAMERA FIRST";return false;}
        int pipes[2];if(pipe(pipes)!=0){error="CAMERA UNAVAILABLE";return false;}
        posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_adddup2(&actions,pipes[1],STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&actions,pipes[0]);
        posix_spawn_file_actions_addclose(&actions,pipes[1]);
        const std::string exe=python.string(),script=resources+"/tracker.py",model=resources+"/hand_landmarker.task";
        char* args[]={const_cast<char*>(exe.c_str()),const_cast<char*>(script.c_str()),const_cast<char*>("--model"),const_cast<char*>(model.c_str()),nullptr};
        const int result=posix_spawn(&child,exe.c_str(),&actions,nullptr,args,environ);
        posix_spawn_file_actions_destroy(&actions);close(pipes[1]);
        if(result!=0){close(pipes[0]);child=-1;error="CAMERA UNAVAILABLE";return false;}
        output=pipes[0];fcntl(output,F_SETFL,fcntl(output,F_GETFL)|O_NONBLOCK);running=true;return true;
    }
    // Returns all state changes so a release is not lost when frames arrive together.
    template<class Callback> void poll(Callback onState){
        if(output<0)return;
        char bytes[4096];ssize_t count;
        while((count=read(output,bytes,sizeof(bytes)))>0)buffer.append(bytes,size_t(count));
        size_t end;
        while((end=buffer.find('\n'))!=std::string::npos){
            auto line=buffer.substr(0,end);buffer.erase(0,end+1);
            std::istringstream in(line);std::string tag;in>>tag;
            if(tag=="PALM"){
                double time;float nx,ny;
                if(in>>time>>nx>>ny&&std::isfinite(nx)&&std::isfinite(ny)&&nx>=0&&nx<=1&&ny>=0&&ny<=1){
                    stamp=time;x=nx;y=ny;visible=ready=true;onState();}
            }else if(tag=="FRAME"){
                std::string hex;in>>hex;
                if(hex.size()>524288||hex.size()%2)continue;
                auto digit=[](char c)->int{if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;return -1;};
                std::vector<char> decoded;decoded.reserve(hex.size()/2);bool valid=true;
                for(size_t i=0;i<hex.size();i+=2){int a=digit(hex[i]),b=digit(hex[i+1]);if(a<0||b<0){valid=false;break;}decoded.push_back(char(a*16+b));}
                if(valid&&!decoded.empty()){preview=std::move(decoded);++frames;ready=true;}
            }else if(tag=="HAND"){
                int down;float nx,ny;
                if(in>>nx>>ny>>down){x=nx;y=ny;pinched=down!=0;visible=true;ready=true;
                    for(auto& joint:joints)in>>joint[0]>>joint[1];onState();}
            }else if(tag=="LOST"){in>>stamp;visible=pinched=false;onState();}
            else if(tag=="READY"){ready=true;onState();}
            else if(tag=="ERROR"){error="CAMERA UNAVAILABLE";stop();onState();return;}
        }
        if(count==0){if(error.empty())error="CAMERA STOPPED";stop();onState();}
        if(buffer.size()>1048576){buffer.clear();visible=pinched=false;onState();}
    }
};
}
