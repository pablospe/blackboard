#include <assert.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "LTKTrace.h"
#include "LTKTraceGroup.h"
#include "LTKCaptureDevice.h"
#include "LTKScreenContext.h"
#include "LTKInkFileReader.h"
#include "LTKException.h"
#include "LTKErrors.h"
#include "LTKErrorsList.h"
#include "LTKMacros.h"

using namespace cv;
using namespace std;

typedef vector<Point> Contour;
Contour current_contour;

IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

void drawLine( IplImage* img, const CvPoint &prev_pt, const CvPoint &pt )
{
    cvLine( img, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    imshow( "", img );
}

void normalize( floatVector &x, const int res )
{
    float min = *min_element( x.begin(), x.end());
    float max = *max_element( x.begin(), x.end());
    
    int L = x.size();
    for(int i=0; i<L; i++)
    {
//         cout << x[i] << " -> " << (x[i]-min)/(max-min) * res << endl;
        x[i] = (x[i]-min)/(max-min) * res;
    }
}


void drawTrace( floatVector x, floatVector y )
{
    assert( x.size() == y.size() && img );

    normalize(x,800);
    normalize(y,600);

    for(int i=0; i<y.size(); i++)
    {
        y[i] = -y[i]+600;
    }    
    
    int L = x.size(); 
    for( int i=0; i<L; i++ ) {
        CvPoint pt = cvPoint( x[i], y[i] );
//         cout << pt.x << " " << pt.y << "\n";
        
        if( i== 0 ) {
            prev_pt = pt;
//             cout << " A = [ ";
        }
        else {
            drawLine( img, prev_pt, pt );
        }

//         cout.flush();
//         current_contour.push_back ( Point2f ( x, y ) );

        prev_pt = pt;
//         imshow( "", img );
    }
    prev_pt = cvPoint( -1, -1 );
}

void drawUnipenFileToImage(const string& unipenFileName,const string& imgFileName)
{
    LTKTraceGroup traceGroup;
    LTKCaptureDevice captureDevice;
    LTKScreenContext screenContext;
    string strFileName(unipenFileName);
    try
    {
        LTKInkFileReader::readUnipenInkFile(strFileName,traceGroup,captureDevice,screenContext);
    }
    catch(LTKException e)
    {
        errorCode = EINK_FILE_OPEN;
    }

    LTKTraceVector traces = traceGroup.getAllTraces();
    cout << "traces.size() = " << traces.size() << endl;

    if( traces.size() == 0 )
    {
        cout << "Empty trace!\n";
        exit(1);
    }
    
    int L = traces[0].getNumberOfPoints();
    LTKTraceFormat format = traces[0].getTraceFormat();
    stringVector channels = format.getAllChannelNames();
    
    floatVector outPointCoordinates, x, y;
    traces[0].getChannelValues( 0, x );
    traces[0].getChannelValues( 1, y );

    drawTrace(x, y);
    
//     for( int i=0; i < L; i++ ) {      
//         traces[0].getPointAt(i, outPointCoordinates);
//         cout << "i " << i << "\t\t"
//              << "outPointCoordinates.size() = " << outPointCoordinates.size() << "\t\t"
//              << "(x,y) = (" << outPointCoordinates[0] << "," << outPointCoordinates[1] << ")\n";
// 
//         
// //         CvPoint pt = cvPoint( outPointCoordinates[0], outPointCoordinates[1] );
//         
//         outPointCoordinates.clear();
//     }

    cout << "x = [ ";
    for( int i=0; i < x.size(); i++ ){
        cout << x[i] << ", ";
    }
//     cout << "\b\b ";  // Needed! It deletes the last comma
    cout << "\b\b ]\n";

    cout << "y = [ ";
    for( int i=0; i < y.size(); i++ ){
        cout << y[i] << ", ";
    }
    cout << "\b\b ]\n";
}




void on_mouse ( int event, int x, int y, int flags, void* )
{
    if( !img )
        return;

    if( event == CV_EVENT_LBUTTONUP || ! ( flags & CV_EVENT_FLAG_LBUTTON ) ) {
        if( prev_pt.x > 0 )
        {
            cout << "\b\b ";  // Needed! It deletes the last comma
            cout << "]; ";
            cout << " x = A(:,1)'; x = (x-min(x))/800; ";
            cout << " y = A(:,2)'; y = -y+600; y = (y-min(y))/600; ";
            cout << " recognition( feature_extraction(x,y,15), features )\n";

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
//             res = shaperectst_recog(traceGroup);

            current_contour.clear();
        }
        prev_pt = cvPoint( -1, -1 );
    }
    else
        if( event == CV_EVENT_MOUSEMOVE && ( flags & CV_EVENT_FLAG_LBUTTON ) ) {
            CvPoint pt = cvPoint( x, y );

            if( prev_pt.x < 0 ) {
                prev_pt = pt;
                cout << " A = [ ";
            }
            else {
                cvLine( img, prev_pt, pt, cvScalarAll ( 255 ), 5, 8, 0 );
            }

            cout << x << " " << y << "; ";
            cout.flush();
            current_contour.push_back ( Point2f ( x, y ) );

            prev_pt = pt;
            imshow( "", img );
        }
}


int main( int argc, char** argv )
{
    cout << "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - restore the blackboard\n";

    namedWindow( "" );
    cvSetMouseCallback( "", on_mouse, 0 );
//
    /* create an Blackboard */
    IplImage *img0 = cvCreateImage( cvSize( 800, 600 ), IPL_DEPTH_8U, 3 );

    img = cvCloneImage( img0 );
    imshow( "", img );

    drawUnipenFileToImage( "0.dat", "0.bmp" );
//     "1.dat"
//     "amywldrp.dat"
//     "ibo0.dat"

    
    for( ;; ) {
        int c = waitKey( 0 );

        if(( char ) c == 27 )
            break;

        if(( char ) c == 'r' ) {
            cvCopy( img0, img );
            imshow( "", img );
        }
    }
    
    return 0;
}
    