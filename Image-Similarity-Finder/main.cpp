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
const int numberOfImages = 1000;

/* --------------------------------------------------------------------- */
/* Structs                                                               */

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

void populateHistogram(vector<vector<vector<float>>> *histogram,
                       DivisionLimits *imageDivisions,
                       CvScalar *sc,
                       Point2D_INT *pixelPosition)
{
    int divisionNumber = getDivisionNumber(imageDivisions, pixelPosition);
    for (int c = 0; c < 3; c++) {
        histogram->at(divisionNumber).at(c).at((int) sc->val[c])++;
    }
}

void saveHistogramToFile(vector<vector<vector<float>>> *histogram, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << *image << ".jpg" << endl;
    
    int divisionNumber = 0;
    int colorNumber = 0;
    
    for (vector<vector<float>> divisionVector : *histogram) {
        outputFile << divisionNumber << endl;
        divisionNumber++;
        for (vector<float> colorVector : divisionVector) {
            outputFile << colorNumber << endl;
            colorNumber++;
            for (float value : colorVector) {
                outputFile << value << " ";
            }
            outputFile << endl;
        }
        colorNumber = 0;
        
    }
}

void savePercentileToFile(vector<vector<vector<float>>> *percentile, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << *image << ".jpg" << endl;
    
    int divisionNumber = 0;
    int colorNumber = 0;
    
    for (vector<vector<float>> divisionVector : *percentile) {
        outputFile << divisionNumber << endl;
        divisionNumber++;
        for (vector<float> colorVector : divisionVector) {
            outputFile << colorNumber << endl;
            colorNumber++;
            for (float value : colorVector) {
                outputFile << value << " ";
            }
            outputFile << endl;
        }
        colorNumber = 0;
        
    }
}

void saveFeaturesVectorToFile(vector<float> *features, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << *image << ".jpg" << endl;
    
    for (float value : *features) {
        outputFile << value << " ";
    }
    
}

void removeOldFile(string *outputFileName) {
    ifstream outputFile;
    outputFile.open(*outputFileName);
    if (outputFile.is_open()) {
        outputFile.close();
        remove(outputFileName->c_str());
    }
}

void clearHistogram(vector<vector<vector<float>>> *histogram) {
    for (vector<vector<float> > divisionVector : *histogram) {
        for (vector<float> colorVector : divisionVector) {
            colorVector.clear();
        }
    }
}

void clearPercentile(vector<vector<vector<float>>> *percentile) {
    for (vector<vector<float> > divisionVector : *percentile) {
        for (vector<float> colorVector : divisionVector) {
            colorVector.clear();
        }
    }
}

void clearColorFeatureVector(vector<float> *colorFeatureVector) {
    colorFeatureVector->clear();
}

void normalizeHistogram(vector<vector<vector<float>>> *histogram, DivisionLimits *imageDivisions) {
    int i = 0, j = 0, k = 0;
    for (vector<vector<float> > divisionVector : *histogram) {
        for (vector<float> colorVector : divisionVector) {
            for (float value : colorVector) {
                histogram->at(i).at(j).at(k) = value/imageDivisions->pixelsByDivision;
                k++;
            }
            k = 0;
            j++;
        }
        j = 0;
        i++;
    }
}

void populatePercentile(vector<vector<vector<float>>> *histogram, vector<vector<vector<float>>> *percentile) {
    int i = 0, j = 0, k = 0, last = 0;
    bool start = false;
    float sum = 0.0, p = 1.0;
    for (vector<vector<float> > divisionVector : *histogram) {
        for (vector<float> colorVector : divisionVector) {
            for (float value : colorVector) {
                if (p == 10) {
                    if (value != 0) {
                        last = k;
                    }
                }else if (start) {
                    if (sum < (p * 0.1)) {
                        sum = sum + value;
                    } else {
                        percentile->at(i).at(j).at(p) = k;
                        p++;
                    }
                }
                if (value != 0 && start == false) {
                    start = true;
                    percentile->at(i).at(j).at(0) = k;
                }
                k++;
            }
            percentile->at(i).at(j).at(10) = last;
            start = false;
            last = 0.0;
            sum = 0;
            p = 1.0;
            k = 0;
            j++;
        }
        j = 0;
        i++;
    }
}

void buildColorPercentile(vector<vector<vector<float>>> *histogram, vector<vector<vector<float>>> *percentile, IplImage *currentImage, DivisionLimits *imageDivisions, CvScalar *sc) {
    getDivisionLimits(currentImage, imageDivisions);
    for (int x = 0; x < currentImage->width; x++) {
        for (int y = 0; y < currentImage->height; y++) {
            *sc = cvGet2D(currentImage, y, x);
            Point2D_INT pixelPosition(x, y);
            populateHistogram(histogram, imageDivisions, sc, &pixelPosition);
        }
    }
    normalizeHistogram(histogram, imageDivisions);
    populatePercentile(histogram, percentile);
}

void buildFeatureVector(vector<vector<vector<float>>> *percentile, vector<float> *colorFeatureVector) {
    int i = 0, j = 0;
    for (vector<vector<float> > divisionVector : *percentile) {
        for (vector<float> colorVector : divisionVector) {
            for (int k = 0; k < colorVector.size() - 1; k++) {
                colorFeatureVector->push_back((percentile->at(i).at(j).at(k + 1) - percentile->at(i).at(j).at(k)) / 255);
            }
            j++;
        }
        j = 0;
        i++;
    }
}

int main(int argc, char const *argv[]) {
    
    vector<vector<vector<float>>> histogram(numberOfDivisions, vector<vector<float>>(3, vector<float>(256)));
    vector<vector<vector<float>>> percentile(numberOfDivisions, vector<vector<float>>(3, vector<float>(11)));
    vector<float> colorFeatureVector;
    
    DivisionLimits imageDivisions;
    IplImage *currentImage;
    CvScalar sc;
    string imagePath;
    ostringstream oss;
    
    string outputFileNameHistogram = "normalizedHistogram.txt";
    string outputFileNamePercentile = "normalizedPercentile.txt";
    string outputFileNameFeatures = "colorFeatureVector.txt";
    removeOldFile(&outputFileNameHistogram);
    removeOldFile(&outputFileNamePercentile);
    removeOldFile(&outputFileNameFeatures);
    ofstream outputFileHistogram;
    ofstream outputFilePercentile;
    ofstream outputFileFeatures;
    outputFileHistogram.open(outputFileNameHistogram, ios::app);
    outputFilePercentile.open(outputFileNamePercentile, ios::app);
    outputFileFeatures.open(outputFileNameFeatures, ios::app);
    
    for (int image = 0; image < numberOfImages; image++) {
        oss << "/Users/lucas/Documents/Fall 2016/opencv_images/" << image << ".jpg";
        imagePath = oss.str();
        currentImage = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_COLOR);
        
        // Functions calls to build color histogram and percentile
        buildColorPercentile(&histogram, &percentile, currentImage, &imageDivisions, &sc);
        saveHistogramToFile(&histogram, outputFileHistogram, &image);
        savePercentileToFile(&percentile, outputFilePercentile, &image);
        
        buildFeatureVector(&percentile, &colorFeatureVector);
        saveFeaturesVectorToFile(&colorFeatureVector, outputFileFeatures, &image);
        
        clearHistogram(&histogram);
        clearPercentile(&percentile);
        clearColorFeatureVector(&colorFeatureVector);
        
        oss.str(string());
    }
    
    outputFileHistogram.close();
    outputFilePercentile.close();
    outputFileFeatures.close();
    
    return 0;
}















