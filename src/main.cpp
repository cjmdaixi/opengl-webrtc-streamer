#include <ctime>
#include <cstdio>
#include <thread>

#include "streamer.h"
#include "scene.h"

void SetScene(Scene* scene){
    scene->SetUpEnv();
    scene->SetObjs();
    scene->SetCallback();
}

void do_main(int argc,char* argv[])
{
    uint8_t buffer[SCR_WIDTH*SCR_HEIGHT*3];
    Scene* scene = new Scene();
    Streamer* streamer = new Streamer();
    SetScene(scene);

    // TODO: process input parameters
    // TODO: multi thread
    scene->DrawScene(buffer);
}

int main(int argc,char* argv[])
{
    do_main(argc,argv);
    return 0;
}

