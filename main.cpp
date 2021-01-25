#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include "common.h"
#include "NormalLoader.h"
#include "AlbedoLoader.h"

GLuint programID;
GLuint VertexBufferID;
GLuint ColorBufferID;
GLuint NormalBufferID;
GLuint TransformBufferID;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);

glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 mvp;

float angle;
int face_num;
int outputIdx = 0; // outputfile 저장 인덱스
int screenSize = 800;
char RENDERMODE; // 나중에는 N이나 A를 인자로 받아서 자동으로 출력되도록 구현하자

point3 boundingCent; // 만약 잘 안되면 double로 바꿔볼 것
double boundMaxDist;

void saveScreen(int W, int H, int idx);
double boundingBox(point3 & boxCent);

std::vector<point3> out_vertices;
std::vector<point2> out_uvs;
std::vector<point3> out_normals;

/*
std::vector< glm::vec3 > out_vertices;
std::vector< glm::vec2 > out_uvs;
std::vector< glm::vec3 > out_normals;
*/

void timer(int value) {
    angle += glm::radians(20.0f);
    glutPostRedisplay();
    glutTimerFunc(20, timer, 0);
    if (outputIdx!=0 && outputIdx < 19)
    {
        saveScreen(screenSize, screenSize, outputIdx);
    }
    ++outputIdx;
}

void transform() {
    // Model matrix : an identity matrix (model will be at the origin)
    Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, angle, glm::vec3(0, 1, 0));

    // ModelViewProjection
    mvp = Projection * View * Model;

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
}

void init()
{
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    const char* objName = "model_normalized.obj";
    const char* mtlName = "model_normalized.mtl";

    std::cout << "press rendermode : N or n == Normal, A or a == Albedo\n";
    std::cin >> RENDERMODE;
    if (RENDERMODE == 'N' || RENDERMODE == 'n')
        bool res = loadNormal(objName, face_num, out_vertices, out_normals);

    else if (RENDERMODE == 'A' || RENDERMODE == 'a')
        bool res = loadAlbedo(objName, mtlName, face_num, out_vertices, out_uvs, out_normals);

    //bool res = loadNormal(objName, face_num, out_vertices, out_normals);

    // Bounding Box
    boundMaxDist = boundingBox(boundingCent);

    float scalingSize = 0.65f / boundMaxDist;
    glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scalingSize, scalingSize, scalingSize));
    glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-boundingCent.x, -boundingCent.y, -boundingCent.z));
    glm::mat4 transformedMatrix = translateMatrix * scalingMatrix;

    // homogeneous coordinate
    std::vector<point3> transformed_vertices;
    for (int i = 0; i < out_vertices.size(); ++i)
    {
        point4 temp_homo = point4(out_vertices[i].x, out_vertices[i].y, out_vertices[i].z); // (x,y.z.1 변환)
        float temp_output_homo[4] = { 0, };

        for (int j = 0; j < 3; ++j)
        {
            // 주의!
            // glm::mat4형을 디버그해보면 [0]~[3]은 열에 해당하는 정보를 담고 있었다. (x,y,z,w는 행에 해당하는 정보)
            if (j == 0)
            {
                temp_output_homo[j] += transformedMatrix[0].x * temp_homo.x;
                temp_output_homo[j] += transformedMatrix[1].x * temp_homo.y;
                temp_output_homo[j] += transformedMatrix[2].x * temp_homo.z;
                temp_output_homo[j] += transformedMatrix[3].x * temp_homo.w;
            }

            else if (j == 1)
            {
                temp_output_homo[j] += transformedMatrix[0].y * temp_homo.x;
                temp_output_homo[j] += transformedMatrix[1].y * temp_homo.y;
                temp_output_homo[j] += transformedMatrix[2].y * temp_homo.z;
                temp_output_homo[j] += transformedMatrix[3].y * temp_homo.w;
            }

            else  // j == 2
            {
                temp_output_homo[j] += transformedMatrix[0].z * temp_homo.x;
                temp_output_homo[j] += transformedMatrix[1].z * temp_homo.y;
                temp_output_homo[j] += transformedMatrix[2].z * temp_homo.z;
                temp_output_homo[j] += transformedMatrix[3].z * temp_homo.w;
            }          
        }

        point3 output_homo = point3(temp_output_homo[0], temp_output_homo[1], temp_output_homo[2]);
        transformed_vertices.push_back(output_homo);
    }


   
    glGenBuffers(1, &VertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, (transformed_vertices.size() * sizeof(point3)), &transformed_vertices[0], GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, (out_vertices.size() * sizeof(point3)), &out_vertices[0], GL_STATIC_DRAW);

    int tripleFace = 3 * face_num;
    std::vector<point3> colors;
    colors.assign(tripleFace, { 0.5, 0.5, 0.5});

    glGenBuffers(1, &ColorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
    glBufferData(GL_ARRAY_BUFFER, (colors.size() * sizeof(point3)), &colors[0], GL_STATIC_DRAW);

    glGenBuffers(1, &NormalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
    glBufferData(GL_ARRAY_BUFFER, (out_normals.size() * sizeof(point3)), &out_normals[0], GL_STATIC_DRAW);   

    programID = LoadShaders("deformed_3.vertexshader", "deformed_3.fragmentshader");
    glUseProgram(programID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

}

void myreshape(int w, int h)
{
    glViewport(0, 0, w, h);

    Projection = glm::perspective(glm::radians(45.0f),
        (float)w / (float)h, 0.1f, 100.0f);

    View = glm::lookAt(
        //glm::vec3(3, 4, 3), // Camera is at (3,4,3), in World Space
        glm::vec3(0, 1, -1), // 접시 최적 좌표
        // glm::vec3(tempX, tempY, tempZ),
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    transform();

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // ModelViewProjection
    glm::mat4 mvp = Projection * View * Model;

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
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
    glutInit(&argc, argv);
    glutInitWindowSize(screenSize, screenSize);
    glutInitWindowPosition(300, 150);
    glutCreateWindow("HELPME");
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutTimerFunc(0, timer, 0);
    glutDisplayFunc(mydisplay);
    glutReshapeFunc(myreshape); // 추가!

    GLenum err = glewInit();
    if (err == GLEW_OK) {
        init();
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
    point3 maxCoordX = point3(-9999, -9999, -9999);
    point3 minCoordX = point3(9999,9999,9999);    
    
    point3 maxCoordY = point3(-9999, -9999, -9999);
    point3 minCoordY = point3(9999, 9999, 9999);

    point3 maxCoordZ = point3(-9999, -9999, -9999);
    point3 minCoordZ = point3(9999, 9999, 9999);
    int num = 0;

    // 바운딩 박스의 8개 좌표들 구하기
    for (int i = 0; i < out_vertices.size(); i++)
    {
        point3 temp = out_vertices[i];

        if (maxCoordX.x < temp.x)
            maxCoordX = temp;

        if (minCoordX.x > temp.x)
            minCoordX = temp;

        if (maxCoordY.y < temp.y)
            maxCoordY = temp;

        if (minCoordY.y > temp.y)
            minCoordY = temp;

        if (maxCoordZ.z < temp.z)
            maxCoordZ = temp;

        if (minCoordZ.z > temp.z)
            minCoordZ = temp;
    }

    // 바운딩 박스의 중심의 좌표 구하기
    boxCent.x = (maxCoordX.x + minCoordX.x) / 2.f;
    boxCent.y = (maxCoordY.y + minCoordY.y) / 2.f;
    boxCent.z = (maxCoordZ.z + minCoordZ.z) / 2.f;

    // 바운딩 박스의 대각선 길이
    // maxCoordX를 기준으로 잡을 시, 절대 minCoordX와는 최대거리가 성립하지 않는다
    double maxDist = getDist(maxCoordX, maxCoordY);
    double cmpDist = getDist(maxCoordX, minCoordY);

    if (maxDist < cmpDist)
        maxDist = cmpDist;

    cmpDist = getDist(maxCoordX, maxCoordZ);
    if (maxDist < cmpDist)
        maxDist = cmpDist;

    cmpDist = getDist(maxCoordX, minCoordZ);
    if (maxDist < cmpDist)
        maxDist = cmpDist;

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
    //const char* filename = "output.bmp";
    char filename[100];
    sprintf(filename, "%c_output_%d.bmp", RENDERMODE, idx);

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
