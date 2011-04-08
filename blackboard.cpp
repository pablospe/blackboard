#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace cv;
using namespace std;


// #include "LTKLipiEngineInterface.h"
#include "LTKTrace.h"
#include <../lipiengine/lipiengine.h>

#ifndef _WIN32
#define MAX_PATH 1024
#endif

/* Pointer to the LipiEngine interface */
LTKLipiEngineInterface *ptrObj = NULL;
LTKShapeRecognizer *pShapeReco = NULL;



typedef vector<Point> Contour;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

int hist_size[] = {32, 32};

map<string, Contour>       contours;
map<string, CvHistogram*>  histograms;
map<float, string>         result;  // sorting the result

Contour contour_a;
Contour contour_b;
Contour contour_c;
Contour contour_d;
Contour contour_e;

Contour current_contour;

int shaperectst_init(int argc, char** argv);
int shaperectst_recog(LTKTraceGroup &inTraceGroup);
int shaperectst_end();

void copyFrom(Seq<Point> &seq, Contour& vec)
{
    for(unsigned i=0; i < vec.size(); i++) {
        seq.push_back(vec[i]);
    }
}

std::ostream& operator<< (std::ostream &o, const Contour &v)
{
    for(unsigned i=0; i<v.size(); i++) {
        o << "Point: (" << v[i].x << "," << v[i].y << ")" << endl;
    }

    return o;
}

std::ostream& operator<< (std::ostream &o, const Seq<Point> &s)
{
    Contour vec;
    s.copyTo(vec);
    o << vec;

    return o;
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
            cout << "End!\n";

            CvHistogram *current_hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY );
            calcPGH( current_contour, current_hist );

//             map< string, Contour >::iterator it;
//             for( it = contours.begin() ; it != contours.end(); it++ ) {
//                 const string &letter  = (*it).first;
//                 Contour      &contour = (*it).second;

                // Metodo 1 - cvCompareHist
//                 float res = cvCompareHist( histograms[letter], current_hist, CV_COMP_CORREL );
//                 cout << "cvCompareHist(" << letter << ",.) = " << res << endl;
//                 res = -res; // high score represents a better match

                // Metodo 2 - matchShapes
//                 float res = matchShapes( Mat(contour), Mat(current_contour), CV_CONTOURS_MATCH_I3, 0);
//                 cout << "matchShapes(" << letter << ",.) = " << res << endl;

                // Metodo 3 - cvCalcEMD2  (Very low! Use Simplex!)
//                 float res = calcEMD2( histograms[letter], current_hist );
//                 cout << "cvCalcEMD2(" << letter << ",.) = " << res << endl;

                // Metodo 4 - Lipi
                float res = 0;
                LTKTrace trace;
                for( vector< Point >::iterator iter = current_contour.begin();
                     iter != current_contour.end(); iter++)
                {
                    floatVector point;
                    
                    point.push_back( iter->x );
                    point.push_back( iter->y );
                    
                    trace.addPoint(point);
                }
                LTKTraceGroup traceGroup(trace);
                res = shaperectst_recog(traceGroup);

//                 result[ res ] = letter;
//             }

//             cout << "Letra " << result.begin()->second  << "!!!!!\n";
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
            imshow( "Blackboard", img );
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


    if( shaperectst_init(argc, argv) != 0 )
    {
        cout << "Saliendo...\n";
        return -1;
    }
    
    cout << "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - restore the blackboard\n";

    namedWindow( "Blackboard" );
    cvSetMouseCallback( "Blackboard", on_mouse, 0 );

    /* create an Blackboard */
    IplImage *img0 = cvCreateImage( cvSize( 800, 600 ), IPL_DEPTH_8U, 3 );

    img = cvCloneImage( img0 );
    imshow( "Blackboard", img );

    for( ;; ) {
        int c = waitKey( 0 );

        if(( char ) c == 27 )
            break;

        if(( char ) c == 'r' ) {
            cvCopy( img0, img );
            cvShowImage( "Blackboard", img );
        }
    }


    // Release Histograms
    for( map< string, Contour >::const_iterator it = contours.begin() ; it != contours.end(); it++ ) {
        cvReleaseHist( &histograms[( *it ).first] );
    }

    shaperectst_end();

    return 1;
}


// Parte del main del programa shaperectst
int shaperectst_init(int argc, char** argv)
{
    char *envstring = NULL;
    int iResult;


    // first argument is the logical project name and the
    // second argument is the ink file to recognize
    if(argc < 3)
    {
        cout << endl << "Usage:";
        cout << endl << "shaperectst <logical projectname> <ink file to recognize>";
        cout << endl;
        return -1;
    }

    //Get the LIPI_ROOT environment variable
    envstring = getenv(LIPIROOT_ENV_STRING);
    if(envstring == NULL)
    {
        cout << endl << "Error, Environment variable is not set LIPI_ROOT" << endl;
        return -1;
    }

    //create an instance of LipiEngine Module
    ptrObj = createLTKLipiEngine();

    // set the LIPI_ROOT path in Lipiengine module instance
    ptrObj->setLipiRootPath(envstring);

    //Initialize the LipiEngine module
    iResult = ptrObj->initializeLipiEngine();
    if(iResult != SUCCESS)
    {
        cout << iResult <<": Error initializing LipiEngine." << endl;
        return -1;
    }

    //Assign the logical name of the project to this string, i.e. TAMIL_CHAR
    //(or) "HINDI_GESTURES"
    string strLogicalName = string(argv[1]);
    ptrObj->createShapeRecognizer(strLogicalName, &pShapeReco);
    if(pShapeReco == NULL)
    {
        cout << endl << "Error creating Shape Recognizer" << endl;
        return -1;
    }

    //You can also use project and profile name to create LipiEngine instance as follows...
    //string strProjectName = "hindi_gestures";
    //string strProfileName = "default";
    //LTKShapeRecognizer *pReco = ptrObj->createShapeRecognizer(&strProjectName, &strProfileName);

    //Load the model data into memory before starting the recognition...
    iResult = pShapeReco->loadModelData();
    if(iResult != SUCCESS)
    {
        cout << endl << iResult << ": Error loading Model data." << endl;
        ptrObj->deleteShapeRecognizer(&pShapeReco);
        return -1;
    }


    cout << endl << "Input Logical project name = " << strLogicalName << endl;


    return 0;
}




//Declare variables to be used for recognition...
LTKCaptureDevice captureDevice;
LTKScreenContext screenContext;
vector<int> shapeSubset;
int numChoices = 2;
float confThreshold = 0.0f;
vector<LTKShapeRecoResult> results;
// LTKTraceGroup inTraceGroup;


int shaperectst_recog(LTKTraceGroup &inTraceGroup)
{
    int iResult;

    //  Set the device context, once before starting the recognition...
    pShapeReco->setDeviceContext(captureDevice);

    results.clear();
    results.reserve(numChoices);

    //now call the "recognize" method
    iResult = pShapeReco->recognize(inTraceGroup, screenContext, shapeSubset, confThreshold, numChoices, results);
    if(iResult != SUCCESS)
    {
        cout << iResult << ": Error while recognizing." << endl;
        ptrObj->deleteShapeRecognizer(&pShapeReco);

        return -1;
    }

    cout << endl << "Recognition Results\n\n";

    //Display the recognized results...
    for(unsigned index =0; index < results.size(); ++index)
    {
        cout << "Choice[" << index << "] " << "Recognized Shapeid = " << results[index].getShapeId() << " Confidence = " << results[index].getConfidence() << endl;
    }

    cout << "Numero reconocido: " <<  results[0].getShapeId() << " !!!!\n";

    return results[0].getConfidence();
}

int shaperectst_end()
{
    //Delete the shape recognizer object
    ptrObj->deleteShapeRecognizer(&pShapeReco);
    
    return 0;
}