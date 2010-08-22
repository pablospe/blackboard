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
vector<Point> contour_d;
vector<Point> contour_e;

vector<Point> current_contour;

void on_mouse( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON) ) {
        if( prev_pt.x > 0 ) {
            printf( "End!\n");

            float a, b, c, d, e;

            a = matchShapes( Mat(contour_a), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
            b = matchShapes( Mat(contour_b), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
            c = matchShapes( Mat(contour_c), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
            d = matchShapes( Mat(contour_d), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
            e = matchShapes( Mat(contour_e), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);

            map< float, string > result;  // sorting the result
            result[ a ] = "a";
            result[ b ] = "b";
            result[ c ] = "c";
            result[ d ] = "d";
            result[ e ] = "e";

            cout << "matchShapes(a,.) = " << a << endl;
            cout << "matchShapes(b,.) = " << b << endl;
            cout << "matchShapes(c,.) = " << c << endl;
            cout << "matchShapes(d,.) = " << d << endl;
            cout << "matchShapes(e,.) = " << e << endl;

            cout << "Letra " << result.begin()->second << "!!!!!\n";
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

        printf( "contour.push_back(Point2f(%d,%d));\n", x, y );
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
    #include "train/contour_d.cpp"
    #include "train/contour_e.cpp"


    printf( "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - restore the blackboard\n" );

    namedWindow( "image" );
    cvSetMouseCallback( "image", on_mouse, 0 );

    /* create an image */
    IplImage *img0 = cvCreateImage(cvSize(400, 400), IPL_DEPTH_8U, 3);

    img = cvCloneImage( img0 );
    imshow( "image", img );

    for(;;)
    {
        int c = waitKey(0);

        if( (char) c == 27 )
            break;

        if( (char) c == 'r' )
        {
            cvCopy( img0, img );
            cvShowImage( "image", img );
        }
    }


    return 1;
}
