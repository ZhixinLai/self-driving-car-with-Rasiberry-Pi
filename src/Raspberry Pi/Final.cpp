#include <opencv2/opencv.hpp>
#include <raspicam_cv.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>

using namespace std;
using namespace cv;
using namespace raspicam;

// Image Processing variables
Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, frameFinalDuplicate1;
Mat ROILane, ROILaneEnd;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result, laneEnd;
stringstream ss;

RaspiCam_Cv Camera;

vector<int> histrogramLane;
vector<int> histrogramLaneEnd;

Point2f Source[] = {Point2f(55,145),Point2f(330,145),Point2f(10,195), Point2f(360,195)};
Point2f Destination[] = {Point2f(100,0),Point2f(280,0),Point2f(100,240), Point2f(280,240)};


// Machine Learning variables
CascadeClassifier Stop_Cascade, Object_Cascade, Traffic_Cascade;
Mat frame_Stop, RoI_Stop, gray_Stop, frame_Object, RoI_Object, gray_Object, frame_Traffic, RoI_Traffic, gray_Traffic;
vector<Rect> Stop, Object, Traffic;
int dist_Stop, dist_Object, dist_Traffic;

clock_t TIME_stop = 6 * 1000 * 1000, stop_time = -TIME_stop;
clock_t TIME_object = 10 * 1000 * 1000, object_time = -TIME_object; 
clock_t TIME_traffic = 6 * 1000 * 1000, traffic_time = -TIME_traffic;
clock_t TIME_laneend = 6 * 1000 * 1000, laneend_time = -TIME_laneend;

// camera setup
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

// capture image
void Capture()
{
    Camera.grab();
    Camera.retrieve( frame);
   // cvtColor(frame, frame_Stop,COLOR_BGR2RGB);  
   // cvtColor(frame_Stop, frame_Stop,COLOR_RGB2BGR); 
   frame_Stop = frame.clone();
   frame_Object = frame.clone();
   frame_Traffic = frame.clone();
}

// tranform to bird view perspective
void Perspective()
{
	line(frame,Source[0], Source[1], Scalar(255,255,255), 2);
	line(frame,Source[1], Source[3], Scalar(255,255,255), 2);
	line(frame,Source[3], Source[2], Scalar(255,255,255), 2);
	line(frame,Source[2], Source[0], Scalar(255,255,255), 2);
	
	Matrix = getPerspectiveTransform(Source, Destination);
	warpPerspective(frame, framePers, Matrix, Size(400,240));
	for(int i = 0; i < framePers.rows; i++)
	{
	    for(int j = 0; j < framePers.cols; j++)
	    {
		// change left and right part's pixel value from black to white 
		// so that black track edge can be distinguished from left and right part in the threshold() function
		if(j > 280 || j < 100)//framePers.ptr<Vec3b>(i)[j]==cv::Vec3b(0,0,0))
		{
		    *(framePers.data + framePers.step[0] * i + framePers.step[1] * j + framePers.elemSize1() * 0) = 255;
		    *(framePers.data + framePers.step[0] * i + framePers.step[1] * j + framePers.elemSize1() * 1) = 255;
		    *(framePers.data + framePers.step[0] * i + framePers.step[1] * j + framePers.elemSize1() * 2) = 255;
		}
	    }	
	}
}
 
void Threshold()
{
	cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
	inRange(frameGray, 0, 80, frameThresh);//[][][]
	// get rid of white spots on two lines caused by Perspective()
	//   find white spots
	Canny(frameGray,frameEdge, 600, 800, 3, false);//[][][]
	//   amplify white spots using dilate
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));//[][][]
	dilate(frameEdge, frameEdge, element);
	//   get rid of white spots by adding two images
	frameGray = frameGray + frameEdge;
	Canny(frameGray,frameEdge, 300, 700, 3, false);//[][][]
	add(frameThresh, frameEdge, frameFinal);
	cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
	cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR);   //used in histrogram function only
	cvtColor(frameFinal, frameFinalDuplicate1, COLOR_RGB2BGR);   //used in histrogram function only	
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
    
    histrogramLaneEnd.resize(400);
    histrogramLaneEnd.clear();
    for (int i = 0; i < 400; i++)       
    {
	ROILaneEnd = frameFinalDuplicate1(Rect(i, 0, 1, 240));   
	divide(255, ROILaneEnd, ROILaneEnd);       
	histrogramLaneEnd.push_back((int)(sum(ROILaneEnd)[0]));  
    }
   laneEnd = sum(histrogramLaneEnd)[0];
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

void Stop_detection()
{
    if(!Stop_Cascade.load("//home//pi//Desktop//Stop_cascade.xml"))
    {
	printf("Unable to open stop cascade file");
    }
    
    RoI_Stop = frame_Stop(Rect(200,0,200,140));
    cvtColor(RoI_Stop, gray_Stop, COLOR_RGB2GRAY);
    equalizeHist(gray_Stop, gray_Stop);
    Stop_Cascade.detectMultiScale(gray_Stop, Stop);
    dist_Stop = 0;
    
    for(int i=0; i<Stop.size(); i++)
    {
	Point P1(Stop[i].x, Stop[i].y);
	Point P2(Stop[i].x + Stop[i].width, Stop[i].y + Stop[i].height);
	
	rectangle(RoI_Stop, P1, P2, Scalar(0, 0, 255), 2);
	putText(RoI_Stop, "Stop Sign", P1, FONT_HERSHEY_PLAIN, 1,  Scalar(0, 0, 255, 255), 2);
	dist_Stop = (-1.07)*(P2.x-P1.x) + 102.597;//[][][]
        ss.str(" ");
        ss.clear();
        ss<<"D = "<<dist_Stop<<" (cm)";
        putText(RoI_Stop, ss.str(), Point2f(1,130), 0, 0.5, Scalar(0,0,255), 2);	
	
    }    
    
}

void Object_detection()
{
    if(!Object_Cascade.load("//home//pi//Desktop//Object_cascade.xml"))
    {
	printf("Unable to open Object cascade file");
    }
    
    RoI_Object = frame_Object(Rect(100,100,200,140));
    cvtColor(RoI_Object, gray_Object, COLOR_RGB2GRAY);
    equalizeHist(gray_Object, gray_Object);
    Object_Cascade.detectMultiScale(gray_Object, Object);
    
    dist_Object = 0;
    for(int i=0; i<Object.size(); i++)
    {
	Point P1(Object[i].x, Object[i].y);
	Point P2(Object[i].x + Object[i].width, Object[i].y + Object[i].height);
	
	rectangle(RoI_Object, P1, P2, Scalar(0, 0, 255), 2);
	putText(RoI_Object, "Object", P1, FONT_HERSHEY_PLAIN, 1,  Scalar(0, 0, 255, 255), 2);
	dist_Object = (-0.07)*(P2.x-P1.x) + 102.597;
	
        ss.str(" ");
        ss.clear();
        ss<<"D = "<<dist_Object<<"cm";
        putText(RoI_Object, ss.str(), Point2f(1,130), 0,1, Scalar(0,0,255), 2);
	
    }
    
}

void Traffic_detection()
{
    if(!Traffic_Cascade.load("//home//pi//Desktop//Traffic_cascade.xml"))
    {
	printf("Unable to open traffic cascade file");
    }
    
    RoI_Traffic = frame_Traffic(Rect(200,0,200,140));
    cvtColor(RoI_Traffic, gray_Traffic, COLOR_RGB2GRAY);
    equalizeHist(gray_Traffic, gray_Traffic);
    Traffic_Cascade.detectMultiScale(gray_Traffic, Traffic);\
    
    dist_Traffic = 0;
    for(int i=0; i<Traffic.size(); i++)
    {
	Point P1(Traffic[i].x, Traffic[i].y);
	Point P2(Traffic[i].x + Traffic[i].width, Traffic[i].y + Traffic[i].height);
	
	rectangle(RoI_Traffic, P1, P2, Scalar(0, 0, 255), 2);
	putText(RoI_Traffic, "Traffic Light", P1, FONT_HERSHEY_PLAIN, 1,  Scalar(0, 0, 255, 255), 2);
	dist_Traffic = (-1.07)*(P2.x-P1.x) + 102.597;
	
        ss.str(" ");
        ss.clear();
        ss<<"D = "<<P2.x-P1.x<<"cm";
        putText(RoI_Traffic, ss.str(), Point2f(1,130), 0,1, Scalar(0,0,255), 2);
	
    }
    
}

void Delay(int time)
{
    clock_t now = clock();
    while(clock()-now<time){printf("lalalal\n");};
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
    
    Capture();
    Perspective();
    Threshold();
    Histrogram();
    LaneFinder();
    LaneCenter();
    Stop_detection();
    Object_detection();
    Traffic_detection();
    
    
// prioity I---stop sign, traffic, object, laneEnd 
    if (dist_Stop > 5 && dist_Stop < 200)//[][][]
    {
	digitalWrite(21, 0);
	digitalWrite(22, 0);    //decimal = 8
	digitalWrite(23, 0);
	digitalWrite(24, 1);
	
	cout<<"Stop Sign"<<endl;
	
	ss.str(" ");
        ss.clear();
        ss<<" Stop!";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
        stop_time = clock();	
	goto Stop_Sign;
    }


    if (dist_Object > 5 && dist_Object < 200)//[][][]
    {
	digitalWrite(21, 1);
	digitalWrite(22, 0);    //decimal = 9
	digitalWrite(23, 0);
	digitalWrite(24, 1);
	cout<<"Object"<<endl;
	dist_Object = 0;
	ss.str(" ");
        ss.clear();
        ss<<" Change Lane";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
        object_time = clock();
	goto Object;
    }
    

    if (dist_Traffic > 5 && dist_Traffic < 200)//[][][]
    {
	digitalWrite(21, 0);
	digitalWrite(22, 1);    //decimal = 10
	digitalWrite(23, 0);
	digitalWrite(24, 1);
	cout<<"Traffic"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<" Caution Traffic Light!";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	traffic_time = clock();
	goto Traffic;
    }

    if (laneEnd > 30000000)//[][][]
    {
       	digitalWrite(21, 1);
	digitalWrite(22, 1);    //decimal = 7
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Lane End"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<" Lane End";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(255,0,0), 2);
        laneend_time = clock();
        goto Traffic;
    } 

    
    //--stop
    if( clock()-stop_time < TIME_stop ) 
    {	
	ss.str(" ");
	   ss.clear();
	   ss<<" Stop";
	   putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	goto Stop_Sign;
    }
    else stop_time = -TIME_stop;

    //--object
    if( clock()-object_time < TIME_object ) 
    {	ss.str(" ");
	   ss.clear();
	   ss<<" Change Lane";
	   putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	goto Object;
    }
    else object_time = -TIME_object;


    //--traffic
    if( clock()-traffic_time < TIME_traffic) 
    {	ss.str(" ");
	   ss.clear();
	   ss<<" Caution Traffic Light!";
	   putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	goto Traffic;
    }
    else traffic_time = -TIME_traffic;


    //--lane end
    if( clock()-laneend_time < TIME_laneend) 
    {	ss.str(" ");
	   ss.clear();
	   ss<<" Lane End";
	   putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	goto Traffic;
    }
    else laneend_time = -TIME_laneend;


    // adjust the car to run near the center of the lane
    if (Result == 0)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 0);    //decimal = 0
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Forward"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result<<" Move Forward";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }
    
        
    else if (Result > 0 && Result <10)
    {
	digitalWrite(21, 1);
	digitalWrite(22, 0);    //decimal = 1
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Right1"<<endl;
	ss.str(" ");
        ss.clear();
	ss<<"Result = "<<Result<<" Move Right";
	putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }
    
    else if (Result >=10 && Result <20)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 1);    //decimal = 2
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Right2"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result<<" Move Right";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }
    
    else if (Result >20)
    {
	digitalWrite(21, 1);
	digitalWrite(22, 1);    //decimal = 3
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Right3"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result<<" Move Right";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }
    
    else if (Result <0 && Result >-10)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 0);    //decimal = 4
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Left1"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result<<" Move Left";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }
    
    else if (Result <=-10 && Result >-20)
    {
	digitalWrite(21, 1);
	digitalWrite(22, 0);    //decimal = 5
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Left2"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result<<" Move Left";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }
    
    else if (Result <-20)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 1);    //decimal = 6
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Left3"<<endl;
	ss.str(" ");
        ss.clear();
        ss<<"Result = "<<Result<<" Move Left";
        putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    }


    Stop_Sign:
    Traffic:   
    Object:
     
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
    
    namedWindow("Stop Sign", WINDOW_KEEPRATIO);
    moveWindow("Stop Sign", 900, 580);
    resizeWindow("Stop Sign", 640, 480);
    imshow("Stop Sign", RoI_Stop);
    
    namedWindow("Object", WINDOW_KEEPRATIO);
    moveWindow("Object", 640, 580);
    resizeWindow("Object", 640, 480);
    imshow("Object", RoI_Object);
    

    namedWindow("Traffic", WINDOW_KEEPRATIO);
    moveWindow("Traffic", 0, 580);
    resizeWindow("Traffic", 640, 480);
    imshow("Traffic", RoI_Traffic);
    
    waitKey(1);
    
    }
    
    return 0;
    
}
