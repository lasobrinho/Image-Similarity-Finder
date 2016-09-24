//
//  main.cpp
//  OpenCVTest
//
//  Created by Lucas Alves Sobrinho on 8/26/16.
//  Copyright © 2016 Lucas Alves Sobrinho. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int main(int argc, char const *argv[]) {
    
    vector<vector<vector<int> > > histogram(4, vector<vector<int> >(3, vector<int>(256, 0)));
    IplImage *dst;
    CvScalar v;
    string imagePath;
    ostringstream oss;
    int numberOfImages = 1000;

    for (int image = 0; image < numberOfImages; image++) {
        oss << "/Users/lucas/Documents/Fall 2016/opencv_images/" << image << ".jpg";
        imagePath = oss.str();
        dst = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_COLOR);
    }
    
//    int s = 0;
//    for (vector<vector<int> > i : hist) {
//        for (vector<int> j : i) {
//            for (int k : j) {
//                s++;
//                cout << k << " ";
//            }
//            cout << endl;
//            cout << s << endl;
//            s = 0;
//        }
//    }
    
    return 0;
}















