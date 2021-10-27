#include <ctime>
#include <cstdio>

#include "scene.h"
#include "streamer.h"
void SetScene(Scene* scene){
    scene->SetUpEnv();
    scene->SetObjs();
    scene->SetCallback();
}

void do_main(int argc,char* argv[])
{
    Scene* scene = new Scene();
    Streamer* streamer = new Streamer(*scene);

    SetScene(scene);
    scene->AttachStreamer(streamer);
    streamer->SetUpEnv();

    scene->DrawScene();
    streamer->EndStream();
    scene->Terminate();
    return;
}

int main(int argc,char* argv[])
{
    do_main(argc,argv);
    return 0;
}

