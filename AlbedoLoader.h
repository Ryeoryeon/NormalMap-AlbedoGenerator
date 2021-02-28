
struct point2
{
    point2(float a, float b) : x(a), y(b) {};
    point2() {};
    float x;
    float y;
    float z;
};

class MaterialData
{
    public:
        point3 Kd; // diffuse
        point3 Ka; // ambient
        point3 Ks; // specular
        int Ns; // specular exponent
        float d = 1; // .mtl 중 d가 없는 파일이 존재하므로 그럴 경우를 위해 초기값 지정
        std::string name;

        MaterialData() {};
};

bool loadAlbedo(const char* objName, const char* mtlName, int& face_num, std::vector<point3>& out_vertices, std::vector<point3>& diffuseColors, std::vector<point3>& ambientColors, std::vector<point3>& specularColors, std::vector<float>& dissolveColors ,std::vector<point3>& out_normals);
//bool loadAlbedo(const char* objName, const char* mtlName, int& face_num, std::vector<point3>& out_vertices, std::vector<point2>& out_uvs, std::vector<point3>& out_normals);
