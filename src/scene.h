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
#include <ft2build.h>
#include FT_FREETYPE_H
#include "streamer.h"
#include<chrono>

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

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
    int frame_count;
    float prev_time;
    float delta_t;
    void Notify(uint8_t* buf);
    void loadFonts();
    void renderText(std::string text,GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
    Streamer* streamer_; // This can be changed to a list
private:
    GLFWwindow* window;
    unsigned int VBO,tVBO;
    unsigned int VAO,tVAO;
    Shader* shader;
    Shader* textShader;
    Camera* camera;
    glm::vec3 lightPos;
    std::map<GLchar, Character> Characters;
};


#endif
