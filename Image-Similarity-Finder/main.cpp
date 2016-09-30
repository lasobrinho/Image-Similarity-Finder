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
#include <fstream>

using namespace std;

/* --------------------------------------------------------------------- */
/* Enumerations                                                          */

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

/* --------------------------------------------------------------------- */
/* Constants                                                             */

const DivisionType numberOfDivisions = TWO_BY_TWO;
const int numberOfImages = 3;

/* --------------------------------------------------------------------- */
/* Structs                                                               */

typedef struct HistogramElement
{
    Color color;
    float value;
    float normalizedValue;
    HistogramElement (Color c, float v, float nV) : color(c), value(v), normalizedValue(nV) {};
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
    vector<DivisionElements > limits;
    int pixelsByDivision;
}
DivisionLimits;

typedef struct Point2D_INT
{
    int X;
    int Y;
    Point2D_INT (int pX, int pY) : X(pX), Y(pY) {};
}
Point2D_INT;

/* --------------------------------------------------------------------- */
/* Functions                                                             */

void getDivisionLimits(IplImage *image, DivisionLimits *imageDivisions) {
    int sqrtDivisions = sqrt((double) numberOfDivisions);
    int heightStep = (image->height)/sqrtDivisions;
    int widthStep = (image->width)/sqrtDivisions;
    int divisionNumber = 0;
    imageDivisions->pixelsByDivision = widthStep * heightStep;
    for (int i = 0; i < sqrtDivisions; i++) {
        for (int j = 0; j < sqrtDivisions; j++) {
            imageDivisions->limits.push_back(DivisionElements(divisionNumber, j*widthStep, i*heightStep, (j+1)*widthStep, (i+1)*heightStep));
            divisionNumber++;
        }
    }
}

bool isInDivision(Point2D_INT *pixelPosition, DivisionElements *divElem) {
    if (pixelPosition->X >= divElem->startX &&
        pixelPosition->X <  divElem->endX   &&
        pixelPosition->Y >= divElem->startY &&
        pixelPosition->Y <  divElem->endY)
    {
        return true;
    }
    return false;
}

int getDivisionNumber(DivisionLimits *imageDivisions, Point2D_INT *pixelPosition) {
    for (DivisionElements divElem : imageDivisions->limits) {
        if (isInDivision(pixelPosition, &divElem)) {
            return divElem.divisionNumber;
        }
    }
    return 0;
}

void populateHistogram(vector<vector<HistogramElement> > *histogram, DivisionLimits *imageDivisions, CvScalar *sc, Point2D_INT *pixelPosition) {
    int divisionNumber = getDivisionNumber(imageDivisions, pixelPosition);
    for (int c = 0; c < 3; c++) {
        HistogramElement histElem(Color(c), sc->val[c], sc->val[c]/imageDivisions->pixelsByDivision);
        histogram->at(divisionNumber).push_back(histElem);
    }
}

void saveHistogramToFile(vector<vector<HistogramElement> > *histogram, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << *image << ".jpg" << endl;
    int divisionNumber = 0;
    for (vector<HistogramElement> histElemVector : *histogram) {
        outputFile << divisionNumber << endl;
        divisionNumber++;
        for (HistogramElement histElem : histElemVector) {
            outputFile << histElem.color << " " << histElem.normalizedValue << ",";
        }
        outputFile << endl;
    }
}

void removeOldHistogramFile(string *outputFileName) {
    ifstream outputFile;
    outputFile.open(*outputFileName);
    if (outputFile.is_open()) {
        outputFile.close();
        remove(outputFileName->c_str());
    }
}

void clearHistogram(vector<vector<HistogramElement> > *histogram) {
    for (vector<HistogramElement> histElem : *histogram) {
        histElem.clear();
    }
}


int main(int argc, char const *argv[]) {
    
    vector<vector<HistogramElement> > histogram(numberOfDivisions);
    DivisionLimits imageDivisions;
    IplImage *currentImage;
    CvScalar sc;
    string imagePath;
    ostringstream oss;
    
    string outputFileName = "normalizedHistogram.txt";
    removeOldHistogramFile(&outputFileName);
    ofstream outputFile;
    outputFile.open(outputFileName, ios::app);
    
    for (int image = 0; image < numberOfImages; image++) {
        oss << "/Users/lucas/Documents/Fall 2016/opencv_images/" << image << ".jpg";
        imagePath = oss.str();
        currentImage = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_COLOR);
        
        getDivisionLimits(currentImage, &imageDivisions);
        
        for (int x = 0; x < currentImage->width; x++) {
            for (int y = 0; y < currentImage->height; y++) {
                sc = cvGet2D(currentImage, y, x);
                Point2D_INT pixelPosition(x, y);
                populateHistogram(&histogram, &imageDivisions, &sc, &pixelPosition);
            }
        }
        
        saveHistogramToFile(&histogram, outputFile, &image);
        clearHistogram(&histogram);
        oss.str(string());
    }
    
    outputFile.close();
    
    return 0;
}















