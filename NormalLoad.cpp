#include "common.h"
#include "NormalLoader.h"

bool loadNormal(const char* objName, int &face_num, std::vector<point3> &out_vertices, std::vector<point3> &out_normals)
{
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

        // ù �ܾ v�� ���, vertex�� �д´�
        if (strcmp(lineHeader, "v") == 0)
        {
            point3 vertex;
            fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }

        // ù �ܾ vt��� uv�� �д´� 
        /*
         else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(fp, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        */

        // ù �ܾ vn�̶��, normal�� �д´�
        else if (strcmp(lineHeader, "vn") == 0)
        {
            point3 normal;
            fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }

        // ù �ܾ f���, face�� �д´�
        else if (strcmp(lineHeader, "f") == 0)
        {
            unsigned int vertexIndex[3], normalIndex[3];
            std::vector<int> temp_facelist; // face table�� ����� ������ �ӽ÷� �����ϴ� ����

            char str[128];
            fgets(str, sizeof(str), fp);
            char* ptr = strtok(str, " //");
            int ptrSize = 0;

            while (ptr != NULL) // �ڸ� ���ڿ��� ������ ���� ������ ���
            {
                temp_facelist.push_back(atoi(ptr));
                ptr = strtok(NULL, " //");
                ++ptrSize;
            }

            // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ������ �����
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

            // f v1//vn1 v2//vn2 v3//vn3 ������ �����
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

        // ù �ܾ l�� ��

        //
    }

    // �ε��� ����

    // �� �ﰢ���� �������� ��� ��ȸ
    for (int i = 0; i < vertexIndices.size(); i++)
    {
        unsigned int vertexIdx = vertexIndices[i];
        unsigned int normalIdx = normalIndices[i];
        // obj�� 1���� ����������, C++�� index�� 0���� �����ϱ� ������
        point3 vertex = temp_vertices[vertexIdx - 1];
        point3 normal = temp_normals[normalIdx - 1];

        out_vertices.push_back(vertex);
        out_normals.push_back(normal);
    }

    fclose(fp);
    return true;
}