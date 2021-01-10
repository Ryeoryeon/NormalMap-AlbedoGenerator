#version 330 core

in vec3 fragmentColor;
in vec3 Normal;

out vec3 color;

void main(){
  //color = fragmentColor;
  color = Normal;
}
