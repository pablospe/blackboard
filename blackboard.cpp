#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "LTKTrace.h"
#include "LipiEngineModule.h"


using namespace cv;
using namespace std;


#ifndef _WIN32
#define MAX_PATH 1024
#endif

/* Pointer to the LipiEngine interface */
LTKLipiEngineInterface *lipiEngine = NULL;
LTKShapeRecognizer *pShapeReco = NULL;


typedef vector<Point> Contour;
Contour current_contour;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

int shaperectst_init(int argc, char** argv);
int shaperectst_recog(LTKTraceGroup &inTraceGroup);
int shaperectst_end();


void on_mouse ( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || ! ( flags & CV_EVENT_FLAG_LBUTTON ) ) {
        if( prev_pt.x > 0 )
        {
            cout << "End!\n";

            // using LipiTk
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

            current_contour.clear();
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

    shaperectst_end();

    return 1;
}


// Parte del main del programa shaperectst
int shaperectst_init(int argc, char** argv)
{
    char *envstring = NULL;
    int iResult;


    // first argument is the logical project name
    if(argc < 2)
    {
        cout << endl << "Usage:";
        cout << endl << "./blackboard <projectname>" << endl;
        return -1;
    }

    // Get the LIPI_ROOT environment variable
    envstring = getenv(LIPIROOT_ENV_STRING);
    if(envstring == NULL)
    {
        cout << endl << "Error, Environment variable is not set LIPI_ROOT" << endl;
        return -1;
    }

    // create an instance of LipiEngine Module
    lipiEngine = LTKLipiEngineModule::getInstance();

    // set the LIPI_ROOT path in Lipiengine module instance
    lipiEngine->setLipiRootPath(envstring);

    // Initialize the LipiEngine module
    iResult = lipiEngine->initializeLipiEngine();
    if(iResult != SUCCESS)
    {
        cout << iResult <<": Error initializing LipiEngine." << endl;
        return -1;
    }

    // Assign the logical name of the project to this string, i.e. NUMERALS_NUM
    string strLogicalName = string(argv[1]);
    lipiEngine->createShapeRecognizer(strLogicalName, &pShapeReco);
    if(pShapeReco == NULL)
    {
        cout << endl << "Error creating Shape Recognizer" << endl;
        return -1;
    }

    // You can also use project and profile name to create LipiEngine instance as follows...
    // string strProjectName = "hindi_gestures";
    // string strProfileName = "default";
    // LTKShapeRecognizer *pReco = lipiEngine->createShapeRecognizer(&strProjectName, &strProfileName);

    // Load the model data into memory before starting the recognition...
    iResult = pShapeReco->loadModelData();
    if(iResult != SUCCESS)
    {
        cout << endl << iResult << ": Error loading Model data." << endl;
        lipiEngine->deleteShapeRecognizer(&pShapeReco);
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
        lipiEngine->deleteShapeRecognizer(&pShapeReco);

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
    lipiEngine->deleteShapeRecognizer(&pShapeReco);
    
    return 0;
}