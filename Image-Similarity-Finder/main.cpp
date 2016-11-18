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
#include <unordered_map>
#include <map>
#include <sstream>

using namespace std;

/* --------------------------------------------------------------------- */
/* Enumerations                                                          */

enum DivisionType {
    TWO_BY_TWO      = 4,
    THREE_BY_THREE  = 9,
    FOUR_BY_FOUR    = 16,
    FIVE_BY_FIVE    = 25
};

enum SearchMode {
    COLOR_HISTOGRAM         = 0,
    LOCAL_BINARY_PATTERN    = 1,
    ALL_MODES               = 2
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

void saveFeaturesFile(vector<float> *features, vector<vector<float>> *histogramLBP, ofstream& outputFile, int *image) {
    outputFile << *image << ".jpg" << endl;
    
    outputFile << fixed;
    outputFile.precision(10);
    
    for (float value : *features) {
        outputFile << value << " ";
    }
    
    outputFile << endl;
    
    for (vector<float> divisionVector : *histogramLBP) {
        for (float value : divisionVector) {
            outputFile << value << " ";
        }
    }
    
    outputFile << endl;
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
    int i, j, k;
    for (i = 0; i < histogram->size(); i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 256; k++) {
                histogram->at(i).at(j).at(k) = 0;
            }
        }
    }
}

void clearPercentile(vector<vector<vector<float>>> *percentile) {
    int i, j, k;
    i = 0;
    for (vector<vector<float> > divisionVector : *percentile) {
        j = 0;
        for (vector<float> colorVector : divisionVector) {
            k = 0;
            for (float v : colorVector) {
                percentile->at(i).at(j).at(k) = 0;
                k++;
            }
            j++;
        }
        i++;
    }
}

void clearColorFeatureVector(vector<float> *colorFeatureVector) {
    colorFeatureVector->clear();
}

void clearHistogramLBPVector(vector<vector<float>> *histogramLBP) {
    int i, j;
    i = 0;
    for (vector<float> divisionVector : *histogramLBP) {
        j = 0;
        for (float v : divisionVector) {
            histogramLBP->at(i).at(j) = 0;
            j++;
        }
        i++;
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

void clearImageDivisions(Divisions *imageDivisions) {
    imageDivisions->limits.clear();
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
    clearImageDivisions(imageDivisions);
}

void buildColorFeatureVector(vector<vector<vector<float>>> *percentile, vector<float> *colorFeatureVector) {
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

void initializeDirectionInts(int *left, int *right, int *up, int *down, int x, int y, IplImage *currentImageGrayscale) {
    if (x == 0) {
        *left = x;
        *right = x + 1;
    } else if (x == currentImageGrayscale->width - 1) {
        *left = x - 1;
        *right = x;
    } else {
        *left = x - 1;
        *right = x + 1;
    }
    if (y == 0) {
        *up = y;
        *down = y + 1;
    } else if (y == currentImageGrayscale->height - 1) {
        *up = y - 1;
        *down = y;
    } else {
        *up = y - 1;
        *down = y + 1;
    }
}

void buildLBPHistogram(vector<vector<float>> *histogramLBP, IplImage *currentImageGrayscale, Divisions *imageDivisions, CvScalar *center) {
    IplImage *destinationImage = cvCreateImage(cvGetSize(currentImageGrayscale), IPL_DEPTH_8U, 1);
    getDivisionLimits(currentImageGrayscale, imageDivisions);
    int left, right, up, down;
    for(int x = 0; x < currentImageGrayscale->width; x++) {
        for(int y = 0; y < currentImageGrayscale->height; y++) {
            initializeDirectionInts(&left, &right, &up, &down, x, y, currentImageGrayscale);
            *center = cvGet2D(currentImageGrayscale, y, x);
            unsigned char code = 0;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, up, left)   .val[0]) code += 128;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, y, left)    .val[0]) code += 64;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, down, left) .val[0]) code += 32;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, down, x)    .val[0]) code += 16;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, down, right).val[0]) code += 8;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, y, right)   .val[0]) code += 4;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, up, right)  .val[0]) code += 2;
            if(center->val[0] <= cvGet2D(currentImageGrayscale, up, x)      .val[0]) code += 1;
            center->val[0] = code;
            cvSet2D(destinationImage, y, x, *center);
            Point2D_INT pixelPosition(x, y);
            populateLBPHistogram(histogramLBP, imageDivisions, &pixelPosition, &code);
        }
    }
    normalizeLBPHistogram(histogramLBP, imageDivisions);
    clearImageDivisions(imageDivisions);
}

IplImage* convertToGrayscale(IplImage *currentImage) {
    IplImage *currentImageGrayscale = cvCreateImage(cvGetSize(currentImage), IPL_DEPTH_8U, 1);
    cvCvtColor(currentImage, currentImageGrayscale, CV_RGB2GRAY);
    return currentImageGrayscale;
}

void saveLBPFile(vector<vector<float>> *histogramLBP, ofstream& outputFile, int *image) {
    outputFile << endl;
    outputFile << *image << ".jpg" << endl;
    
    for (vector<float> divisionVector : *histogramLBP) {
        for (float value : divisionVector) {
            outputFile << value << " ";
        }
    }
}

vector<float> getFeatureValues(string *featuresLine) {
    vector<float> fileColorFeatureVector;
    istringstream iss(*featuresLine);
    float value;
    while (iss >> value){
        fileColorFeatureVector.push_back(value);
    }
    return fileColorFeatureVector;
}

void getColorFeatureVector(unordered_map<string, vector<float>> *imageNameToFeatures, ifstream& featuresFile) {
    vector<float> fileColorFeatureVector;
    string imageFileName, featuresLine, temp;
    
    while (getline(featuresFile, imageFileName)) {
        getline(featuresFile, featuresLine);
        fileColorFeatureVector = getFeatureValues(&featuresLine);
        (*imageNameToFeatures)[imageFileName] = fileColorFeatureVector;
        getline(featuresFile, temp);
    }
}

float computeEuclidianDistance(vector<float> *colorFeatureVector, vector<float> *mappedFeatureVector) {
    float sum = 0;
    for (int i = 0; i < colorFeatureVector->size(); i++) {
        sum += pow(colorFeatureVector->at(i) - mappedFeatureVector->at(i), 2);
    }
    return sqrt(sum);
}

void sortImagesByDifference(vector<pair<float, string>> *imageNameToDistance) {
    sort(imageNameToDistance->begin(), imageNameToDistance->end());
}

void computeDifference(vector<float> *colorFeatureVector, ifstream& featuresFile) {
    unordered_map<string, vector<float>> imageNameToFeatures;
    vector<pair<float, string>> imageNameToDistance;
    getColorFeatureVector(&imageNameToFeatures, featuresFile);
    for (auto& entry : imageNameToFeatures) {
        pair<float, string> entryPair;
        entryPair.second = entry.first;
        entryPair.first = computeEuclidianDistance(colorFeatureVector, &entry.second);
        imageNameToDistance.push_back(entryPair);
    }
    sortImagesByDifference(&imageNameToDistance);
}

void computeDifference(vector<vector<float>> *histogramLBP, ifstream& featuresFile) {
    // to do
}

void computeDifference(vector<float> *colorFeatureVector, vector<vector<float>> *histogramLBP, ifstream& featuresFile) {
    // to do
}


void findSimilarImages(char const *inputImagePath, SearchMode mode) {
    vector<vector<vector<float>>> histogram(numberOfDivisions, vector<vector<float>>(3, vector<float>(256)));
    vector<vector<vector<float>>> percentile(numberOfDivisions, vector<vector<float>>(3, vector<float>(11)));
    vector<float> colorFeatureVector;
    vector<vector<float>> histogramLBP(numberOfDivisions, vector<float>(256));
    
    IplImage *inputImage = cvLoadImage(inputImagePath, CV_LOAD_IMAGE_COLOR);
    IplImage *currentImageGrayscale = convertToGrayscale(inputImage);
    Divisions imageDivisions;
    CvScalar sc;
    
    string featuresFilePath = "./features.txt";
    ifstream featuresFile;
    featuresFile.open(featuresFilePath);
    if(!featuresFile.is_open()) {
        cerr << "Error opening file " << featuresFilePath << endl;
        // return 0 instead of just returning nothing at all
        return;
    }
    
    if (mode == COLOR_HISTOGRAM) {
        buildColorPercentile(&histogram, &percentile, inputImage, &imageDivisions, &sc);
        buildColorFeatureVector(&percentile, &colorFeatureVector);
        
        computeDifference(&colorFeatureVector, featuresFile);
    }
    if (mode == LOCAL_BINARY_PATTERN) {
        buildLBPHistogram(&histogramLBP, currentImageGrayscale, &imageDivisions, &sc);
        
        computeDifference(&histogramLBP, featuresFile);
    }
    if (mode == ALL_MODES) {
        buildColorPercentile(&histogram, &percentile, inputImage, &imageDivisions, &sc);
        buildColorFeatureVector(&percentile, &colorFeatureVector);
        buildLBPHistogram(&histogramLBP, currentImageGrayscale, &imageDivisions, &sc);
        
        computeDifference(&colorFeatureVector, &histogramLBP, featuresFile);
    }
    
    featuresFile.close();
    
}


int main(int argc, char const *argv[]) {
    
    // Build and save file for a set of images
    // Usage: --build path_to_images images_extension
    if (strcmp(argv[1], "--build") == 0) {
        if (argc == 4) {
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
            string imageFolderPath = argv[2];
            string imageExtension = argv[3];
            
            vector<string> fileNames {"DEBUG_normalizedHistogram.txt", "DEBUG_normalizedPercentile.txt", "features.txt", "DEBUG_histogramLBP.txt"};
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
                
                buildColorPercentile(&histogram, &percentile, currentImage, &imageDivisions, &sc);
                
                saveFile(&histogram, outputFileHistogram, &image);
                saveFile(&percentile, outputFilePercentile, &image);
                
                buildColorFeatureVector(&percentile, &colorFeatureVector);
                
                currentImageGrayscale = convertToGrayscale(currentImage);
                buildLBPHistogram(&histogramLBP, currentImageGrayscale, &imageDivisions, &sc);
                saveLBPFile(&histogramLBP, outputFileHistogramLBP, &image);
                
                saveFeaturesFile(&colorFeatureVector, &histogramLBP, outputFileFeatures, &image);
                
                clearHistogram(&histogram);
                clearPercentile(&percentile);
                clearHistogramLBPVector(&histogramLBP);
                clearColorFeatureVector(&colorFeatureVector);
                
                oss.str(string());
            }
            
            outputFileHistogram.close();
            outputFilePercentile.close();
            outputFileHistogramLBP.close();
            outputFileFeatures.close();
            
        } else {
            cerr << "Invalid arguments for --build. Usage: --build [path_to_images] [images_extension]" << endl;
            return 0;
        }
    } else {
        if (argc == 3) {
            
            // Compare a given image to a set of images using color histogram comparison
            // Usage: --color path_to_image
            if (strcmp(argv[1], "--color") == 0) {
                findSimilarImages(argv[2], COLOR_HISTOGRAM);
            }
        
            // Compare a given image to a set of images using Local Binary Pattern (LBP) method
            // Usage: --lbp path_to_image
            else if (strcmp(argv[1], "--lbp") == 0) {
                findSimilarImages(argv[2], LOCAL_BINARY_PATTERN);
            }
        
            // Compare a given image using both color histogram and LBP methods
            // Usage: --all path_to_image
            else if (strcmp(argv[1], "--all") == 0) {
                findSimilarImages(argv[2], ALL_MODES);
            } else {
                cerr << "Invalid argument option: " << argv[1] << ". Usage: [--color, --lbp, --all] [path_to_image]" << endl;
                return 0;
            }
        } else {
            cerr << "Invalid arguments for " << argv[1] << ". Usage: " << argv[1] << " [path_to_image]" << endl;
            return 0;
        }
    }
    
    return 1;
}















