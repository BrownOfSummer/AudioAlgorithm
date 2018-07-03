/*************************************************************************
    > File Name: selectChessBoardImgs.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-07-02 16:28:47
 ************************************************************************/

#include<iostream>
#include<vector>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include<opencv2/opencv.hpp>

#define IN_CORNER_WIDTH 7
#define IN_CORNER_HEIGHT 5
#define SKIP_FRAME_NUM 10
#define MAX_WIDTH_FOR_SHOW 640
using namespace std;
using namespace cv;
vector<string> listDir( const char* path, const char *ext );
void help()
{
    printf("Demonstrate the process of chessboard image selection.\n");
    printf("Input a dir contain a list of images or a video.\n");
    printf("ESC to break, s for select, other for next.\n");
    printf("Usage:\n\t ./selectChessBoardImgs -I ../path/to/imageDir/ /path/to/selected.txt\n\t ./selectChessBoardImgs -V ../path/to/video.mp4 ../dir/to_save_images/\n");
    return;
}
int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        printf("ERROR INPUT\n");
        help();
        return -1;
    }
    bool isVideo = false;
    if( !strcmp(argv[1], "-V") )
    {
        printf("Video mode !\n");
        isVideo = true;
    }
    else if( !strcmp(argv[1], "-I") )
    {
        printf("Image list mode.\n");
        isVideo = false;
    }
    else
    {
        help();
        printf("INVALID MODE, Should be -V or -I\n");
        return -1;
    }
    Size boardSize;
    boardSize.width = IN_CORNER_WIDTH;
    boardSize.height = IN_CORNER_HEIGHT;
    if( !isVideo )
    {
        const char *imageDir = argv[2];
        vector<string> imageNames = listDir( imageDir, ".jpg");
        std::sort(imageNames.begin(), imageNames.end());
        printf("Totall get %d image file.\n", static_cast<int>(imageNames.size()));
        
        vector<string> selectedFilePath;
        for(size_t idx = 0; idx < imageNames.size();)
        {
            string imagePath = string( imageDir ) + imageNames[idx]; // imageDir like ../path/
            Mat view = imread(imagePath, 1);
            if( view.empty() ){
                printf("Load image %s Failed.\n", imagePath.c_str());
                break;
            }
            vector<Point2f> pointBuf;
            bool found = false;
            int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;
            found = findChessboardCorners( view, boardSize, pointBuf, chessBoardFlags);
            if( found )
            {
                Mat viewGray;
                cvtColor(view, viewGray, COLOR_BGR2GRAY);
                cornerSubPix( viewGray, pointBuf, Size(11,11), Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));
                drawChessboardCorners( view, boardSize, Mat(pointBuf), found );
            }

            if(view.cols > MAX_WIDTH_FOR_SHOW) {
                double ratio = MAX_WIDTH_FOR_SHOW / (double)(view.cols);
                resize(view, view, Size(), ratio, ratio);
            }
            
            /*If found, show and save*/
            if( found )
            {
                imshow("s-select,ESC-break,p->prev-image,other-next", view);
                char k = waitKey();
                if(k == 's') {
                    char *imageAbsPath = realpath(imagePath.c_str(), NULL);
                    printf("Selected %s\n", imageAbsPath);
                    selectedFilePath.push_back(imageAbsPath);
                }
                else if(k == 27) break;
                else if(k == 'p') idx = idx - 1 > 0 ? idx - 1 : 0;
                else idx ++;
            }
            else
            {
                idx ++;
                printf("No found in %s\n", imageNames[idx].c_str());
                imshow("s-select,ESC-break,p->prev-image,other-next", view);
                waitKey(30);
                continue;
            }
        }

        /*save abspath to selected.txt*/
        if( selectedFilePath.size() > 0 )
        {
            printf("Save the paths to %s\n", argv[3]);
            FILE *fp = fopen(argv[3],"w");
            if( fp == NULL ) {
                printf("Open %s failed.\n", argv[3]);
                return -1;
            }
            for( size_t i = 0; i < selectedFilePath.size(); i ++ )
                fprintf(fp,"%s\n",selectedFilePath[i].c_str());
            fclose(fp);
        }
    }
    else
    {
        vector<Mat> selectedImages;
        VideoCapture cap(argv[2], CAP_FFMPEG);
        if( !cap.isOpened() ) {
            cout<<"ERROR: Open video failed !\n";
            return -1;
        }
        Mat frame, gray, view;
        int cntFrame = 0;
        for(;;)
        {
            if(cntFrame < SKIP_FRAME_NUM)
            {
                cntFrame ++;
                cap.grab();
                continue;
            }
            cntFrame = 0;
            cap.grab();
            cap.retrieve(frame);
            if( frame.empty() ) break;
            
            frame.copyTo( view );
            vector<Point2f> pointBuf;
            bool found = false;
            int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;
            found = findChessboardCorners( view, boardSize, pointBuf, chessBoardFlags);
            if( found )
            {
                cvtColor(view, gray, COLOR_BGR2GRAY);
                cornerSubPix( gray, pointBuf, Size(11,11), Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));
                drawChessboardCorners( view, boardSize, Mat(pointBuf), found );
            }
            if(view.cols > MAX_WIDTH_FOR_SHOW)
            {
                double ratio = MAX_WIDTH_FOR_SHOW / (double)(view.cols);
                resize(view, view, Size(), ratio, ratio);
            }
            if( found )
            {
                imshow("s-select,ESC-break,other-next", view);
                char k = waitKey();
                if(k == 's') selectedImages.push_back(frame.clone());
                else if(k == 27) break;
                else continue;
            }
            else
            {
                imshow("s-select,ESC-break,other-next", view);
                char k = waitKey(30);
                if(k == 27) break;
            }
        }
        /*save the select image to image path*/
        char save_path[200];
        if( selectedImages.size() > 0 )
        {
            for(size_t i = 0; i < selectedImages.size(); i ++) {
                sprintf(save_path,"%sselected_%04d.jpg",argv[3], (int)(i));
                //imshow("selected", selectedImages[i]);
                printf("save to %s\n", save_path);
                imwrite(save_path, selectedImages[i]);
                //char k = waitKey();
                //if( k == 27 ) break;
            }
        }
    }
    return 0;
}
vector<string> listDir( const char* path, const char *ext )
{
    DIR* dirFile = opendir( path );
    vector<string> ret;
    if ( dirFile ) 
    {
        struct dirent* hFile;
        errno = 0;
        while (( hFile = readdir( dirFile )) != NULL ) 
        {
            // skip . and ..
            if ( !strcmp( hFile->d_name, "."  )) continue;
            if ( !strcmp( hFile->d_name, ".." )) continue;

            // in linux hidden files all start with '.', skip it
            if (hFile->d_name[0] == '.' ) continue;

            // dirFile.name is the name of the file. Do whatever string comparison 
            // you want here. Something like:
            if(ext != NULL)
            {
                if ( strstr( hFile->d_name, ext ))
                    ret.push_back( hFile->d_name );
            }
            else
                ret.push_back( hFile->d_name );
      } 
      closedir( dirFile );
   }
    return ret;
}

void getAbsPath(const char *dirpath, const char* file)
{
    //
}
