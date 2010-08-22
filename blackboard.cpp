#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace cv;
using namespace std;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};


map< string, vector<Point> > contours;
map< float, string > result;  // sorting the result

vector<Point> contour_a;
vector<Point> contour_b;
vector<Point> contour_c;
vector<Point> contour_d;
vector<Point> contour_e;

vector<Point> current_contour;


void copyFrom(Seq<Point> &seq, vector<Point>& vec)
{
    for (int i=0; i < vec.size(); i++) {
        seq.push_back(vec[i]);
    }
}

std::ostream& operator<< (std::ostream &o, const vector<Point> &v)
{
    for (int i=0; i<v.size(); i++) {
        cout << "Point: (" << v[i].x << "," << v[i].y << ")" << endl;
    }
}

std::ostream& operator<< (std::ostream &o, const Seq<Point> &s)
{
    vector<Point> vec;
    s.copyTo(vec);
    cout << vec;
}



void on_mouse( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON) ) {
        if( prev_pt.x > 0 ) {
            printf( "End!\n");

            int hist_size[] = {32, 32};
            CvHistogram *current_hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY );

            MemStorage storage_current = cvCreateMemStorage(0);
            Seq<Point> seq_contour_current(storage_current);
            copyFrom( seq_contour_current, current_contour );

            cvCalcPGH( seq_contour_current.seq, current_hist );

            CvHistogram *hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY );

            map< string, vector<Point> >::iterator it;
            for ( it=contours.begin() ; it != contours.end(); it++ ) {
                const string &letra = (*it).first;
                vector<Point> &contour = (*it).second;

                // Transformo a cvSeq, ya que cvCalcPGH necesita este tipo
                MemStorage storage = cvCreateMemStorage(0);
                Seq<Point> seq_contour( storage );
                copyFrom( seq_contour, contour );

                // calculo el histograma con PGH
                cvCalcPGH( seq_contour.seq , hist );

                float res = cvCompareHist( hist, current_hist, CV_COMP_CORREL );
                cout << "cvCompareHist(" << letra << ",.) = " << res << endl;
                res = -res; // high score represents a better match

                // Metodo 2
//                 float res = matchShapes( Mat(contour), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
//                 cout << "matchShapes(" << letra << ",.) = " << res << endl;

                result[ res ] = letra;
            }
            cvReleaseHist(&hist);

            cout << "Letra " << result.begin()->second  << "!!!!!\n";
//             cout << "Letra " << result.rbegin()->second << "!!!!!\n"; // last one - rbeing (reverse begin)

            current_contour.clear();
            result.clear();
            cvReleaseHist(&current_hist);
        }
        prev_pt = cvPoint(-1,-1);
    }
    else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) )
    {
        CvPoint pt = cvPoint(x,y);

        if( prev_pt.x < 0 ) {
            prev_pt = pt;
            printf( "Start moving!\n");
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

    contours[ "a" ] = contour_a;
    contours[ "b" ] = contour_b;
    contours[ "c" ] = contour_c;
    contours[ "d" ] = contour_d;
    contours[ "e" ] = contour_e;

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
