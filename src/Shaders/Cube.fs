#version 330 core
out vec4 FragColor;

uniform vec4 inputColor;

void main()
{
    FragColor = inputColor; // set alle 4 vector values to 1.0
}