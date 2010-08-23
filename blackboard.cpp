#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace cv;
using namespace std;

typedef vector<Point> Contour;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

int hist_size[] = {128, 128};

map<string, Contour>       contours;
map<string, CvHistogram*>  histograms;
map<float, string>         result;  // sorting the result

Contour contour_a;
Contour contour_b;
Contour contour_c;
Contour contour_d;
Contour contour_e;

Contour current_contour;


void copyFrom(Seq<Point> &seq, Contour& vec)
{
    for(int i=0; i < vec.size(); i++) {
        seq.push_back(vec[i]);
    }
}

std::ostream& operator<< (std::ostream &o, const Contour &v)
{
    for(int i=0; i<v.size(); i++) {
        cout << "Point: (" << v[i].x << "," << v[i].y << ")" << endl;
    }
}

std::ostream& operator<< (std::ostream &o, const Seq<Point> &s)
{
    Contour vec;
    s.copyTo(vec);
    cout << vec;
}

void calcPGH( Contour &contour, CvHistogram *hist )
{
    // Contour -> cvSeq
    MemStorage storage = cvCreateMemStorage(0);
    Seq<Point> seq_contour( storage );
    copyFrom( seq_contour, contour );

    // PGH
    cvCalcPGH( seq_contour.seq, hist );
}

float calcEMD2( CvHistogram *hist1, CvHistogram *hist2 )
{
    // Convert histograms into signatures for EMD matching
    // assume we already have 2D histograms hist1 and hist2
    // that are both of dimension h_bins by s_bins (though for EMD,
    // histograms don’t have to match in size).
    int h_bins = 32,  s_bins = 32;
    int numrows = h_bins * s_bins;

    // Create matrices to store the signature in
    CvMat* sig1 = cvCreateMat( numrows, 3, CV_32FC1 ); //1 count + 2 coords = 3
    CvMat* sig2 = cvCreateMat( numrows, 3, CV_32FC1 ); //sigs are of type float.

    // Fill signatures for the two histograms
    for ( int h = 0; h < h_bins; h++ ) {
        for ( int s = 0; s < s_bins; s++ ) {
            float bin_val = cvQueryHistValue_2D( hist1, h, s );
            cvSet2D( sig1, h*s_bins + s, 0, cvScalar( bin_val ) ); //bin value
            cvSet2D( sig1, h*s_bins + s, 1, cvScalar( h ) );
            //Coord 1
            cvSet2D( sig1, h*s_bins + s, 2, cvScalar( s ) );
            //Coord 2

            bin_val = cvQueryHistValue_2D( hist2, h, s );
            cvSet2D( sig2, h*s_bins + s, 0, cvScalar( bin_val ) ); //bin value
            cvSet2D( sig2, h*s_bins + s, 1, cvScalar( h ) );
            //Coord 1
            cvSet2D( sig2, h*s_bins + s, 2, cvScalar( s ) );
            //Coord 2
        }
    }

    // Do EMD
    return cvCalcEMD2(sig1,sig2,CV_DIST_L2);
}


void on_mouse ( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || ! ( flags & CV_EVENT_FLAG_LBUTTON ) ) {
        if( prev_pt.x > 0 ) {
            printf( "End!\n" );

            CvHistogram *current_hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY );
            calcPGH( current_contour, current_hist );

            map< string, Contour >::iterator it;
            for( it = contours.begin() ; it != contours.end(); it++ ) {
                const string &letter  = (*it).first;
                Contour      &contour = (*it).second;

                // Metodo 1 - cvCompareHist
                float res = cvCompareHist( histograms[letter], current_hist, CV_COMP_CORREL );
                cout << "cvCompareHist(" << letter << ",.) = " << res << endl;
                res = -res; // high score represents a better match

                // Metodo 2 - matchShapes
//                 float res = matchShapes( Mat(contour), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
//                 cout << "matchShapes(" << letter << ",.) = " << res << endl;

                // Metodo 3 - cvCalcEMD2  (Very low! Use Simplex!)
//                 float res = calcEMD2( histograms[letter], current_hist );
//                 cout << "cvCalcEMD2(" << letter << ",.) = " << res << endl;

                result[ res ] = letter;
            }

            cout << "Letra " << result.begin()->second  << "!!!!!\n";
//             cout << "Letra " << result.rbegin()->second << "!!!!!\n"; // last one - rbeing (reverse begin)

            current_contour.clear();
            result.clear();
            cvReleaseHist( &current_hist );
        }
        prev_pt = cvPoint( -1, -1 );
    }
    else
        if( event == CV_EVENT_MOUSEMOVE && ( flags & CV_EVENT_FLAG_LBUTTON ) ) {
            CvPoint pt = cvPoint( x, y );

            if( prev_pt.x < 0 ) {
                prev_pt = pt;
                printf( "Start moving!\n" );
            }
            else {
                cvLine( img, prev_pt, pt, cvScalarAll ( 255 ), 5, 8, 0 );
            }

            printf( "contour.push_back(Point2f(%d,%d));\n", x, y );
            current_contour.push_back ( Point2f ( x, y ) );

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

    // Calculate histograms (for training letters)
    map<string, Contour>::iterator it;
    for( it = contours.begin() ; it != contours.end(); it++ ) {
        const string  &letter  = ( *it ).first;
        Contour       &contour = ( *it ).second;

        histograms[letter] = cvCreateHist( 2, hist_size, CV_HIST_ARRAY );;
        calcPGH( contour, histograms[letter] );
    }


    printf( "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - restore the blackboard\n" );

    namedWindow( "image" );
    cvSetMouseCallback( "image", on_mouse, 0 );

    /* create an image */
    IplImage *img0 = cvCreateImage( cvSize( 400, 400 ), IPL_DEPTH_8U, 3 );

    img = cvCloneImage( img0 );
    imshow( "image", img );

    for( ;; ) {
        int c = waitKey( 0 );

        if(( char ) c == 27 )
            break;

        if(( char ) c == 'r' ) {
            cvCopy( img0, img );
            cvShowImage( "image", img );
        }
    }


    // Release Histograms
    for( it = contours.begin() ; it != contours.end(); it++ ) {
        cvReleaseHist( &histograms[( *it ).first] );
    }

    return 1;
}
