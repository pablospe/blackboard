#include <istream>
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

int samples = 0;


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


void drawTrace( floatVector &x, floatVector &y )
{
    assert( x.size() == y.size() && img );

    normalize(x,800);
    normalize(y,600);

    // Y =  -Y
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



void matlab_output_current( const LTKTrace &trace, int digit, int num_stroke =1 )
{
    int L = trace.getNumberOfPoints();
    LTKTraceFormat format = trace.getTraceFormat();
    stringVector channels = format.getAllChannelNames();

    floatVector outPointCoordinates, x, y;
    trace.getChannelValues( 0, x );
    trace.getChannelValues( 1, y );

//     drawTrace(x, y);

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


// Example (matlab output code)
// x = [ ... ];
// y = [ ... ];
// t=trace;
//     t.channel{1} = x; t.dim{1} = 800;
//     t.channel{2} = y; t.dim{2} = 600;
//     t.label = '0';

    cout << "x = [ ";
    int i, size;
    size = x.size();
    for( i=0; i < size-1; i++ ){
        cout << x[i] << ", ";
    }
    cout << x[i] << " ];\n";

    cout << "y = [ ";
    size = y.size();
    for( i=0; i < size-1; i++ ){
        cout << y[i] << ", ";
    }
    cout << y[i] << " ];\n";
}



void matlab_output( const LTKTraceVector &traces, int label )
{
    int N = traces.size();
    if( N == 0 )
    {
        cerr << "Empty trace!\n";
//         exit(1);
        return;
    }

    for( int i=0; i < N; i++ )
    {
        matlab_output_current( traces[i], label );
        cout << "t=trace;\n"
             << "\tt.channel{1} = x; t.dim{1} = 800;"
             << "\tt.channel{2} = y; t.dim{2} = 600;"
             << "\tt.label = '" << label << "';\n";
        cout << "db{" << ++samples <<"} = t;\n";
    }
}

void readUnipenFile( const string &unipenFileName, LTKTraceVector &traces )
{
    LTKTraceGroup traceGroup;
    LTKCaptureDevice captureDevice;
    LTKScreenContext screenContext;
  
    try
    {
        LTKInkFileReader::readUnipenInkFile( unipenFileName, traceGroup, captureDevice, screenContext );
    }
    catch(LTKException e)
    {
        errorCode = EINK_FILE_OPEN;
        exit(1);
    }

    traces = traceGroup.getAllTraces();
}


int str2int (const string &str) {
    stringstream ss(str);
    int n;
    ss >> n;  
    return n;
}

// Concatenate two traces
LTKTrace concat( const LTKTrace &a, const LTKTrace &b )
{
    floatVector pointVec;
    LTKTrace trace = a;

    int N = b.getNumberOfPoints();
    for( int i=0; i<N ; i++ )
    {
        b.getPointAt(i, pointVec);
        trace.addPoint( pointVec );
        pointVec.clear();
    }

    return trace;
}


void grouping( const string &data, LTKTraceVector &traces )
{
    LTKTraceVector output;
    string line, str;
    int begin, end;
   
    ifstream myfile( data.c_str() );
    if( myfile.is_open() )
    {
        while( myfile.good() )
        {
            getline (myfile,line);
//             cout << line << endl;
            
            istringstream iss(line);

            iss >> str;
            if( str == ".SEGMENT" )
            {
                iss >> str;
                iss >> str;
//                 cout << "Substring: " << str << endl;

                size_t found =   str.find_last_of("-");
                begin = str2int( str.substr(0,found) );
                  end = str2int( str.substr(found+1) );
//                 cout << "Substring2: " << begin << " - " << end << endl;

                // Concatenate two traces
                if( begin == end )
                    output.push_back( traces[begin] );
                else
                    output.push_back( concat(traces[begin], traces[end]) );
            }
        }
        
        myfile.close();
    }
    else {
        cerr << "Unable to open file";
    }

    traces = output;
}

void createDB( const vector<string> &files,
               const vector<string> &data,
               const vector<string> &labels )
{
    int size = files.size();
    LTKTraceVector traces;
    
    for(int i = 0; i < size ; i++)
    {
        readUnipenFile( files[i], traces );
        grouping( data[i], traces );
        matlab_output( traces, atoi(labels[i].c_str()) );
    }
}


string find_and_replace( string source, const string &find, const string &replace ) {

    size_t j;
    for ( ; (j = source.find( find )) != string::npos ; ) {
        source.replace( j, find.length(), replace );
    }

    return source;
}

int main( int argc, char** argv )
{
//     cout << "Hot keys: \n"
//             "\tESC - quit the program\n"
//             "\tr - restore the blackboard\n";

//     namedWindow( "" );
//     cvSetMouseCallback( "", on_mouse, 0 );
// //
//     /* create an Blackboard */
//     IplImage *img0 = cvCreateImage( cvSize( 800, 600 ), IPL_DEPTH_8U, 3 );
// 
//     img = cvCloneImage( img0 );
//     imshow( "", img );

    
    vector<string> files, data, labels;

    int size = argc-1;
    labels.reserve(size);
     files.reserve(size);
      data.reserve(size);

    //! Getting "files" from command-line
    for(int i = 1; i < argc; i++)
    {
        files.push_back(argv[i]);
    }

    //! Creating "data" and "labels"
    for(int i = 0; i < size ; i++)
    {
        string str = files[i];
        
        data.push_back( find_and_replace( str, "include/", "data/") );

        // get basename (without dirname)
        size_t found = str.find_last_of("/");
        str = str.substr(found+1);

        // get filename (wihtout extension)
        found = str.find_last_of(".");
        labels.push_back( str.substr(0,found) );

//         cout << "Data: " << data[i] << "\tlabel: " << labels[i] << endl;
    }


    createDB( files, data, labels );

    
/*    for( ;; ) {
        int c = waitKey( 0 );

        if(( char ) c == 27 )
            break;

        if(( char ) c == 'r' ) {
            cvCopy( img0, img );
            imshow( "", img );
        }
    }*/
    
    return 0;
}
