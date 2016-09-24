//
//  main.cpp
//  OpenCVTest
//
//  Created by Lucas Alves Sobrinho on 8/26/16.
//  Copyright © 2016 Lucas Alves Sobrinho. All rights reserved.
//

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <sstream>


void initHistVector(float hist[][3][256]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 256; k++) {
                hist[i][j][k] = 0;
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    
    argc = 2;
    argv[2] = "/Users/lucas/Documents/Fall 2016/opencv_images/30.jpg";
    
    if(argc == 1) {
        float hist[4][3][256];//, perc[4][3][10];
        char img_path[1000];
        IplImage *dst;
        CvScalar v;
        int n_images = 1000;
        for (int img = 0; img < n_images; img++) {
            
            sprintf(img_path, "/Users/lucas/Documents/Fall 2016/opencv_images/%d.jpg", img);
            
            dst = cvLoadImage(img_path, CV_LOAD_IMAGE_COLOR);
            initHistVector(hist);
            for (int i = 0; i < dst->height; i++)
                for (int j = 0; j < dst->width; j++) {
                    v = cvGet2D(dst, i, j);
                    if (i <= dst->height / 2 && j <= dst->width / 2) {
                        hist[0][0][(int) v.val[0]]++;
                        hist[0][1][(int) v.val[1]]++;
                        hist[0][2][(int) v.val[2]]++;
                        
                    } else if (i > dst->height / 2 && j <= dst->width / 2) {
                        hist[1][0][(int) v.val[0]]++;
                        hist[1][1][(int) v.val[1]]++;
                        hist[1][2][(int) v.val[2]]++;
                    } else if (i <= dst->height / 2 && j > dst->width / 2) {
                        hist[2][0][(int) v.val[0]]++;
                        hist[2][1][(int) v.val[1]]++;
                        hist[2][2][(int) v.val[2]]++;
                    } else {
                        hist[3][0][(int) v.val[0]]++;
                        hist[3][1][(int) v.val[1]]++;
                        hist[3][2][(int) v.val[2]]++;
                    }
                }
            
            
            FILE *file;
            file = fopen("/Users/lucas/Documents/Fall 2016/opencv_images/normalized_output.txt", "a");
            
            if (file == NULL) exit (-1);
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 3; j++) {
                    for (int k = 0; k < 256; k++) {
                        fprintf(file, "%f,", hist[i][j][k]/(((dst->height/2) + 1) * ((dst->width/2) + 1)));
                    }
                }
            }
            fprintf(file, "%s\n", img_path);
            fclose(file);
        }
    } else if (argc == 2){
        printf("%s", argv[2]);
    }
    
    return 0;
}















