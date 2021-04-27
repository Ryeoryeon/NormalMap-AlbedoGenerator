#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include<experimental/filesystem>
#include <bits/stdc++.h>
#include <io.h>
#include<random>
using namespace std;
namespace fs = experimental::filesystem;
#include<opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
using namespace cv;

random_device rd;
mt19937 mt(rd());
uniform_int_distribution<int> d(0, 1e9);

string loadFilePath = "D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\";
string category = "03001627";

vector<string> LoadFileList(string root) {
	vector<string> res;
	for (const auto& entry : fs::directory_iterator(root)) {
		auto cur = entry.path().native();
		string path(cur.begin(), cur.end());
		if (is_directory(entry.path())) {
			auto tmp = LoadFileList(path);
			for (auto i : tmp)
				res.push_back(i);
		}
		else
			res.push_back(path);
	}
	return res;
}

vector<string> LoadDirList(string root) {
	vector<string> res;
	for (const auto& entry : fs::directory_iterator(root)) {
		auto cur = entry.path().native();
		string path(cur.begin(), cur.end());
		if (is_directory(entry.path()))
			res.push_back(path);
	}

	return res;
}

int main() {
	auto res = LoadFileList(loadFilePath + category); // test
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\04379243\\"); // 책상 + 협탁 + 테이블
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\03593526\\"); // 화분 + 꽃병
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\04256520\\"); // 소파
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\03001627\\"); // 실내용 의자
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\02747177\\"); // 쓰레기통
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\02828884\\"); // 벤치
	//auto res = LoadFileList("D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\02958343\\"); // 자동차
	vector<string> obj, mtl;
	vector<string> dirName;
	for (auto i : res) {
		if (i.find(".obj") != string::npos) {
			obj.push_back(i);
			char * cur = (char*)i.c_str();
			char* name = strtok(cur, "\\");
			for (int i = 0; i < 5; ++i)
				name = strtok(0, "\\");
			dirName.push_back(name);
		}
		if (i.find(".mtl") != string::npos)
			mtl.push_back(i);

	}
	
	//for(auto i : ans)
	//	printf("%s\n", i.c_str());

	puts("File load complete"); 
	char msg[1010];
	for (int i = 0; i < obj.size(); ++i) {
		//string imageFilePath = "D:\\Lab\\ShapeNetCore.v2\\ShapeNetCore.v2\\test\\";
		string imageFilePath = loadFilePath + category + "\\";
		imageFilePath += dirName[i];
		imageFilePath += "\\images";
		//cout << imageFilePath << '\n';
		//cout << fs::is_empty(imageFilePath) << '\n';
		int exist = access(imageFilePath.c_str(), 00);
		if (exist < 0) // images 폴더가 존재하지 않는다면
		{
			sprintf(msg, "NormalLoader.exe %s %s %s N", obj[i].c_str(), mtl[i].c_str(), dirName[i].c_str());
			system(msg);
			sprintf(msg, "NormalLoader.exe %s %s %s A", obj[i].c_str(), mtl[i].c_str(), dirName[i].c_str());
			system(msg);
			sprintf(msg, "NormalLoader.exe %s %s %s I", obj[i].c_str(), mtl[i].c_str(), dirName[i].c_str());
			system(msg);
		}

	}
	
	int imageCount = 1;
	vector<Mat> images;


	/*
	puts("Image processing start");
	for (auto root : res) {
		for (int deg = 0; deg < 360; deg += 12) {
			Mat img1, img2, concatImg;
			char fName[232];

			sprintf(fName, "%s/models_r_%03d.png", root.c_str(), deg);
			img1 = imread(fName, IMREAD_UNCHANGED);

			sprintf(fName, "%s/models_r_%03d_normal.png0001.png", root.c_str(), deg);
			img2 = imread(fName, IMREAD_UNCHANGED);

			for (int i = 0; i < img1.rows; ++i) {
				for (int j = 0; j < img1.cols; ++j) {
					auto cur = img1.at<Vec4b>(i, j);
					if (cur[3] == 0) {
						for (int k = 0; k < 4; ++k)
							img1.at<Vec4b>(i, j)[k] = 255;
					}
				}
			}
			hconcat(img2, img1, concatImg);

			sprintf(fName, "../img/%d.jpg", imageCount++);
			images.push_back(concatImg);
		}
		if (images.size() > n) break;
	}
	puts("Image processing done");

	shuffle(images.begin(), images.end(), mt);

	int test = 1, train = 1, val = 1;
	int cnt = 0;

	puts("Image save start");
	const double testRatio = 0.1, valRatio = 0.1;
	for (auto img : images) {
		if (cnt % 100 == 0)
			printf("%d/%d done\n", cnt, n);
		char fName[505];
		if (cnt < n * testRatio)
			sprintf(fName, "../img/test/%d.jpg", test++);
		else if (cnt < n * (testRatio + valRatio))
			sprintf(fName, "../img/val/%d.jpg", val++);
		else
			sprintf(fName, "../img/train/%d.jpg", train++);

		++cnt;

		imwrite(fName, img);
	}
	*/
}