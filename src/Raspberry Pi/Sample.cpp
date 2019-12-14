#include <opencv2/opencv.hpp>
#include <raspicam_cv.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>

using namespace std;
using namespace cv;
using namespace raspicam;

Mat frame;


 void Setup ( int argc,char **argv, RaspiCam_Cv &Camera )
  {
    Camera.set ( CAP_PROP_FRAME_WIDTH,  ( "-w",argc,argv,400 ) );
    Camera.set ( CAP_PROP_FRAME_HEIGHT,  ( "-h",argc,argv,240 ) );
    Camera.set ( CAP_PROP_BRIGHTNESS, ( "-br",argc,argv,50 ) );
    Camera.set ( CAP_PROP_CONTRAST ,( "-co",argc,argv,50 ) );
    Camera.set ( CAP_PROP_SATURATION,  ( "-sa",argc,argv,50 ) );
    Camera.set ( CAP_PROP_GAIN,  ( "-g",argc,argv ,50 ) ); 
    Camera.set ( CAP_PROP_FPS,  ( "-fps",argc,argv,0));

}


int main(int argc, char** argv)
{
    RaspiCam_Cv Camera;
    Setup(argc, argv, Camera);
    cout<<"Connecting to camera"<<endl;
    if (!Camera.open())
    {
		
	cout<<"Failed to Connect"<<endl;
    }
     
	cout<<"Camera Id = "<<Camera.getId()<<endl;
	
	for (int i=0; i<40; i++)
	{
		Camera.grab();
		Camera.retrieve(frame);
		cvtColor(frame,frame, COLOR_BGR2GRAY);
		imshow("Sample", frame);
		imwrite("Object"+to_string(i)+".jpg" , frame);
		waitKey();
	}
     
    return 0;
}
