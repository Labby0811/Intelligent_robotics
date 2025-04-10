#include "ros/ros.h"
#include "tiago_iaslab_simulation/Objs.h"
#include <cstdlib>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include <assignment1/ScanAction.h>
#include <assignment2/pickAndPlace.h>

//states if the action client of assignment 1 has done 
void doneCb(const actionlib::SimpleClientGoalState& state, const assignment1::ScanResultConstPtr& result) {
    ROS_INFO("DONE");
}
//states if the action client of assignment 1 is active
void activeCb(){
	ROS_INFO("ACTIVE");
}
//prints to the screen the feedback of the navigation server from assignment 1
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
}

//given a sequence of positions id to reach, this function returns the sequence of positions in the wrf
std::vector<float> getPosFromSeq(std::vector<int> sequence){
	std::vector<float> positions;

	for(int i = 0; i < sequence.size(); i++){ 
		switch(sequence[i]){
			case 0: //starting point
				positions.push_back(0); // x
				positions.push_back(0); // y
				positions.push_back(0); // z
				positions.push_back(0); // x_orienttation
				positions.push_back(0); // y_orienttation
				positions.push_back(0); // z_orienttation
				positions.push_back(1); // w_orienttation
				break;
			case 1: //pick blue
				positions.push_back(8.2);
				positions.push_back(-2);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 2: //pick green
				positions.push_back(7.65);
				positions.push_back(-4.15); //was -4
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0.6816);
				positions.push_back(0.731);
				break;
			case 3: //pick red
				positions.push_back(7.3);
				positions.push_back(-2);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 4: //place blue
				positions.push_back(12.4);
				positions.push_back(0.35);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 5: //place green
				positions.push_back(11.5);
				positions.push_back(-1);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0.6816);
				positions.push_back(0.731);
				break;
			case 6: //place red
				positions.push_back(10.6);
				positions.push_back(0.35);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 7: //waypoint
				positions.push_back(8.5);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 8: //waypoint
				positions.push_back(8.8);
				positions.push_back(-1);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 9: //waypoint
				positions.push_back(8.8);
				positions.push_back(-4);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(-0.713);
				positions.push_back(0.7);
				break;
			case 10: //waypoint
				positions.push_back(8.8);
				positions.push_back(-4);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(1);
				break;
			case 11: //waypoint
				positions.push_back(8.8);
				positions.push_back(-4);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0);
				positions.push_back(0.713);
				positions.push_back(0.7);
				break;
			default:
				break;
		}
	}

	return positions;
}
//given an actual id position and a position to go to, this function returns a sequence of id positions to passthrough to reach the destination
std::vector<int> posSeq(int actual, int next){
	std::vector<int> sequence;
    //hard-coded waypoints
	if(actual == 0 && next == 1){
		sequence.push_back(7);
		sequence.push_back(1);
	}else if(actual == 0 && next == 2){
		sequence.push_back(7);
		sequence.push_back(8);
		sequence.push_back(9);
		sequence.push_back(2);
	}else if(actual == 0 && next == 3){
		sequence.push_back(7);
		sequence.push_back(3);
	}else if(actual == 1 && next == 4){
		sequence.push_back(4);
	}else if(actual == 2 && next == 5){
		sequence.push_back(10);
		sequence.push_back(5);
	}else if(actual == 3 && next == 6){
		sequence.push_back(6);
	}else if(actual == 4 && next == 2){
		sequence.push_back(7);
		sequence.push_back(8);
		sequence.push_back(9);
		sequence.push_back(2);
	}else if(actual == 4 && next == 3){
		sequence.push_back(7);
		sequence.push_back(3);
	}else if(actual == 5 && next == 1){
		sequence.push_back(11);
		sequence.push_back(8);
		sequence.push_back(1);
	}else if(actual == 5 && next == 3){
		sequence.push_back(11);
		sequence.push_back(8);
		sequence.push_back(3);
	}else if(actual == 6 && next == 1){
		sequence.push_back(7);
		sequence.push_back(1);
	}else if(actual == 6 && next == 2){
		sequence.push_back(7);
		sequence.push_back(8);
		sequence.push_back(9);
		sequence.push_back(2);
	}else if(actual == 1 && next == 0){
		sequence.push_back(7);
		sequence.push_back(0);
	}else if(actual == 2 && next == 0){
		sequence.push_back(10);
		sequence.push_back(8);
		sequence.push_back(0);
	}else if(actual == 3 && next == 0){
		sequence.push_back(7);
		sequence.push_back(0);
	}else if(actual == 4 && next == 0){
		sequence.push_back(0);
	}else if(actual == 5 && next == 0){
		sequence.push_back(11);
		sequence.push_back(8);
		sequence.push_back(7);
		sequence.push_back(0);
	}else if(actual == 6 && next == 0){
		sequence.push_back(0);
	}else if(actual == 0 && next == 5){
		sequence.push_back(7);
		sequence.push_back(8);
		sequence.push_back(10);
		sequence.push_back(5);
	}else if(actual == 0 && next == 8){
		sequence.push_back(7);
		sequence.push_back(8);
	}else if(actual == 2 && next == 1){
		sequence.push_back(10);
		sequence.push_back(8);
		sequence.push_back(1);
	}else{
		ROS_ERROR("INVALID SEQUENCE OF POSITIONS");
	} 
	return sequence;
}
//this function, given an actual position and a destination(next) makes the robot go to the destination as a client for the action server of assignment1 and returns the new actual position
int makeRobotGo(int actual, int next){
	if(actual != next){													//allow for standing in the current position when actual = next
		std::vector<int> sequence = posSeq(actual, next);  
		std::vector<float> positions = getPosFromSeq(sequence);
		
		//ROS_INFO("POSITIONS SIZE %ld",positions.size());
		actionlib::SimpleActionClient<assignment1::ScanAction> ac("scan", true);// connect to the action servere of assignment1
		ROS_INFO("Waiting for action server to start.");				
		ac.waitForServer();	
		for(int i = 0; ros::ok() && i<positions.size(); i = i+7)		//from the first waypoint to the destination
		{
			assignment1::ScanGoal goal;									//create a new goal position
			
            //fill the goal position with the values from the vector
			goal.x =     positions[i];
			goal.y =     positions[i+1];
			goal.z =     positions[i+2];
			goal.or_x =  positions[i+3];
			goal.or_y =  positions[i+4];
			goal.or_z =  positions[i+5];
			goal.or_w =  positions[i+6];

			
			ac.sendGoal(goal, &doneCb, &activeCb, &feedbackCb);			//send the actual goal position
			ROS_INFO(" I HAVE SENT THE GOAL %f %f %f",goal.x,goal.y,goal.z);
			ac.waitForResult();										    //wait until the robot has reached the destination or waypoint
			ac.getResult();												//flush the result
			ROS_INFO(" I HAVE REACHED THE GOAL");
		}
	}
	return next;														//return the actual position of the robot
}



int main(int argc, char **argv)
{
	ros::init(argc, argv, "Assistant");

	ros::NodeHandle n;
	ros::ServiceClient client = n.serviceClient<tiago_iaslab_simulation::Objs>("/human_objects_srv"); //linking to the human service
	tiago_iaslab_simulation::Objs srv; 									//variable to access the service
	
	std::vector<int> obj_ids;
	int current_obj = 0; //not used yet
	int actual = 0;                                                     //actual position of the robot: at the beginning is supposed to be the origin 0 0 0 0 0 0 1

	srv.request.ready = true;											//request the order of the pick and place routine to the human node
	srv.request.all_objs = true;										//for all the three objects
	while (ros::ok() && !client.call(srv)) {			
        ROS_WARN("WAITNG THE HUMAN TO GIVE INSTRUCTIONS...");
        ros::Duration(1.0).sleep(); 
    }

	if (client.call(srv))												//save the order of the pick and place routine sent from the human node
	{
		obj_ids.push_back(srv.response.ids[0]);
		obj_ids.push_back(srv.response.ids[1]);
		obj_ids.push_back(srv.response.ids[2]);
	}
	else
	{
		ROS_ERROR("FAILED TO CALL THE HUMAN SERVICE");
	}

	ros::NodeHandle n1;
	ros::ServiceClient client_pick_and_place = n1.serviceClient<assignment2::pickAndPlace>("/human_objects_srv"); //linking to the pick and place service
	assignment2::pickAndPlace srv_pap;
	
    //----pick and place routine-----
	for(int i = 0; i< obj_ids.size(); i++){
		actual = makeRobotGo(actual,obj_ids[i]);
		
        //pick from table
		srv_pap.request.ready = true;
		srv_pap.request.action = obj_ids[i];
		
		while (ros::ok() && !client_pick_and_place.call(srv_pap)) {			
			ROS_WARN("WAITNG THE PICK AND PLACE SERVICE TO BE DONE");
			ros::Duration(1.0).sleep(); 
		}
		if(srv_pap.response.status != true){
			ROS_ERROR("NOT COMPLETED PICK AND PLACE CORRECTLY");
			return 0;
		}
		//go to the column
		if(actual == 1){
			actual = makeRobotGo(actual,4);
		}
		else if(actual == 2){
			actual = makeRobotGo(actual,5);
		}
		else if(actual == 3){
			actual = makeRobotGo(actual,6);
		}
		//place on the column
		srv_pap.request.ready = true;
		srv_pap.request.action = obj_ids[i]+3;
		
		while (ros::ok() && !client_pick_and_place.call(srv_pap)) {			//wait until the pick and place service has completed the action
			ROS_WARN("WAITNG THE PICK AND PLACE SERVICE TO BE DONE");       //if the service is not done, wait 1 second and try again
			ros::Duration(1.0).sleep(); 
		}
		if(srv_pap.response.status != true){                                   //if the service has not completed the action correctly, return 0
			ROS_ERROR("NOT COMPLETED PICK AND PLACE CORRECTLY");
			return 0;
		}
	}
	makeRobotGo(actual,0); //go back to the origin

	
	ROS_INFO("TERMINATED");
	return 0;
}
