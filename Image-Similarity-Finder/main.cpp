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
    TWO_BY_TWO      = 4,
    THREE_BY_THREE  = 9,
    FOUR_BY_FOUR    = 16,
    FIVE_BY_FIVE    = 25
};

/* --------------------------------------------------------------------- */
/* Constants                                                             */

const DivisionType numberOfDivisions = TWO_BY_TWO;
const int numberOfImages = 250;

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

typedef struct Divisions
{
    vector<DivisionElements > limits;
    int pixelsByDivision;
}
Divisions;

typedef struct Point2D_INT
{
    int X;
    int Y;
    Point2D_INT (int pX, int pY) : X(pX), Y(pY) {};
}
Point2D_INT;

/* --------------------------------------------------------------------- */
/* Functions                                                             */

void getDivisionLimits(IplImage *image, Divisions *imageDivisions) {
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

int getDivisionNumber(Divisions *imageDivisions, Point2D_INT *pixelPosition) {
    for (DivisionElements divElem : imageDivisions->limits) {
        if (isInDivision(pixelPosition, &divElem)) {
            return divElem.divisionNumber;
        }
    }
    return 0;
}

void populateHistogram(vector<vector<vector<float>>> *histogram,
                       Divisions *imageDivisions,
                       CvScalar *sc,
                       Point2D_INT *pixelPosition)
{
    int divisionNumber = getDivisionNumber(imageDivisions, pixelPosition);
    for (int c = 0; c < 3; c++) {
        histogram->at(divisionNumber).at(c).at((int) sc->val[c])++;
    }
}

void saveFile(vector<vector<vector<float>>> *file, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << "image_file " << *image << ".jpg" << endl;
    
    int divisionNumber = 0;
    int colorNumber = 0;
    
    for (vector<vector<float>> divisionVector : *file) {
        outputFile << "division " << divisionNumber << endl;
        divisionNumber++;
        for (vector<float> colorVector : divisionVector) {
            outputFile << "color "<< colorNumber << endl;
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

void removeOldFiles(vector<string> *fileNames) {
    ifstream outputFile;
    for (string fileName : *fileNames) {
        outputFile.open(fileName);
        if (outputFile.is_open()) {
            outputFile.close();
            remove(fileName.c_str());
        }
    }
}

void clearHistogram(vector<vector<vector<float>>> *histogram) {
    for (vector<vector<float>> divisionVector : *histogram) {
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

void clearHistogramLBPVector(vector<vector<float>> *histogramLBP) {
    for (vector<float> divisionVector : *histogramLBP) {
        divisionVector.clear();
    }
}

void normalizeHistogram(vector<vector<vector<float>>> *histogram, Divisions *imageDivisions) {
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

void buildColorPercentile(vector<vector<vector<float>>> *histogram, vector<vector<vector<float>>> *percentile, IplImage *currentImage, Divisions *imageDivisions, CvScalar *sc) {
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

void populateLBPHistogram(vector<vector<float>> *histogramLBP, Divisions *imageDivisions, Point2D_INT *pixelPosition, unsigned char *code) {
    int divisionNumber = getDivisionNumber(imageDivisions, pixelPosition);
    histogramLBP->at(divisionNumber).at(*code)++;
}

void normalizeLBPHistogram(vector<vector<float>> *histogramLBP, Divisions *imageDivisions) {
    int i = 0, j = 0;
    for (vector<float> divisionVector : *histogramLBP) {
        for (float value : divisionVector) {
            histogramLBP->at(i).at(j) = value/imageDivisions->pixelsByDivision;
            j++;
        }
        j = 0;
        i++;
    }
}

void buildLBPHistogram(vector<vector<float>> *histogramLBP, IplImage *currentImageGrayscale, Divisions *imageDivisions, CvScalar *center) {
    IplImage *destinationImage = cvCreateImage(cvGetSize(currentImageGrayscale), IPL_DEPTH_8U, 1);
    getDivisionLimits(currentImageGrayscale, imageDivisions);
    for(int x = 0; x < currentImageGrayscale->width; x++) {
        for(int y = 0; y < currentImageGrayscale->height; y++) {
            *center = cvGet2D(currentImageGrayscale, x, y);
            unsigned char code = 0;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x - 1, y - 1).val[0]) code += 128;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x - 1, y)    .val[0]) code += 64;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x - 1, y + 1).val[0]) code += 32;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x, y + 1)    .val[0]) code += 16;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x + 1, y + 1).val[0]) code += 8;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x + 1, y)    .val[0]) code += 4;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x + 1, y - 1).val[0]) code += 2;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, x, y - 1)    .val[0]) code += 1;
            center->val[0] = code;
            cvSet2D(destinationImage, x, y, *center);
            Point2D_INT pixelPosition(x, y);
            populateLBPHistogram(histogramLBP, imageDivisions, &pixelPosition, &code);
        }
    }
    normalizeLBPHistogram(histogramLBP, imageDivisions);
}

IplImage* convertToGrayscale(IplImage *currentImage) {
    IplImage *currentImageGrayscale = cvCreateImage(cvGetSize(currentImage), IPL_DEPTH_8U, 1);
    cvCvtColor(currentImage, currentImageGrayscale, CV_RGB2GRAY);
    return currentImageGrayscale;
}

void saveLBPFile(vector<vector<float>> *histogramLBP, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << "image_file " << *image << ".jpg" << endl;
    
    int divisionNumber = 0;
    
    for (vector<float> divisionVector : *histogramLBP) {
        outputFile << "division " << divisionNumber << endl;
        divisionNumber++;
        for (float value : divisionVector) {
            outputFile << value << "";
            
        }
        outputFile << endl;
    }
}


int main(int argc, char const *argv[]) {
    
    vector<vector<vector<float>>> histogram(numberOfDivisions, vector<vector<float>>(3, vector<float>(256)));
    vector<vector<vector<float>>> percentile(numberOfDivisions, vector<vector<float>>(3, vector<float>(11)));
    vector<vector<float>> histogramLBP(numberOfDivisions, vector<float>(256));
    vector<float> colorFeatureVector;
    
    Divisions imageDivisions;
    IplImage *currentImage;
    IplImage *currentImageGrayscale;
    CvScalar sc;
    string imagePath;
    ostringstream oss;
    string imageFolderPath = "/Users/lucas/Documents/Fall 2016/opencv_images/";
    string imageExtension = "jpg";
    
    vector<string> fileNames {"normalizedHistogram.txt", "normalizedPercentile.txt", "colorFeatureVector.txt", "histogramLBP.txt"};
    ofstream outputFileHistogram, outputFilePercentile, outputFileFeatures, outputFileHistogramLBP;
    removeOldFiles(&fileNames);
    outputFileHistogram.open(fileNames.at(0), ios::app);
    outputFilePercentile.open(fileNames.at(1), ios::app);
    outputFileFeatures.open(fileNames.at(2), ios::app);
    outputFileHistogramLBP.open(fileNames.at(3), ios::app);
    
    for (int image = 0; image < numberOfImages; image++) {
        oss << imageFolderPath << image << "." << imageExtension;
        imagePath = oss.str();
        currentImage = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_COLOR);
        
        //buildColorPercentile(&histogram, &percentile, currentImage, &imageDivisions, &sc);
        //saveFile(&histogram, outputFileHistogram, &image);
        //saveFile(&percentile, outputFilePercentile, &image);

// test LBP function
        currentImageGrayscale = convertToGrayscale(currentImage);
        buildLBPHistogram(&histogramLBP, currentImageGrayscale, &imageDivisions, &sc);
        saveLBPFile(&histogramLBP, outputFileHistogramLBP, &image);
// save LBP to file
        
        //buildFeatureVector(&percentile, &colorFeatureVector);
        //saveFeaturesVectorToFile(&colorFeatureVector, outputFileFeatures, &image);
// append LBP information to feature vector
        
        //clearHistogram(&histogram);
        //clearPercentile(&percentile);
        clearHistogramLBPVector(&histogramLBP);
        //clearColorFeatureVector(&colorFeatureVector);
        
        oss.str(string());
    }
    
    outputFileHistogram.close();
    outputFilePercentile.close();
    outputFileHistogramLBP.close();
    outputFileFeatures.close();
    
    return 0;
}















