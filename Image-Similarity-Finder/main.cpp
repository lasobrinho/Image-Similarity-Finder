//
//  main.cpp
//  Image-Similarity-Finder
//
//  Created by Lucas Alves Sobrinho on 8/26/16.
//  Copyright © 2016 Lucas Alves Sobrinho. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

enum Color {
    RED     = 0,
    GREEN   = 1,
    BLUE    = 2
};

enum DivisionType {
    ONE_BY_TWO      = 2,
    TWO_BY_TWO      = 4,
    THREE_BY_THREE  = 9,
    FOUR_BY_FOUR    = 16,
    FIVE_BY_FIVE    = 25
};

const DivisionType numberOfDivisions = TWO_BY_TWO;
const int numberOfImages = 1000;

typedef struct HistogramElement
{
    Color c;
    float value;
}
HistogramElement;

typedef struct DivisionLimits
{
    vector<tuple<int, int> > limits;
}
DivisionLimits;

void getDivisionLimits(IplImage *image, DivisionLimits imageDivisions) {
    int sqrtDivisions = sqrt((double) numberOfDivisions);
    int heightStep = (image->height)/sqrtDivisions;
    int widthStep = (image->width)/sqrtDivisions;
    imageDivisions.limits.push_back(make_tuple(0,0));
    for (int i = 1; i < sqrtDivisions; i++) {
        for (int j = 1; j < sqrtDivisions; j++) {
            imageDivisions.limits.push_back(make_tuple(i*heightStep, j*widthStep));
        }
    }
    imageDivisions.limits.push_back(make_tuple(image->height, image->width));
}

void populateHistogram(vector<vector<HistogramElement> > histogram, DivisionLimits *divisionLimits, CvScalar *sc, int i, int j) {
    
}

int main(int argc, char const *argv[]) {
    
    vector<vector<HistogramElement> > histogram(numberOfDivisions);
    DivisionLimits imageDivisions;
    IplImage *currentImage;
    CvScalar sc;
    string imagePath;
    ostringstream oss;

    for (int image = 0; image < numberOfImages; image++) {
        oss << "/Users/lucas/Documents/Fall 2016/opencv_images/" << image << ".jpg";
        imagePath = oss.str();
        currentImage = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_COLOR);
        
        getDivisionLimits(currentImage, imageDivisions);
        
        for (int i = 0; i < currentImage->height; i++) {
            for (int j = 0; j < currentImage->width; j++) {
                sc = cvGet2D(currentImage, i, j);
                populateHistogram(histogram, &imageDivisions, &sc, i, j);
            }
        }
    }
    
    return 0;
}















