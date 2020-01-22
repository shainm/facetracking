#include<opencv2/opencv.hpp>
#include<conio.h>
#include <opencv2/core/ocl.hpp>
#include <opencv2/tracking.hpp>
#include <string> 
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

//Used to keep all the aspects of a tracked obj together
class Tracked {
		public:
		Rect2d face;
		Ptr<Tracker> tracker = TrackerMedianFlow::create();
		Scalar color = Scalar(rand() % 256, rand() % 256, rand() % 256);
	
};

int detection(Mat frame, vector<Tracked> *tracked) {
	//Grayscale of input image
	Mat gray;
	//Facial cascade file path
	string face_cascade_path = "haarcascade_frontalface_default.xml";
	CascadeClassifier face_cascade;
	vector<Rect> detectedFaces;
	Tracked track;
	int i, j;
	bool dup;

	//Load facial cascade
	if (!face_cascade.load(face_cascade_path)) {
		return 0;
	}

	//Convert to grayscale
	cvtColor(frame, gray, CV_BGR2GRAY);

	//Find and highlight faces on webcame frame
	/*                           (img,
										objects,
								haarcascade_frontalface_default					   scale factor,
															minneighbours,
															   flags                       )*/
	face_cascade.detectMultiScale(gray, detectedFaces, 1.3, 5, 0, Size(80, 80), Size(200, 200));
	//draw detection rectangle around faces
	for (i = 0; i < detectedFaces.size() && (*tracked).size() < 3; i++) {
		dup = false;
		for (j = 0; j < (*tracked).size(); j++){
			//if the center-point of a new face is found in a currently tracked face, skip it
			if (((detectedFaces.at(i).x + (detectedFaces.at(i).width/2))  > (*tracked).at(j).face.x) &&
				((detectedFaces.at(i).y + (detectedFaces.at(i).height/2)) > (*tracked).at(j).face.y) &&
				((detectedFaces.at(i).x + (detectedFaces.at(i).width/2))  < (*tracked).at(j).face.x + (*tracked).at(j).face.width) &&
				((detectedFaces.at(i).y + (detectedFaces.at(i).height/2)) < (*tracked).at(j).face.y + (*tracked).at(j).face.height)){
				dup = true;
				break;
			}
		}
		if (!dup){
			(*tracked).push_back(track);
			(*tracked).back().face = detectedFaces.at(i);
			
		}
	}
	return(1);
}

int main() {
	//Open webcam
	VideoCapture capWebcam(0);

	//produces error if the webcam cannot be accessed
	if (capWebcam.isOpened() == false) {
		cout << "error: capWebcam was not accessed successfully\n";
		_getch();
		return(0);
	}

	Mat frame; // input image
	vector<Rect2d> faces; //Faces detected and tracked
	vector<Tracked> tracked; //Template for new tracked objects
	char charCheckForEscKey = 0; //Exit character
	int i, numFaces, frameCounter = 0; //counter for For loops, number of faces inits already, regulate detection
    Mat gray;
	//loop while esc is not pressed and the webcam is accessible
	while (charCheckForEscKey != 27 && capWebcam.isOpened()) {
		frameCounter++;	
		//Exit loop and produce an error if the frame cannot be read
		if (!capWebcam.read(frame) || frame.empty()) {
			cout << "error: frame not read from webcam\n";
			_getch();
			return(0);
		}
		
		cvtColor(frame, gray, CV_BGR2GRAY);
		
		//if there are faces detected, track them
		if (!tracked.empty()){
			for (i = 0; i < tracked.size(); i++){
				//Draws a box around a tracked object
				if (tracked.at(i).tracker->update(frame, tracked.at(i).face)){
					rectangle(frame, tracked.at(i).face, tracked.at(i).color, 2, 1);
				//when an object is lost, remove it
				} else {
					tracked.erase(tracked.begin() + i);
				}
			}
		}
		
		//run the detection method
		if (!(frameCounter % 20)){
			frameCounter = 0;
			numFaces = (int) tracked.size();
			if (numFaces < 3) {
				//if statement fails if detection function could 
				//not load the facial cascade
				if (!detection(frame, &tracked)){
					cout << "error: loading facial cascade\n";
					_getch();
					return(0);
				}
				for (i = numFaces; i < tracked.size(); i++)
					tracked.at(i).tracker->init(frame, tracked.at(i).face);
			}
		}
		
		//Declare window then show it
		namedWindow("Face Tracking", CV_WINDOW_AUTOSIZE);
		imshow("Face Tracking", frame);

		//used to close the program by pressing Esc
		charCheckForEscKey = cv::waitKey(1);
	}
	return(0);
}