#pragma once

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GL/glut.h> // or <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp> // 추가!

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iostream>

struct point3
{
    point3(float a, float b, float c) : x(a), y(b), z(c) {};
    point3() {}; // 기본 생성자 추가
    float x;
    float y;
    float z;
};
