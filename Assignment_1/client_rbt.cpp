#include <ros/ros.h>
#include <stdlib.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include <assignment1/ScanAction.h>


//This function is called once when the goal completes
void doneCb(const actionlib::SimpleClientGoalState& state, const assignment1::ScanResultConstPtr& result) {
    
	ROS_INFO("DONE!!!");
    ROS_INFO("FINISHED IN STATE [%s] ", state.toString().c_str());

	//printing the result of the detection (number of obstacles and their coordinates)
    ROS_INFO("Number of obstacles: %d", result->n_obstacles);  
    int j = 0;
   for(int i = 0; i < result->n_obstacles; i++){
		ROS_INFO("Obstacle %d Coordinates: %f, %f ", i, result->obst_coords[j], result->obst_coords[j+1]);
		j = j+2;
   }
    ros::shutdown();
}

//This function is called once when the goal becomes active
void activeCb(){
	ROS_INFO("ACTIVE");
}

//This function is called every time feedback is received for the goal
void feedbackCb(const assignment1::ScanFeedbackConstPtr& fb){
	//Called everytime robot moves
	if(fb->current_state ==1)
		ROS_INFO("GOAL ACCEPTED; ROBOT IS MOVING");

	//Called everytime robot receives a new goal
	if(fb->current_state ==2)
		ROS_INFO("CANCELLING GOAL BECAUSE ANOTHER GOAL WAS RECEIVED");

	//Called everytime robot reaches final destination POS_B
	if(fb->current_state ==3)
		ROS_INFO("ROBOT HAS REACHED THE FINAL DESTINATION...ROBOT HAS STOPPED");
	
	//Called if the goal is cancelled for invalid coordinates
	if(fb->current_state ==4)
		ROS_INFO("CANCELLED GOAL BECAUSE OF INVALID COORDINATES");

	//Called if the status is not one of the above
	if(fb->current_state ==-1)
		ROS_INFO("ERROR STATUS");
	
	//Called when detection starts
	if(fb->current_state ==10)
		ROS_INFO("DETECTION STARTED");
	
	//Called when detection finishes
	if(fb->current_state ==11)
		ROS_INFO("DETECTION FINISHED");

}





int main(int argc, char **argv){

	/**
	  Check input parameters number and initialization of POS_B (coordinates: xc, yc, zc; orientation: xc_o, yc_o, zc_o, wc_o)
	*/
	if(argc != 8){
		std::cout << "Not enough arguments, please insert 7; POS_B = (x, y, z, x_o, y_o, z_o, w_o)" << std::endl;
		return -1;
	} 

	/**
	  Initialization of POS_B (coordinates: xc, yc, zc; orientation: xc_o, yc_o, zc_o, wc_o)
	*/
	float xc = atof( argv[1] );
	float yc = atof( argv[2] );
	float zc = atof( argv[3] );
	float xc_o = atof(argv[4]);
	float yc_o = atof(argv[5]);
	float zc_o = atof(argv[6]);
	float wc_o = atof(argv[7]);
	
	/**
	  Client init and waiting for server to start
	*/
	ros::init(argc, argv, "test_client");
	actionlib::SimpleActionClient<assignment1::ScanAction> ac("scan", true);
	ROS_INFO("Waiting for action server to start.");
	ac.waitForServer();
	
	assignment1::ScanGoal goal;
	
	/**
	  it fills goal with info before sending it to server
	*/
	goal.x = xc;
	goal.y = yc;
	goal.z = zc;
	goal.or_x = xc_o;
	goal.or_y = yc_o;
	goal.or_z = zc_o;
	goal.or_w = wc_o;
	
	/**
	  it sends goal to server
	*/
	ac.sendGoal(goal, &doneCb, &activeCb, &feedbackCb);
	
	ros::spin();
	
	return 0;
}
