#include "common.h"
#include "AlbedoLoader.h"
#include "NormalLoader.h"

bool loadNormal(const char* objName, const char * mtlName, int &face_num, std::vector<point3> &out_vertices, std::vector<point3> &out_normals)
{
    // .mtl파일 유효 검사를 위한 .mtl load
    // 유효하지 않은 .mtl파일일 경우, normal에 대한 이미지도 생성시키지 않아야 함

    FILE* fp2;
    fp2 = fopen(mtlName, "r");

    if (fp2 == NULL) {
        printf("Impossible to open the .mtl file !\n");
        return false;
    }

    std::vector<MaterialData> mtlData;
    bool first = true; // 첫번째 newmtl인가?
    MaterialData temp;

    while (1)
    {
        char lineHeader[128];

        int res = fscanf(fp2, "%s", lineHeader);

        if (res == EOF)
        {
            break;
        }

        if (strcmp(lineHeader, "newmtl") == 0)
        {
            if (first) // 아직 넣을 데이터가 없을 때
            {
                first = false;
                temp = MaterialData(); // temp 초기화
            }

            else
            {
                mtlData.push_back(temp);
                temp = MaterialData();
            }

            continue;

        }

        //                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               std::string lHeader;
        if (strcmp(lineHeader, "Kd") == 0) // diffuse color
        {
            fscanf(fp2, "%f %f %f\n", &temp.Kd.x, &temp.Kd.y, &temp.Kd.z);
        }

        else if (strcmp(lineHeader, "Ka") == 0) // ambient color
        {
            fscanf(fp2, "%f %f %f\n", &temp.Ka.x, &temp.Ka.y, &temp.Ka.z);
        }

        else if (strcmp(lineHeader, "Ks") == 0) // specular color
        {
            fscanf(fp2, "%f %f %f\n", &temp.Ks.x, &temp.Ks.y, &temp.Ks.z);
        }

        else if (strcmp(lineHeader, "d") == 0) // dissolve (transparency)
        {
            fscanf(fp2, "%f\n", &temp.d);
        }

        else if (strcmp(lineHeader, "Ns") == 0) // specular exponent
        {
            fscanf(fp2, "%d\n", &temp.Ns);
        }

        else if (strcmp(lineHeader, "Ke") == 0) // Ke는 건너뛰기
            continue;

        else if (strcmp(lineHeader, "illum") == 0) // Ke는 건너뛰기
            continue;

        // 나머지 문자열일 경우, material의 이름을 포함하고 있는지 확인해보자
        else
        {
            //char* originalName;
            char originalName[128];
            strcpy(originalName, lineHeader); //원본 보존
            char* nameTemp = strtok(lineHeader, "_");

            if (strcmp(nameTemp, "material") == 0)
                temp.name = originalName;

            else
                continue;
        }

    }

    mtlData.push_back(temp);
    // 유효한 mtlData인지 검사하기
    int mtlSize = mtlData.size();
    for (int i = 0; i < mtlSize; ++i)
    {
        if (mtlData[i].name == "")
            exit(0);
    }

    FILE* fp;
    fp = fopen(objName, "r");

    if (fp == NULL) {
        printf("Impossible to open the file !\n");
        return false;
    }

    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;

    std::vector<point3> temp_vertices;
    std::vector<point3> temp_normals;

    while (1)
    {
        char lineHeader[128];

        int res = fscanf(fp, "%s", lineHeader);
        if (res == EOF)
            break;

        // 첫 단어가 v인 경우, vertex를 읽는다
        if (strcmp(lineHeader, "v") == 0)
        {
            point3 vertex;
            fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }

        // 첫 단어가 vt라면 uv를 읽는다 
        /*
         else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(fp, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        */


        // 첫 단어가 vn이라면, normal을 읽는다
        else if (strcmp(lineHeader, "vn") == 0)
        {
            point3 normal;
            fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }

        // 첫 단어가 f라면, face를 읽는다
        else if (strcmp(lineHeader, "f") == 0)
        {
            unsigned int vertexIndex[3], normalIndex[3];
            std::vector<int> temp_facelist; // face table에 저장된 수들을 임시로 저장하는 벡터

            char str[128];
            fgets(str, sizeof(str), fp);
            char* ptr = strtok(str, " //");
            int ptrSize = 0;

            while (ptr != NULL) // 자른 문자열이 나오지 않을 때까지 출력
            {
                temp_facelist.push_back(atoi(ptr));
                ptr = strtok(NULL, " //");
                ++ptrSize;
            }

            // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 순으로 저장됨
            if (ptrSize == 9)
            {
                int temp1 = 0;
                int temp2 = 0;

                for (int i = 0; i < 9; i++)
                {
                    if (i % 3 == 0)
                    {
                        vertexIndex[temp1++] = temp_facelist[i];
                    }

                    else if (i % 3 == 2)
                    {
                        normalIndex[temp2++] = temp_facelist[i];
                    }
                }
            }

            // f v1//vn1 v2//vn2 v3//vn3 순으로 저장됨
            else if (ptrSize == 6)
            {
                int temp1 = 0;
                int temp2 = 0;

                for (int i = 0; i < 6; i++)
                {
                    if (i % 2 == 0)
                    {
                        vertexIndex[temp1++] = temp_facelist[i];
                    }

                    else
                    {
                        normalIndex[temp2++] = temp_facelist[i];
                    }
                }
            }

            else
            {
                std::cout << "This File can't be read by our simple parser : Try exporting with other options\n";
                return false;
            }

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);

            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);

            ++face_num;
        }

        // 첫 단어가 l일 때

        //
    }

    // 인덱싱 과정

    // 각 삼각형의 꼭짓점을 모두 순회
    for (int i = 0; i < vertexIndices.size(); i++)
    {
        unsigned int vertexIdx = vertexIndices[i];
        unsigned int normalIdx = normalIndices[i];
        // obj는 1부터 시작하지만, C++의 index는 0부터 시작하기 때문에
        point3 vertex = temp_vertices[vertexIdx - 1];
        point3 normal = temp_normals[normalIdx - 1];

        out_vertices.push_back(vertex);
        out_normals.push_back(normal);
    }

    fclose(fp);
    return true;
}