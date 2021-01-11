
struct point2
{
    point2(float a, float b) : x(a), y(b) {};
    point2() {};
    float x;
    float y;
    float z;
};

bool loadAlbedo(const char* objName, int& face_num, std::vector<point3>& out_vertices, std::vector<point2>& out_uvs, std::vector<point3>& out_normals);
