#include <opencv2/opencv.hpp>
#include <raspicam_cv.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>

using namespace std;
using namespace cv;
using namespace raspicam;

Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate;
Mat ROILane;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result;
stringstream ss;

RaspiCam_Cv Camera;

vector<int> histrogramLane;

Point2f Source[] = {Point2f(55,145),Point2f(330,145),Point2f(10,195), Point2f(360,195)};
Point2f Destination[] = {Point2f(100,0),Point2f(280,0),Point2f(100,240), Point2f(280,240)};


void Setup ( int argc,char **argv, RaspiCam_Cv &Camera )
{
    Camera.set ( CAP_PROP_FRAME_WIDTH,  ( "-w",argc,argv,400 ) );
    Camera.set ( CAP_PROP_FRAME_HEIGHT,  ( "-h",argc,argv,240 ) );
    Camera.set ( CAP_PROP_BRIGHTNESS, ( "-br",argc,argv,50 ) );
    Camera.set ( CAP_PROP_CONTRAST ,( "-co",argc,argv,50 ) );
    Camera.set ( CAP_PROP_SATURATION,  ( "-sa",argc,argv,50 ) );
    Camera.set ( CAP_PROP_GAIN,  ( "-g",argc,argv ,50 ) );
    Camera.set ( CAP_PROP_FPS,  ( "-fps",argc,argv,100));
}

void Capture()
{
    Camera.grab();
    Camera.retrieve( frame);
}

void Perspective()
{
	line(frame,Source[0], Source[1], Scalar(255,255,255), 2);
	line(frame,Source[1], Source[3], Scalar(255,255,255), 2);
	line(frame,Source[3], Source[2], Scalar(255,255,255), 2);
	line(frame,Source[2], Source[0], Scalar(255,255,255), 2);
	
	Matrix = getPerspectiveTransform(Source, Destination);
	warpPerspective(frame, framePers, Matrix, Size(400,240));
	for(int i = 0; i < framePers.rows; i++)
	    for(int j = 0; j < framePers.cols; j++)
	    {//Point2f Destination[] = {Point2f(100,0),Point2f(280,0),Point2f(100,240), Point2f(280,240)};
            if(j > 280 || j < 100)//framePers.ptr<Vec3b>(i)[j]==cv::Vec3b(0,0,0))
            {
                *(framePers.data + framePers.step[0] * i + framePers.step[1] * j + framePers.elemSize1() * 0) = 255;
                *(framePers.data + framePers.step[0] * i + framePers.step[1] * j + framePers.elemSize1() * 1) = 255;
                *(framePers.data + framePers.step[0] * i + framePers.step[1] * j + framePers.elemSize1() * 2) = 255;
            }
	    }
}

void Threshold()
{
	cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
	inRange(frameGray, 0, 80, frameThresh);
	Canny(frameGray,frameEdge, 600, 800, 3, false);
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	dilate(frameEdge, frameEdge, element);
	frameGray = frameGray + frameEdge;
	Canny(frameGray,frameEdge, 300, 700, 3, false);
	add(frameThresh, frameEdge, frameFinal);
	cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
	cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR);   //used in histrogram function only
}

void Histrogram()
{
    histrogramLane.resize(400);
    histrogramLane.clear();
    
    for(int i=0; i<400; i++)       //frame.size().width = 400
    {
        ROILane = frameFinalDuplicate(Rect(i,140,1,100));
        divide(255, ROILane, ROILane);
        histrogramLane.push_back((int)(sum(ROILane)[0])); 
    }
}

void LaneFinder()
{
    vector<int>:: iterator LeftPtr;
    LeftPtr = max_element(histrogramLane.begin()+150, histrogramLane.begin()+200);
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr); 
    
    vector<int>:: iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin() +200, histrogramLane.begin()+250);
    RightLanePos = distance(histrogramLane.begin(), RightPtr);
    
    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), Scalar(0, 255,0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), Scalar(0,255,0), 2); 
}

void LaneCenter()
{
    laneCenter = (RightLanePos-LeftLanePos)/2 +LeftLanePos;
    frameCenter = 188;
    
    line(frameFinal, Point2f(laneCenter,0), Point2f(laneCenter,240), Scalar(0,255,0), 3);
    line(frameFinal, Point2f(frameCenter,0), Point2f(frameCenter,240), Scalar(255,0,0), 3);

    Result = laneCenter-frameCenter;
}



int main(int argc,char **argv)
{
    wiringPiSetup();
    pinMode(21, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);	
	Setup(argc, argv, Camera);
	cout<<"Connecting to camera"<<endl;

	if (!Camera.open())
	{	
	    cout<<"Failed to Connect"<<endl;
    }
     
    cout<<"Camera Id = "<<Camera.getId()<<endl;
     
    while(1)
    {
        //auto start = std::chrono::system_clock::now();

        Capture();
        Perspective();
        Threshold();
        Histrogram();
        LaneFinder();
        LaneCenter();

        if (Result != 100)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 0);    //decimal = 0
            digitalWrite(23, 1);
            digitalWrite(24, 0);
            cout<<"Forward"<<endl;
        }
        
            
        else if (Result > 0 && Result <10)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 0);    //decimal = 1
            digitalWrite(23, 0);
            digitalWrite(24, 0);
            cout<<"Right1"<<endl;
        }
        
        else if (Result >=10 && Result <20)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 1);    //decimal = 2
            digitalWrite(23, 0);
            digitalWrite(24, 0);
            cout<<"Right2"<<endl;
        }
        
        else if (Result >20)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 1);    //decimal = 3
            digitalWrite(23, 0);
            digitalWrite(24, 0);
            cout<<"Right3"<<endl;
        }
        
        else if (Result <0 && Result >-10)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 0);    //decimal = 4
            digitalWrite(23, 1);
            digitalWrite(24, 0);
            cout<<"Left1"<<endl;
        }
        
        else if (Result <=-10 && Result >-20)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 0);    //decimal = 5
            digitalWrite(23, 1);
            digitalWrite(24, 0);
            cout<<"Left2"<<endl;
        }
        
        else if (Result <-20)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 1);    //decimal = 6
            digitalWrite(23, 1);
            digitalWrite(24, 0);
            cout<<"Left3"<<endl;
        }

        ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result;
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
        
        namedWindow("orignal", WINDOW_KEEPRATIO);
        moveWindow("orignal", 0, 100);
        resizeWindow("orignal", 640, 480);
        imshow("orignal", frame);
        
        namedWindow("Perspective", WINDOW_KEEPRATIO);
        moveWindow("Perspective", 640, 100);
        resizeWindow("Perspective", 640, 480);
        imshow("Perspective", framePers);
        
        namedWindow("Final", WINDOW_KEEPRATIO);
        moveWindow("Final", 1280, 100);
        resizeWindow("Final", 640, 480);
        imshow("Final", frameFinal);
        
        waitKey(1);
        //auto end = std::chrono::system_clock::now();
        //std::chrono::duration<double> elapsed_seconds = end-start;
        
        //float t = elapsed_seconds.count();
        //int FPS = 1/t;
        //cout<<"FPS = "<<FPS<<endl;
    }

    return 0;
     
}
