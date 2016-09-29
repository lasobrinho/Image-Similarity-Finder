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

typedef struct DivisionElements
{
    int startX;
    int startY;
    int endX;
    int endY;
    DivisionElements (int sX, int sY, int eX, int eY) : startX(sX), startY(sY), endX(eX), endY(eY) {};
}
DivisionElements;

typedef struct DivisionLimits
{
    vector<DivisionElements > limits{numberOfDivisions};
}
DivisionLimits;

void getDivisionLimits(IplImage *image, DivisionLimits imageDivisions) {
    int sqrtDivisions = sqrt((double) numberOfDivisions);
    int heightStep = (image->height)/sqrtDivisions;
    int widthStep = (image->width)/sqrtDivisions;
    for (int i = 0; i < sqrtDivisions; i++) {
        for (int j = 0; j < sqrtDivisions; j++) {
            imageDivisions.limits.push_back(DivisionElements(i*heightStep, j*widthStep, (i+1)*heightStep, (j+1)*widthStep));
        }
    }
}

int getDivision(DivisionLimits *imageDivisions, int x, int y) {
    // Implementation...
    return 0;
}

void populateHistogram(vector<vector<HistogramElement> > histogram, DivisionLimits *imageDivisions, CvScalar *sc, int x, int y) {
    int divisionNumber = getDivision(imageDivisions, x, y);
    // Implementation...
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
        
        for (int x = 0; i < currentImage->height; x++) {
            for (int y = 0; j < currentImage->width; y++) {
                sc = cvGet2D(currentImage, x, y);
                populateHistogram(histogram, &imageDivisions, &sc, x, y);
            }
        }
    }
    
    return 0;
}















