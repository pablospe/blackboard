#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

void on_mouse( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON) ) {
        if( prev_pt.x > 0 ) {
            printf( "End!\n");
        }
        prev_pt = cvPoint(-1,-1);
    }
    else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) )
    {
        CvPoint pt = cvPoint(x,y);
        printf( "Point: (%d,%d)\n", x, y );

        if( prev_pt.x < 0 ) {
            prev_pt = pt;
            printf( "Start moving!\n");
        }
        else {
            cvLine( img, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
        }

        prev_pt = pt;
        imshow( "image", img );
    }
}


int main( int argc, char** argv )
{
    printf( "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - restore the original image\n" );

    namedWindow( "image", 1 );
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
