#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
#include <io.h>

using namespace cv;
using namespace std;

Mat src;
Mat src_gray;
Mat roi,rom,roi_grey,roi2;
int thresh = 60;
int max_thresh = 255;
RNG rng(12345);
_finddata_t danePliku;
int licz = 0;


///*****************************************FUNCJONS*****************************

template <typename T>  string NumberToString ( T Number ) //  int to string
{
    ostringstream ss;
    ss << Number;
    return ss.str();
}


///**************************************** MAIN **********************************************************************************
int main( int argc, char** argv )
{

///make Folder
    string folderCreateCommand = "mkdir br";
    system(folderCreateCommand.c_str());
    folderCreateCommand = "mkdir gr";
    system(folderCreateCommand.c_str());
    folderCreateCommand = "mkdir tmp";
    system(folderCreateCommand.c_str());

///*** BANCH MODE - Find files and processing
    long uchwyt = _findfirst( "*.png", & danePliku );
    if( (uchwyt = _findfirst( "*.png", &danePliku )) == -1L )
        cout<< "No file with good extention\n" ;
    else
    {
        do
        {
            cout<< danePliku.name<< "\n";
            src = imread(danePliku.name);
            if ( !src.data )
            {
                cout << "No File! \n. ";
                return -1;
            }

            /// Convert image to gray and blur it
            cvtColor( src, src_gray, CV_BGR2GRAY );
            blur( src_gray, src_gray, Size(3,3) );

            Mat threshold_output;
            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;

            /// Detect edges using Threshold
            threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );

            //morphological opening (remove small objects from the foreground)
            erode(threshold_output, threshold_output, getStructuringElement(MORPH_OPEN, Size(7, 7)) );
            dilate( threshold_output, threshold_output, getStructuringElement(MORPH_OPEN, Size(7, 7)) );

            //morphological closing (fill small holes in the foreground)
            dilate( threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)) );
            erode(threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)) );

            /*namedWindow("morfologia",CV_WINDOW_AUTOSIZE);
            imshow("morfologia",threshold_output);*/

            /// Find contours
            findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

            /// Find the rotated rectangles and ellipses for each contour
            vector<RotatedRect> minRect( contours.size() );
            vector<RotatedRect> minEllipse( contours.size() );

            for( int i = 0; i < contours.size(); i++ )
            {
                minRect[i] = minAreaRect( Mat(contours[i]) );
                if( contours[i].size() > 75 )
                {
                    minEllipse[i] = fitEllipse( Mat(contours[i]) );
                }
            }

            /// Draw contours + rotated rects + ellipses
            Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
            for( int i = 0; i< contours.size(); i++ )
            {
                if(contourArea( contours[i],false) > 600)
                {
                    roi =src( boundingRect(contours[i])); // set ROI
                    copyMakeBorder( roi, roi, 50, 50, 50, 50,BORDER_CONSTANT, 0 );


                    /*if(minRect[i].size.width > minRect[i].size.height)
                         int maxsize = minRect[i].size.width;
                     else
                         int maxsize = minRect[i].size.height;
                    */

                    //float angle = minEllipse[i].angle;

                    rom = getRotationMatrix2D(Point( roi.cols/2., roi.rows/2. ), minEllipse[i].angle, 1.0 );
                    //rom = getRotationMatrix2D(minEllipse[i].center, angle, 1.0); // get the rotation matrix
                    /*if(minRect[i].size.width > minRect[i].size.height){
                        swap(minRect[i].size.width,minRect[i].size.height);
                    }*/

                    warpAffine(roi, roi, rom, roi.size(), cv::INTER_CUBIC);  // perform the affine transformation

//Find the biggest countours ****
                    /// Convert image to gray and blur it
                    cvtColor( roi, roi_grey, CV_BGR2GRAY );
                    blur( roi_grey, roi_grey, Size(3,3) );

                    Mat threshold_output2;
                    vector<vector<Point> > contours2;
                    vector<Vec4i> hierarchy2;

                    /// Detect edges using Threshold
                    threshold( roi_grey, threshold_output2, thresh, 255, THRESH_BINARY );
                    /// Find contours
                    findContours( threshold_output2, contours2, hierarchy2, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
                    int bigest=0;
                    int which_bigest=0;

                    for( int j = 0; j< contours2.size(); j++ )
                    {
                        if(contourArea( contours2[j],false) > bigest)
                        {
                            bigest = contourArea( contours2[j],false);
                            which_bigest = j;
                            roi2 = roi( boundingRect(contours2[j])); // set ROI 2
                        }

                    }
/// Get the moments
                    vector<Moments> mu(contours2.size() );

                    mu[which_bigest] = moments( contours2[which_bigest], false );

///  Get the mass centers:
                    vector<Point2f> mc( contours2.size() );
                    mc[which_bigest] = Point2f( mu[which_bigest].m10/mu[which_bigest].m00, mu[which_bigest].m01/mu[which_bigest].m00 );
                    circle( roi, mc[which_bigest], 4, Scalar( 255, 255, 255 ), -1, 8, 0 );
                    //imshow( "CENTER "+NumberToString(which_bigest),  roi);

/// Detection UP / DOWN
                    cvtColor( roi2, roi_grey, CV_BGR2GRAY );
                    blur( roi_grey, roi_grey, Size(3,3) );

                    threshold( roi_grey, threshold_output2, thresh, 255, THRESH_BINARY );

                    int topsize = 0;
                    int downsize = 0;
                    for( int y = 0; y < threshold_output2.rows/4; y++ )
                    {
                        for( int x = 0; x < threshold_output2.cols; x++ )
                        {
                            if ( threshold_output2.at<uchar>(y,x) == 255 ) topsize++;
                        }
                    }

                    for( int y = threshold_output2.rows - (threshold_output2.rows/4); y < threshold_output2.rows; y++ )
                    {
                        for( int x = 0; x < threshold_output2.cols; x++ )
                        {
                            if ( threshold_output2.at<uchar>(y,x) == 255 ) downsize++;
                        }
                    }

                    if (downsize<topsize)

                    {
                        flip(roi2,roi2,0); // FLIP if needed
                        flip(roi_grey,roi_grey,0);
                    }

///Valey detect

                    /// Gradient X
                    Mat roi_grey_sob;
                    //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
                    equalizeHist( roi_grey, roi_grey );
                    Sobel( roi_grey, roi_grey_sob, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT );
                    convertScaleAbs( roi_grey_sob, roi_grey_sob );
                    ///ROI AREA
                    int roi_top_down=10, roi_left_right = 45;
                    threshold( roi_grey_sob, roi_grey_sob, 75, 255, THRESH_BINARY );

                    int valeysize = 0;

                    for( int y = roi_grey_sob.rows/2-roi_left_right; y < roi_grey_sob.rows/2+roi_left_right+20; y++ )
                    {
                        for( int x = roi_grey_sob.cols/2-roi_top_down; x < roi_grey_sob.cols/2+roi_top_down; x++ )
                        {
                            if ( roi_grey_sob.at<uchar>(y,x) == 255 ) valeysize++;
                        }
                    }

                    //putText(roi_grey_sob, NumberToString(valeysize), Point(0, 10),FONT_HERSHEY_DUPLEX, 0.4, cvScalar(255,255,255), 1, CV_AA);
                    rectangle( roi_grey_sob,
                               Point( roi_grey_sob.cols/2-roi_top_down,
                                      roi_grey_sob.rows/2-roi_left_right+20 ),
                               Point( roi_grey_sob.cols/2+roi_top_down,
                                      roi_grey_sob.rows/2+roi_left_right),
                               Scalar( 255, 255, 255 ), 2, 8 );
                    //imshow( "ROI TMP"+NumberToString(i),  roi_grey_sob );


///***Valey end


///Rectangle visualization
                    //int widthimg = threshold_output3.cols;
                    //int heightimg = threshold_output3.rows;
                    //int topsize = countNonZero(threshold_output3(Rect(0, 0, widthimg, heightimg/4)));
                    //int downsize = countNonZero(threshold_output3(Rect(0, heightimg - (heightimg/4), widthimg, heightimg/4)));

                    //rectangle( roi2, Point( 0, 0 ), Point( widthimg, heightimg/4),Scalar( 0, 255, 255 ), 2, 8 );
                    //rectangle( roi2, Point( 0, heightimg - (heightimg/4 )), Point( widthimg, heightimg),Scalar( 0, 0, 255 ), 2, 8 );


///Normalize Resize
//width //height
                    int diffrent_size_w, diffrent_size_h;
                    int tmp_flag = 0;

                    if (roi2.cols <= 80 && roi2.rows <= 170)
                    {
                        diffrent_size_w = 80-roi2.cols;
                        diffrent_size_h = 170-roi2.rows;
                        if (diffrent_size_w%2 == 0 && diffrent_size_h%2 == 0)
                            copyMakeBorder( roi2, roi2, diffrent_size_h/2,diffrent_size_h/2, diffrent_size_w/2, diffrent_size_w/2,BORDER_CONSTANT,0);
                        else if (diffrent_size_w%2 != 0 && diffrent_size_h%2 != 0)
                            copyMakeBorder( roi2, roi2, diffrent_size_h/2+1,diffrent_size_h/2, diffrent_size_w/2 + 1, diffrent_size_w/2,BORDER_CONSTANT,0);
                        else if (diffrent_size_w%2 != 0 && diffrent_size_h%2 == 0)
                            copyMakeBorder( roi2, roi2, diffrent_size_h/2,diffrent_size_h/2, diffrent_size_w/2 + 1, diffrent_size_w/2,BORDER_CONSTANT,0);
                        else if (diffrent_size_w%2 == 0 && diffrent_size_h%2 != 0)
                            copyMakeBorder( roi2, roi2, diffrent_size_h/2+1,diffrent_size_h/2, diffrent_size_w/2, diffrent_size_w/2,BORDER_CONSTANT,0);
                    }
                    else if (roi2.cols > 80 || roi2.rows > 170) tmp_flag = 1;

///WRITE and SORT  DATA
                    //putText(roi2, NumberToString(valeysize), Point(0, 10),FONT_HERSHEY_DUPLEX, 0.4, cvScalar(255,255,255), 1, CV_AA);
                    string iplik =
                        NumberToString ( valeysize )+"_"+NumberToString ( i );
                    //if(valeysize/roi2.cols > 3) imwrite("br" + iplik + danePliku.name, roi2 );
                    if (tmp_flag ==1)
                    {
                        imwrite("tmp/" + iplik + danePliku.name, roi2 );                // 60 - good
                        //imwrite("br/" + iplik + "1" + danePliku.name, roi_grey_sob );  // B&W
                    }
                    else if(valeysize > 420)
                    {
                        imwrite("br/" + iplik + danePliku.name, roi2 );                // 60 - good
                        //imwrite("br/" + iplik + "1" + danePliku.name, roi_grey_sob );  // B&W
                    }
                    else
                    {
                        imwrite("gr/" + iplik + danePliku.name, roi2 );
                        //imwrite("gr/" + iplik + "1" + danePliku.name, roi_grey_sob ); //B&W
                    }
                    //namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
                    //imshow( "Contours", drawing );
                    //licz++;
                    // cout << "zapisano "<<licz<< " plikow\n";

///Visualisation countour
                    /*
                    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
                    // contour
                    drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
                    // ellipse
                    //minEllipse[i].angle = 0; // angle test
                    ellipse( drawing, minEllipse[i], color, 2, 8 );
                    // rotated rectangle
                    /*Point2f rect_points[4]; minRect[i].points( rect_points );
                    for( int j = 0; j < 4; j++ )
                     line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
                    //txt
                    putText(drawing, NumberToString(topsize), Point(minEllipse[i].center.x + minEllipse[i].size.width/4 +20, minEllipse[i].center.y + minEllipse[i].size.height/2-100),FONT_HERSHEY_DUPLEX, 0.4, cvScalar(0,0,200), 1, CV_AA);
                    putText(drawing, NumberToString(downsize), Point(minEllipse[i].center.x + minEllipse[i].size.width/4 +20, minEllipse[i].center.y + minEllipse[i].size.height/2-50),FONT_HERSHEY_DUPLEX, 0.4, cvScalar(0,250,0), 1, CV_AA);
                    */
///Open small windows with roi obiects
                    //namedWindow( "roi_"+NumberToString(i), CV_WINDOW_AUTOSIZE );
                    //imshow( "roi_"+NumberToString(i), roi2 );

                    //putText(drawing, NumberToString(minEllipse[i].angle), Point(minEllipse[i].center.x + minEllipse[i].size.width/4,minEllipse[i].center.y + minEllipse[i].size.height/2),FONT_HERSHEY_DUPLEX, 0.4, cvScalar(150,150,150), 1, CV_AA);
                }


            }

            /// Show in a window
            //namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
            //imshow( "Contours", drawing );
            waitKey(0);

        }

        while( _findnext( uchwyt, &danePliku ) == 0 );
        _findclose( uchwyt );
    }
}






