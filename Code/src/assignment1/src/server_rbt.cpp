#include <ros/ros.h>
#include <bits/stdc++.h>
#include "geometry_msgs/PoseStamped.h"
#include <stdlib.h>
#include <stdio.h>
#include <actionlib/server/simple_action_server.h>
#include <assignment1/ScanAction.h>
#include <actionlib_msgs/GoalStatusArray.h>
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/OccupancyGrid.h"
#include "nav_msgs/Odometry.h"

class ScanAction{
	protected: 
		// Create a ROS node handle for the server
    	ros::NodeHandle nh;
    	//node handle for interacting with geometry_msgs
    	ros::NodeHandle nh_;

		// Define the server 	
    	actionlib::SimpleActionServer<assignment1::ScanAction> as_;
		std::string action_name_;
		assignment1::ScanFeedback feedback_;
		assignment1::ScanResult result_;
		ros::Subscriber sub, sub_map, sub_scan, sub_odom;
		
		/*STATUS VARIABLES (USED FOR FEEDBACK)*/
		int state; 
		
		/*GOAL COORDINATES*/
		float x_goal, y_goal, z_goal;
		float x_orient_goal, y_orient_goal, z_orient_goal, w_orient_goal;
		
		/*ROBOT COORDINATES IN THE MAP*/
		double rbt_x = 0;
		double rbt_y = 0;
		double rbt_angle = 0; //angle (yaw)
		//quaternion variables
		double 	quat_x;
		double	quat_y;
		double	quat_z;
		double	quat_w;


		/*MAP PARAMETERS*/
		int map_width = 0; //width of the map
		int map_height = 0; //height of the map
		float map_origin_y = 0; // y origin of the wrf (World Reference Frame)
		float map_origin_x = 0; // x origin of the wrf (World Reference Frame)
		float map_resolution = 0; //resolution of the map
		bool map_ready = false; // when set to true this parameter indicates we can exploit the map
		std::vector<int> map; // we save once the whole map 
		
		/*DETECTION VARIABLES*/
		std::vector<std::tuple<float,float>> col_pos;//vector of the coordinates of the detected columns
		bool start_detection = false;
		int numObs = -1; //number of obstacles detected (setted to -1 to know if the action fails)
		bool success;
			
	public:
		/*constructor*/
		ScanAction(std::string name) : as_(nh, name, boost::bind(&ScanAction::executeS, this, _1), false),
		 action_name_(name), 
		 sub(nh.subscribe("/move_base/status", 1, &ScanAction::statusCallback, this)),
		 sub_map(nh.subscribe("/map", 1, &ScanAction::mapCallback, this)),
		 sub_scan(nh.subscribe("/scan", 1, &ScanAction::scannerCallback, this)),
		 sub_odom(nh.subscribe("/mobile_base_controller/odom", 1, &ScanAction::odomCallback, this))
		{ 
			as_.start(); //start the server
		}
		 
		 /*destructor*/
		~ScanAction(void){} 

		/*retrives the status of the robot*/
		void statusCallback(const actionlib_msgs::GoalStatusArray::ConstPtr& msg)   
		{
				/*status can be 
				* 1 = goal received and robot starting to move
				* 2 = goal cancelled because another goal was received
				* 3 = goal reached (at the beginning the goal is supposed to be reached so we have to be careful of false positive)
				* 4 = goal cancelled for invalid coordinates 
				* 10 = if the detection starts
				* 11 = if the detection ends*/
			
			if(!msg->status_list.empty()){
				// setting the status of the robot	
				for (const auto& goalStatus : msg->status_list)
				{
					state = goalStatus.status;
				}
			}
		}
		
		
		/*retrives the position of the robot x y angle in the wrf*/
		void odomCallback(const nav_msgs::Odometry::ConstPtr& msg) {
			
			geometry_msgs::PoseWithCovariance poseWithCovariance = msg->pose; //ignore the covariance and take just the pose
			geometry_msgs::Pose pose = poseWithCovariance.pose; //take the pose

			//save the position of the robot in the wrf
			rbt_x = pose.position.x;
			rbt_y = pose.position.y;

			//save the orientation of the robot in the wrf
			geometry_msgs::Quaternion orientation = pose.orientation;
			quat_x = orientation.x;
			quat_y = orientation.y;
			quat_z = orientation.z;
			quat_w = orientation.w;
			
			//convert quaternion to euler angles
			double siny = 2*(quat_w*quat_z + quat_x*quat_y);
			double cosy = 1 - 2*(quat_z*quat_z + quat_y*quat_y);
			rbt_angle = std::atan2(siny,cosy); //yaw angle
		}
		
		
		/*retrives the data from the laser scanner and detects obstacles*/
		void scannerCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
		{
			//start the detection only when the robot reaches the goal
			if(start_detection){ 
				if(map_ready){
					std::vector<std::tuple<float,float>> cartesian_values;	//temporary vector to keep track of all the points detected for each cluster
					std::vector<std::tuple<float,float>> column_pos;   //vector to store the mean of the points for each column table
					std::vector<std::vector<std::tuple<float,float>>> clusters; //clusters of points that are distant from each other
					
					//each measurement has an angle in radiants equal to the previous + increment + angle_min
					float increment = msg->angle_increment; 							
					float min_angle = msg->angle_min;									

					std::tuple<float,float> actual_point = std::make_tuple(0,0); //2d point in wrf
					bool to_drop = false; //a point has to drop if it is too close to the robot or if its range value is inf
					bool next_cluster = false; //a point has to be saved in a new cluster if it is too far from the previous one
					
					for(int i=0; i < msg->ranges.size(); i++){							//a single scan contains 666 elements 
						if(msg->ranges[i] < 0.2 || isinf(msg->ranges[i]))   //if a point is too close to the robot or if its range value is inf it has to be dropped
						{
							to_drop = true;
						}
						else{
							to_drop = false;
							actual_point = toEuclidean(msg->ranges[i],i*increment+min_angle); //a point eligible for a cluster is converted in 2d in wrf
						}
						if(cartesian_values.size() > 1)  
						{
							if(!isClose(cartesian_values[cartesian_values.size()-1],actual_point)) //this condition switches the next cluster based on a distance value
							{
								next_cluster = true;
								i--;
							}
						}
						if(to_drop == false && next_cluster == false)
						{
							cartesian_values.push_back(actual_point); //save a point in a cluster
						}

						//if this condition applies a cluster has to be saved and switch to the next one
						if((to_drop == true || next_cluster == true) && cartesian_values.size()>0 )  
						{
							to_drop = false;
							next_cluster = false;
							clusters.push_back(cartesian_values);
							cartesian_values.clear();					
						}
					}

					column_pos = columns_from_clusters(clusters); //screma :) (italian word for filter) the clusters

					//saving the coordinates of the columns in the wrf in a vector of tuples
					std::copy(column_pos.begin(), column_pos.end(), std::back_inserter(col_pos));
					col_pos.clear();
					for(int i = 0; i<column_pos.size(); i++)
					{
						col_pos.push_back(std::make_tuple(std::get<0>(column_pos[i]),std::get<1>(column_pos[i])));
					}
					start_detection = false;
				}
			}
		}
		
		/*saves the map in the memory of the node for future usage*/
		void mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg){
			if(!map_ready){ //save the map only once thanks to the /map topic
				map_width = msg->info.width;
				map_height = msg->info.height;
				map_resolution = msg->info.resolution;
				map_origin_y = msg->info.origin.position.y;
				map_origin_x = msg->info.origin.position.x;
				
				if(map_width>100){
					map_ready = true;
					for(int i = 0; i<map_width*map_height ;i++){
						map.push_back(msg->data[i]);
					}
				}
				map_ready = true;
				ROS_INFO("I HAVE SAVED THE MAP w %d h %d res %f", map_width, map_height, map_resolution);
			}
		}
		
		
		void executeS(const assignment1::ScanGoalConstPtr &goal){
			
			/*STARTING NAVIGATION TO POS_B*/
			// Create a ROS publisher that will publish a geometry_msgs::PoseStamped message on the /move_base_simple/goal topic to set the goal for the robot
 			ros::Publisher pub = nh_.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1); //Queue of 200 messages

			ROS_INFO("I HAVE RECEIVED A GOAL");
			//filling the variables with goal coordinates given by client request from action file
			x_goal = goal->x;
			y_goal = goal->y;
			z_goal = goal->z;
			//filling the variables with goal orientations given by client request from action file
			x_orient_goal = goal->or_x;
			y_orient_goal = goal->or_y;
			z_orient_goal = goal->or_z;
			w_orient_goal = goal->or_w;

			geometry_msgs::PoseStamped msg; //message to be published
			
			//assigning values to the message...
			msg.header.stamp = ros::Time::now();
			msg.header.frame_id = "map"; // coordinates are referred to the map frame		
			msg.pose.position.x = x_goal;
			msg.pose.position.y = y_goal;
			msg.pose.position.z = z_goal;
				
			msg.pose.orientation.x = x_orient_goal;
			msg.pose.orientation.y = y_orient_goal;
			msg.pose.orientation.z = z_orient_goal;
			msg.pose.orientation.w = w_orient_goal;
        		
        	ROS_INFO("I AM WAITING TO PUBLISH THE FINAL DESTINATION TO THE ROBOT");
        	while(pub.getNumSubscribers() == 0)
        	{
				ros::Duration(1.0).sleep();
			}
			
			//publishing the message
			pub.publish(msg);
			ROS_INFO("I HAVE SENT THE DESTINATION TO THE ROBOT");
				
			int time_iter = 0; //tmp variable to avoid infinite loops
			ROS_INFO("WAITING TO REACH THE DESTINATION...");
			while(state != 3 || (std::abs(rbt_x-x_goal) > 0.5 || std::abs(rbt_y-y_goal) > 0.5 )){ //to avoid false positives
				ros::Duration(1.0).sleep(); 
				feedback_.current_state = state;
				as_.publishFeedback(feedback_);
				time_iter++;

				//preempted check
				if(as_.isPreemptRequested() || !ros::ok() || time_iter > 120){
					ROS_INFO("%s: PREEMPTED", action_name_.c_str());
					as_.setPreempted();
					success = false;
				break;
				}
			}
			//the goal is now reached


			/*STARTING THE DETECTION...*/
			ROS_INFO("FINAL DESTINATION REACHED START THE DETECTION...");
			start_detection = true;
			feedback_.current_state = 10;  //set the status to 10 to notify the client that the detection has started
			as_.publishFeedback(feedback_);
			while(start_detection){
				ros::Duration(1.0).sleep();  //wait for the detection to finish
			}
			ROS_INFO("DETECTION FINISHED");
				
			feedback_.current_state = 11;  //set the status to 11 to notify the client that the detection has finished
			as_.publishFeedback(feedback_);

			//set the result of the action	
			result_.n_obstacles = col_pos.size();
			numObs = col_pos.size();
				

			/*CHECK IF ACTION SUCCEDED*/		
			if(numObs != -1)
				success = true;
			else{
				success = false;
				ROS_INFO("ACTION FAILED");
				as_.setAborted(result_);
			}
			if(success){
				std::vector<float> flat_data;
				for (int i=0;i<col_pos.size();i++){
					flat_data.push_back(std::get<0>(col_pos[i]));
					flat_data.push_back(std::get<1>(col_pos[i]));
					}
				result_.obst_coords = flat_data;
				as_.setSucceeded(result_);
			}
		}
		


		//DEFINE OTHER FUNCTIONS HERE
		private:					
			/*asses whethever a point is in the nighbourhood of a wall*/
			bool isWall(std::tuple<float,float> scan_point){
				float target_x = std::get<0>(scan_point);
				float target_y = std::get<1>(scan_point);
				int square_size  = 33; //size of the squred neighbourhood that will be explored
				int circular_map_index = 0; //tmp variable to explore the neighbourhood
				//convert from point coordinates to position in the map vector
				int map_index = (int)((target_y - map_origin_y) / map_resolution) * map_width +
								(int)((target_x - map_origin_x) / map_resolution);
				
				for(int i = -square_size/2; i<square_size/2; i++){ //explore in y
					for(int j = -square_size/2; j<square_size/2; j++){ //explore in x
						circular_map_index = map_index+i*map_width + j;
						if (circular_map_index >= 0 && circular_map_index < map_width * map_height) // verify if the index is valid
						{
							int cell_value = map[circular_map_index];

							// verify if cell value is a wall=100, (it can be wall = 100 free = 0 or unknown = -1)
							if (cell_value == 100)
							{
								return true; //if at least a neighbour is a wall the point is defined as a wall
							}
						}
					}
				}
				return false; //if no point in the neighbourhood is a wall the point is not a wall
			}



			/*returns the 2d position of a point in the wrf*/
			std::tuple<float,float> toEuclidean(double range,double angle){ 
				
				angle += rbt_angle;
				return std::make_tuple(range*cos(angle)+rbt_x,range*sin(angle)+rbt_y); //save a cartesian measurement of the point in the wrf
			}

			/*assesses whethever a point is close to another thanks to a threshold, this method is not very precise but sufficent for the current task*/
			bool isClose(std::tuple<float,float> pt1, std::tuple<float,float> pt2){
				double distance = pow(std::get<0>(pt1)-std::get<0>(pt2),2) + pow(std::get<1>(pt1)-std::get<1>(pt2),2); //euclidean 2d distance^2
				if(distance < 0.04) //distance <20 cm
					return true;
				return false;
			}

			/*this function takes as input a vector of tuples and returns the mean column-wise*/
			std::tuple<float,float> mean(std::vector<std::tuple<float,float>> vec){
				float sum1 = 0;
				float sum2 = 0;
				for(int i = 0; i<vec.size(); i++){
					sum1+= std::get<0>(vec[i]);
					sum2+= std::get<1>(vec[i]);
				}
				return std::make_tuple(sum1/vec.size(),sum2/vec.size());
			}

			/*this is a main function that asseses if a cluster of points is a wall/object closed to a wall or if it is a column table and returns just the position of the tables in the wrf*/
			std::vector<std::tuple<float,float>> columns_from_clusters(std::vector<std::vector<std::tuple<float,float>>> clusters){

				int pts_closed_to_walls = 0; //tmp variable to store the number of points closed to a wall in a cluster
				float percentage = 0.1; // a cluster is detected as non column table if the number of its points closed to a wall exceed this threshold
				int clusters_size= clusters.size();
				
				//remove clusters that are not column tables
				for(int i=clusters_size-1;i>=0;i--){
					pts_closed_to_walls = 0;
					for(int j = 0; j<clusters[i].size(); j++){
						if(isWall(clusters[i][j]))
							pts_closed_to_walls++;
						if(pts_closed_to_walls > clusters[i].size()*percentage){
							clusters.erase(clusters.begin()+i); //remove that cluster
							break; //exit from this cluster j = clusters[i].size();
						} 
					}
				}
				//save the column tables and return their centers in wrf
				std::vector<std::tuple<float,float>> columns_centers;
				for(int i=0;i<clusters.size();i++){
					columns_centers.push_back(mean(clusters[i]));
				}
				return columns_centers;
			}
};

int main(int argc, char* argv[])
{
  	// Laser node init
  	ros::init(argc, argv, "receiving_coord");
	
	// Create an object of class ScanAction that will take care of everything
	ScanAction scan("scan");

 	
  	//keep program alive
  	ros::spin();
  	
  	return 0;
}