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
    Color color;
    float value;
    HistogramElement (Color c, float v) : color(c), value(v) {};
}
HistogramElement;

typedef struct DivisionElements
{
    int divisionNumber;
    int startX;
    int startY;
    int endX;
    int endY;
    DivisionElements (int dN, int sX, int sY, int eX, int eY) : divisionNumber(dN), startX(sX), startY(sY), endX(eX), endY(eY) {};
}
DivisionElements;

typedef struct DivisionLimits
{
    vector<DivisionElements > limits{numberOfDivisions};
}
DivisionLimits;

typedef struct Point2D_INT
{
    int X;
    int Y;
    Point2D_INT (int pX, int pY) : X(pX), Y(pY) {};
}
Point2D_INT;

void getDivisionLimits(IplImage *image, DivisionLimits imageDivisions) {
    int sqrtDivisions = sqrt((double) numberOfDivisions);
    int heightStep = (image->height)/sqrtDivisions;
    int widthStep = (image->width)/sqrtDivisions;
    int divisionNumber = 0;
    for (int i = 0; i < sqrtDivisions; i++) {
        for (int j = 0; j < sqrtDivisions; j++) {
            divisionNumber++;
            imageDivisions.limits.push_back(DivisionElements(divisionNumber, i*heightStep, j*widthStep, (i+1)*heightStep, (j+1)*widthStep));
        }
    }
}

bool isInDivision(Point2D_INT *pixelPosition, DivisionElements *divElem) {
    if (pixelPosition->X >= divElem->startX &&
        pixelPosition->X <  divElem->endX   &&
        pixelPosition->Y <= divElem->startY &&
        pixelPosition->Y <  divElem->endY)
    {
        return true;
    }
    return false;
}

int getDivisionNumber(DivisionLimits *imageDivisions, Point2D_INT *pixelPosition) {
    int divisionNumber = 0;
    for (DivisionElements divElem : imageDivisions->limits) {
        if (isInDivision(pixelPosition, &divElem)) {
            divisionNumber = divElem.divisionNumber;
        }
    }
    return divisionNumber;
}

void populateHistogram(vector<vector<HistogramElement> > histogram, DivisionLimits *imageDivisions, CvScalar *sc, Point2D_INT *pixelPosition) {
    int divisionNumber = getDivisionNumber(imageDivisions, pixelPosition);
    HistogramElement histElem(RED, sc->val[0]);
    histogram.at(divisionNumber).push_back(histElem);
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
        
        for (int x = 0; x < currentImage->height; x++) {
            for (int y = 0; y < currentImage->width; y++) {
                sc = cvGet2D(currentImage, x, y);
                Point2D_INT pixelPosition(x, y);
                populateHistogram(histogram, &imageDivisions, &sc, &pixelPosition);
            }
        }
    }
    
    return 0;
}















