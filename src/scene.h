//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "camera.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "streamer.h"
#include <mutex>
#include <thread>

class Scene {
public:
    Scene();
    void SetUpEnv();
    void SetObjs(); // Set objs for drawing
    void SetCallback();
    void DrawScene();
    void Terminate();
    void AttachStreamer(Streamer* streamer);
    Camera* get_camera();
private:
    void Notify();
    Streamer* streamer_; // This can be changed to a list
private:
    uint8_t* buf;
    GLFWwindow* window;
    unsigned int VBO;
    unsigned int VAO;
    Shader* shader;
    Camera* camera;
    glm::vec3 lightPos;
};


#endif
