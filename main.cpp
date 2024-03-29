#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include "common.h"
#include "NormalLoader.h"
#include "AlbedoLoader.h"
#include <opencv2/opencv.hpp>
#include<string>

using namespace cv;

std::string saveFName;

GLuint programID;
GLuint VertexBufferID;
GLuint ColorBufferID;
GLuint ambientColorBufferID;
GLuint specularColorBufferID;
GLuint NormalBufferID;
GLuint LightID;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);

glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 mvp;
glm::mat4 transformedMatrix;

glm::mat4 rotate30Matrix = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0, 1.0f, 0));
glm::mat4 rotateCounter30Matrix = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0, 1.0f, 0));

glm::vec3 eyePosition = glm::vec3(0, 1, 1);
glm::vec3 originPosition = glm::vec3(0, 0, 0);

float angle;
int face_num;
int screenSize = 256;
char RENDERMODE;

point3 boundingCent; // 만약 잘 안되면 double로 바꿔볼 것
double boundMaxDist;
float scalingFactor = 0.63f;

void openglToPngSave(int outputIdx);
void saveScreen(int W, int H, int idx);
double boundingBox(point3 & boxCent);

std::vector<point3> out_vertices;
std::vector<point2> out_uvs;
std::vector<point3> out_normals;

// .mtl파일을 읽어올경우 필요
std::vector<point3> specularColors;
std::vector<point3> ambientColors;
std::vector<point4> diffuseColors;

// directional Light
std::vector<point3> lightDir;
std::vector<point4> boundingBoxCoordinate;

int lightIdx = 0; // 조명 번호
//std::vector<point3> lightPos;

void timer(int value) {
    static int cnt = -1;
    static int outputIdx = -1;

    glutPostRedisplay();
    glutTimerFunc(30, timer, 0);
    angle += glm::radians(30.0f);

    if (RENDERMODE == 'I' || RENDERMODE == 'i')
    {
        if (lightIdx != lightDir.size())
            glUniform3f(LightID, lightDir[lightIdx].x, lightDir[lightIdx].y, lightDir[lightIdx].z);
    }

    ++outputIdx;
    ++cnt;

    if (outputIdx != 0 && outputIdx <= 12)
        openglToPngSave(outputIdx - 1);

    // 여러 개의 light를 사용할 때
    if (RENDERMODE == 'I' || RENDERMODE == 'i')
    {
        if (cnt == 12 * lightDir.size())
            exit(0);

        // 360도 회전이 끝나면 다음 조명으로 변경 (0~11, 12~23..)
        else if (cnt % 12 == 11)
        {
            ++lightIdx;
            outputIdx = 0;
        }
    }

    else
    {
        if (cnt == 12)
            exit(0);
    }

}

void transform() {
    // Model matrix : an identity matrix (model will be at the origin)
    Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, angle, glm::vec3(0, 1, 0));

    // ModelViewProjection
    mvp = Projection * View * Model;

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
    //
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);

    // 변환 행렬
    GLuint TransformedMatrixID = glGetUniformLocation(programID, "Transform");
    glUniformMatrix4fv(TransformedMatrixID, 1, GL_FALSE, &transformedMatrix[0][0]);
    //

    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
}

void init(int argc, char ** argv)
{
    //glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    /*
    const char* objName = "model_normalized.obj";
    const char* mtlName = "model_normalized.mtl";   
    */

    char* objName = argv[1];
    char* mtlName = argv[2];
    std::cout << "press rendermode : N or n == Normal, A or a == Albedo, I or i == Illumination model (Transparency X), T or t == Illumination model (Transparency O)\n";
    RENDERMODE = argv[4][0];
    //std::cin >> RENDERMODE;
    if (RENDERMODE == 'N' || RENDERMODE == 'n')
    {
        bool res = loadNormal(objName, mtlName, face_num, out_vertices, out_normals);
        programID = LoadShaders("Normal.vertexshader", "Normal.fragmentshader");
        glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    }
        
    else if (RENDERMODE == 'A' || RENDERMODE == 'a')
    {
        bool res = loadAlbedo(RENDERMODE, objName, mtlName, face_num, out_vertices, diffuseColors, ambientColors, specularColors, out_normals);
        programID = LoadShaders("Albedo.vertexshader", "Albedo.fragmentshader");
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    }

    else if (RENDERMODE == 'I' || RENDERMODE == 'i')
    {
        bool res = loadAlbedo(RENDERMODE, objName, mtlName, face_num, out_vertices, diffuseColors, ambientColors, specularColors, out_normals);
        programID = LoadShaders("illumination.vertexshader", "illumination.fragmentshader");
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f);

        lightDir.push_back(point3(0, 2, 1)); // 조명의 첫 위치
        point3 pushTemp;
        glm::vec4 temp(lightDir[0].x, lightDir[0].y, lightDir[0].z, 1);

        for (int j = 0; j < 3; ++j) // 30도 회전 3번
        {
            temp = rotate30Matrix * temp;
            pushTemp = point3(temp.x, temp.y, temp.z);
            lightDir.push_back(pushTemp);
        }

        temp = glm::vec4(lightDir[0].x, lightDir[0].y, lightDir[0].z, 1);

        for (int j = 0; j < 3; ++j) // -30도 회전 3번
        {
            temp = rotateCounter30Matrix * temp;
            pushTemp = point3(temp.x, temp.y, temp.z);
            lightDir.push_back(pushTemp);
        }
    }

    else if (RENDERMODE == 'T' || RENDERMODE == 't')
    {
        bool res = loadAlbedo(RENDERMODE, objName, mtlName, face_num, out_vertices, diffuseColors, ambientColors, specularColors, out_normals);
        programID = LoadShaders("Tr_illumination.vertexshader", "Tr_illumination.fragmentshader");
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    }

    //bool res = loadNormal(objName, face_num, out_vertices, out_normals);

    // Bounding Box
    boundMaxDist = boundingBox(boundingCent);

    float scalingSize = scalingFactor / boundMaxDist;
    glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scalingSize, scalingSize, scalingSize));
    glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-boundingCent.x, -boundingCent.y, -boundingCent.z));
    transformedMatrix = scalingMatrix * translateMatrix;

    // 행렬과 좌표 곱 계산은 GPU에서

    glGenBuffers(1, &VertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    //glBufferData(GL_ARRAY_BUFFER, (transformed_vertices.size() * sizeof(point3)), &transformed_vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, (out_vertices.size() * sizeof(point3)), &out_vertices[0], GL_STATIC_DRAW);

    int tripleFace = 3 * face_num;
    //std::vector<point3> colors;
    //colors.assign(tripleFace, { 0.5, 0.5, 0.5});

    if (RENDERMODE == 'A' || RENDERMODE == 'a')
    {
        // diffuse
        glGenBuffers(1, &ColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(point4)), &diffuseColors[0], GL_STATIC_DRAW);

        // ambient
        glGenBuffers(1, &ambientColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, ambientColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(point3)), &ambientColors[0], GL_STATIC_DRAW);

        // specular
        glGenBuffers(1, &specularColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, specularColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(point3)), &specularColors[0], GL_STATIC_DRAW);       
    }

    else if (RENDERMODE == 'I' || RENDERMODE == 'i' || RENDERMODE == 'T' || RENDERMODE == 't')
    {
        // diffuse
        glGenBuffers(1, &ColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(point4)), &diffuseColors[0], GL_STATIC_DRAW);

        // ambient
        glGenBuffers(1, &ambientColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, ambientColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(point3)), &ambientColors[0], GL_STATIC_DRAW);

        // specular
        glGenBuffers(1, &specularColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, specularColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(point3)), &specularColors[0], GL_STATIC_DRAW);

        // dissolve
        /*
        glGenBuffers(1, &dissolveBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, dissolveBufferID);
        glBufferData(GL_ARRAY_BUFFER, (tripleFace * sizeof(float)), &dissolveColors[0], GL_STATIC_DRAW);
        */
    }

    else
    {
        std::vector<point3> colors;
        colors.assign(tripleFace, { 0.5, 0.5, 0.5 });

        glGenBuffers(1, &ColorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
        glBufferData(GL_ARRAY_BUFFER, (colors.size() * sizeof(point3)), &colors[0], GL_STATIC_DRAW);
    }

    glGenBuffers(1, &NormalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
    glBufferData(GL_ARRAY_BUFFER, (out_normals.size() * sizeof(point3)), &out_normals[0], GL_STATIC_DRAW);   

    //programID = LoadShaders("Normal.vertexshader", "Normal.fragmentshader");
    glUseProgram(programID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    if (RENDERMODE == 'T' || RENDERMODE == 't')
    {
        // 투명도 적용을 위한 블렌드함수
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }

    glCullFace(GL_BACK);

}

void myreshape(int w, int h)
{
    glViewport(0, 0, w, h);

    Projection = glm::perspective(glm::radians(45.0f),
        (float)w / (float)h, 0.1f, 100.0f);

    View = glm::lookAt(
        eyePosition,
        originPosition,
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // transform 내부에서 중복
    /*
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // ModelViewProjection
    glm::mat4 mvp = Projection * View * Model;

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);   
    */

    transform();
}


GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);

    if (VertexShaderStream.is_open())
    {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else
    {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open())
    {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if (InfoLogLength > 0)
    {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if (InfoLogLength > 0)
    {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}


void mydisplay() {

    transform();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    if (RENDERMODE == 'A' || RENDERMODE == 'a')
    {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
        glVertexAttribPointer(
            2,                                // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

    }  


    else if (RENDERMODE == 'I' || RENDERMODE == 'i' || RENDERMODE == 'T' || RENDERMODE == 't')
    {

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
        glVertexAttribPointer(
            2,                                // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // ambient
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, ambientColorBufferID);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // specular
        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, specularColorBufferID);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    }

    // RENDERMODE == N
    else
    {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
        glVertexAttribPointer(
            2,                                // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );
    }

    glDrawArrays(GL_TRIANGLES, 0, (3*face_num));

    // Starting from vertex 0; 3 vertices -> 1 triangle
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    //glDisableVertexAttribArray(2);

    //glFlush();
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    char** tmp = new char* [1];
    tmp[0] = argv[0];
    int ac = 1;
    glutInit(&ac, tmp);
    glutInitWindowSize(screenSize, screenSize);
    glutInitWindowPosition(0, 800);
    glutCreateWindow("Albedo, Nomral, Illumianation Loader");
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutTimerFunc(0, timer, 0);
    glutDisplayFunc(mydisplay);
    glutReshapeFunc(myreshape); // 추가!
    saveFName = argv[3];
    GLenum err = glewInit();
    if (err == GLEW_OK) {
        for (int i = 0; i < argc; ++i)
            printf("%s ", argv[i]);
        puts("");
        init(argc, argv);
        glutMainLoop();
    }
}


double getDist(point3 p1, point3 p2)
{
    float xDist = pow(p1.x - p2.x, 2);
    float yDist = pow(p1.y - p2.y, 2);
    float zDist = pow(p1.z - p2.z, 2);

    return sqrt(xDist + yDist + zDist);
}

double boundingBox(point3 & boxCent)
{
    float maxCoordX = std::numeric_limits<float>::min();
    float maxCoordY = std::numeric_limits<float>::min();
    float maxCoordZ = std::numeric_limits<float>::min();

    float minCoordX = std::numeric_limits<float>::max();
    float minCoordY = std::numeric_limits<float>::max();
    float minCoordZ = std::numeric_limits<float>::max();

    int num = 0;

    // 바운딩 박스의 8개 좌표들 구하기
    for (int i = 0; i < out_vertices.size(); i++)
    {
        point3 temp = out_vertices[i];

        if (maxCoordX < temp.x)
            maxCoordX = temp.x;

        if (minCoordX > temp.x)
            minCoordX = temp.x;

        if (maxCoordY < temp.y)
            maxCoordY = temp.y;

        if (minCoordY > temp.y)
            minCoordY = temp.y;

        if (maxCoordZ < temp.z)
            maxCoordZ = temp.z;

        if (minCoordZ > temp.z)
            minCoordZ = temp.z;
    }

    // 바운딩 박스의 중심의 좌표 구하기
    boxCent.x = (maxCoordX + minCoordX) * 0.5f;
    boxCent.y = (maxCoordY + minCoordY) * 0.5f;
    boxCent.z = (maxCoordZ + minCoordZ) * 0.5f;

    boundingBoxCoordinate.push_back(point4(maxCoordX, maxCoordY, maxCoordZ, 1));

    boundingBoxCoordinate.push_back(point4(maxCoordX, maxCoordY, minCoordZ, 1));
    boundingBoxCoordinate.push_back(point4(maxCoordX, minCoordY, maxCoordZ, 1));
    boundingBoxCoordinate.push_back(point4(minCoordX, maxCoordY, maxCoordZ, 1));

    boundingBoxCoordinate.push_back(point4(maxCoordX, minCoordY, minCoordZ, 1));
    boundingBoxCoordinate.push_back(point4(minCoordX, maxCoordY, minCoordZ, 1));
    boundingBoxCoordinate.push_back(point4(minCoordX, minCoordY, maxCoordZ, 1));

    boundingBoxCoordinate.push_back(point4(minCoordX, minCoordY, minCoordZ, 1));

    // 바운딩 박스의 대각선 길이
    // maxCoordX를 기준으로 잡을 시, 절대 minCoordX와는 최대거리가 성립하지 않는다
    float maxDist = getDist(point3(maxCoordX, maxCoordY, maxCoordZ), boxCent);

    return maxDist;
}

// 화면 캡쳐해 bmp로 이미지 저장
void saveScreen(int W, int H, int idx)
{
    char * pixel_data;
    pixel_data = (char * )malloc(sizeof(char) * W * H * 3) ;

    BITMAPFILEHEADER bf; // 비트맵 파일 헤더
    BITMAPINFOHEADER bi; // 비트맵 정보 헤더

    glReadPixels(0, 0, W, H, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);

    char buff[256];
    //sprintf(buff, "cd D:/Lab/withHong");
    //sprintf(buff, "mkdir /res\\%s", saveFName.c_str());
    sprintf(buff, "mkdir D:\\Lab\\withHong\\res\\%s", saveFName.c_str());
    system(buff);
    //const char* filename = "output.bmp";
    char filename[1010];

    /*
    if(RENDERMODE != 'I' || RENDERMODE != 'i')
        sprintf(filename, "./res/%s/%c_output_%d.bmp", saveFName.c_str(), RENDERMODE, idx);

    else if(RENDERMODE == 'I' || RENDERMODE == 'i')
        sprintf(filename, "./res/%s/%c_output_%d_%d.bmp", saveFName.c_str(), RENDERMODE, idx, lightIdx);
    */

    if(RENDERMODE != 'I' && RENDERMODE != 'i')
        sprintf(filename, "D:/Lab/withHong/res/%s/%c_output_%d.bmp", saveFName.c_str(), RENDERMODE, idx);

    else
        sprintf(filename, "D:/Lab/withHong/res/%s/%c_output_%d_%d.bmp", saveFName.c_str(), RENDERMODE, idx, lightIdx);

    FILE* out = fopen(filename, "wb");
    char* data = pixel_data;

    memset(&bf, 0, sizeof(bf));
    memset(&bi, 0, sizeof(bi));

    bf.bfType = 'MB'; // 비트맵 파일 확장자를 위하여
    bf.bfSize = sizeof(bf) + sizeof(bi) + W * H * 3;
    bf.bfOffBits = sizeof(bf) + sizeof(bi);
    bi.biSize = sizeof(bi);
    bi.biWidth = W;
    bi.biHeight = H;
    bi.biPlanes = 1; // 사용하는 색상판의 수 (항상 1)
    bi.biBitCount = 24; // 픽셀 하나를 표현하는 비트 수
    bi.biSizeImage = W * H * 3;

    fwrite(&bf, sizeof(bf), 1, out);
    fwrite(&bi, sizeof(bi), 1, out);
    fwrite(data, sizeof(unsigned char), H * W * 3, out);

    fclose(out);
    free(pixel_data);
}

void openglToPngSave(int outputIdx)
{
    static int fileNo = 0;
    int bitsNum;
    GLubyte* bits; //RGB bits
    GLint captureImage[4]; //current viewport

    //get current viewport
    glGetIntegerv(GL_VIEWPORT, captureImage); // 이미지 크기 알아내기

    int rows = captureImage[3];
    int cols = captureImage[2];

    bitsNum = 3 * cols * rows;
    bits = new GLubyte[bitsNum]; // opengl에서 읽어오는 비트

    //read pixel from frame buffer
    glFinish(); //finish all commands of OpenGL

    glPixelStorei(GL_PACK_ALIGNMENT, 1); //or glPixelStorei(GL_PACK_ALIGNMENT,4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glReadPixels(0, 0, cols, rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, bits);

    Mat outputImage(rows, cols, CV_8UC3);
    int currentIdx;

    for (int i = 0; i < outputImage.rows; i++)
    {
        for (int j = 0; j < outputImage.cols; j++)
        {
            // stores image from top to bottom, left to right
            currentIdx = (rows - i - 1) * 3 * cols + j * 3; // +0

            outputImage.at<Vec3b>(i, j)[0] = (uchar)(bits[currentIdx]);
            outputImage.at<Vec3b>(i, j)[1] = (uchar)(bits[++currentIdx]); // +1
            outputImage.at<Vec3b>(i, j)[2] = (uchar)(bits[++currentIdx]); // +2
        }
    }

    char buff[256];
    //sprintf(buff, "cd D:/Lab/withHong");
    //sprintf(buff, "mkdir /res\\%s", saveFName.c_str());
    sprintf(buff, "mkdir D:\\Lab\\withHong\\res\\%s", saveFName.c_str());
    system(buff);
    //const char* filename = "output.bmp";
    char filename[1010];

    //sprintf(filename, "%c_output_%d.bmp", RENDERMODE, idx);
    if (RENDERMODE != 'I' && RENDERMODE != 'i')
        sprintf(filename, "D:/Lab/withHong/res/%s/%c_output_%d.png", saveFName.c_str(), RENDERMODE, outputIdx);

    else
        //sprintf(filename, "%c_output_%d_%d.bmp", RENDERMODE, idx, lightIdx);
        sprintf(filename, "D:/Lab/withHong/res/%s/%c_output_%d.png", saveFName.c_str(), RENDERMODE, fileNo++);

    imwrite(filename, outputImage);
    delete[] bits;
}