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

class Scene {
public:
    Scene();
    void SetUpEnv();
    void SetObjs(); // Set objs for drawing
    void SetCallback();
    void DrawScene(uint8_t * buf);
    void Terminate();
    Camera* get_camera();
public:
    bool buffer_read;
private:
    GLFWwindow* window;
    unsigned int VBO;
    unsigned int VAO;
    Shader* shader;
    Camera* camera;
    glm::vec3 lightPos;
};


#endif
