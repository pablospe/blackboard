#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace cv;
using namespace std;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

vector<Point> contour_a;
vector<Point> contour_b;
vector<Point> contour_c;

vector<Point> current_contour;

void on_mouse( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON) ) {
        if( prev_pt.x > 0 ) {
            printf( "End!\n");

            float a, b, c;

            a = matchShapes( Mat(contour_a), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
            b = matchShapes( Mat(contour_b), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
            c = matchShapes( Mat(contour_c), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);

            cout << "matchShapes(a,.) = " << a << endl;
            cout << "matchShapes(b,.) = " << b << endl;
            cout << "matchShapes(c,.) = " << c << endl;

            if( a < b ) {
                if( a < c )
                    cout << "Letra a!!!!!\n";
                else
                    cout << "Letra c!!!!!\n";
            }
            else {
                if( b < c )
                    cout << "Letra b!!!!!\n";
                else
                    cout << "Letra c!!!!!\n";
            }
        }
        prev_pt = cvPoint(-1,-1);
    }
    else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) )
    {
        CvPoint pt = cvPoint(x,y);

        if( prev_pt.x < 0 ) {
            prev_pt = pt;
            printf( "Start moving!\n");
            current_contour.clear();
        }
        else {
            cvLine( img, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
        }

        printf( "Point: (%d,%d)\n", x, y );
        current_contour.push_back(Point2f(x,y));

        prev_pt = pt;
        imshow( "image", img );
    }
}


int main( int argc, char** argv )
{
    #include "train/contour_a.cpp"
    #include "train/contour_b.cpp"
    #include "train/contour_c.cpp"


    printf( "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - restore the original image\n" );

    namedWindow( "image" );
    cvSetMouseCallback( "image", on_mouse, 0 );

    /* create an image */
    IplImage *img0 = cvCreateImage(cvSize(400, 400), IPL_DEPTH_8U, 3);

    img = cvCloneImage( img0 );
    imshow( "image", img );

    for(;;)
    {
        int c = waitKey(0);

        if( (char)c == 27 )
            break;

        if( (char)c == 'r' )
        {
            cvCopy( img0, img );
            cvShowImage( "image", img );
        }
    }


    return 1;
}
