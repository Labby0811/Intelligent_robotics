#include <ros/ros.h>
#include<bits/stdc++.h>
#include <vector>
#include "sensor_msgs/LaserScan.h"
#include <math.h>
//this function takes as input a vector of tuples and returns the mean column-wise
std::tuple<float,float> mean(std::vector<std::tuple<float,float>> vec){
	float sum1 = 0;
	float sum2 = 0;
	for(int i = 0; i<vec.size(); i++){
		sum1+= std::get<0>(vec[i]);
		sum2+= std::get<1>(vec[i]);
	}
	return std::make_tuple(sum1/vec.size(),sum2/vec.size());
}
//function that takes as input the feet positions  and spots the pairs of feet belonging to the same person
//the assumption here is that a person is always rapresented by 2 feet so it
//the idea is that the the two closest feet belong to the same person
std::vector<std::tuple<float,float>> getPeopleFromFeet(std::vector<std::tuple<float,float>> vec){
	float current_best = 100; 											//this value keeps track of the closest distance between a foot and the others, the value 100 is good since a person can't spread the legs that much :)
	int best_index = 1;													//indx of the closest foot
	std::vector<std::tuple<float,float>> people;						//vector that will contain the cartesian x,y pos of the people around the sensor
	int i = 0;													
	while(!vec.empty()){												//after a person has been detected his feet will be removed until all people will be found
		for(int j = 1;j<vec.size();j++){								//find all the possible matches 
			if(pow((std::get<0>(vec[i])-std::get<0>(vec[j])),2)<current_best){ //if the number of initial feet is odd, this means that a foot has been detected as two different feet because it was poitioned along the x-axis so we have two merge tese two virtual feet
				best_index = j;
				current_best = pow((std::get<0>(vec[i])-std::get<0>(vec[j])),2);
			}
		}
		if(vec.size()%2!=0)												//if the number of initial feet is odd, this means that a foot has been detected as two different feet because it was poitioned along the x-axis so we have two merge tese two virtual feet
		{
			vec.push_back(std::make_tuple((std::get<0>(vec[i])+std::get<0>(vec[best_index]))/2, (std::get<1>(vec[i])+std::get<1>(vec[best_index]))/2));
			vec.erase(vec.begin() + best_index); 
			vec.erase(vec.begin());
			continue;
		}
			
		people.push_back(std::make_tuple((std::get<0>(vec[i])+std::get<0>(vec[best_index]))/2, (std::get<1>(vec[i])+std::get<1>(vec[best_index]))/2));//each person has the position tha is the mean of his feet
		vec.erase(vec.begin() + best_index); 							//remove the person's feet
		vec.erase(vec.begin());    
	}
	return people;
}
//function for printing a vector of 2-values-tuple to screen
void print(std::vector<std::tuple<float,float>> vec, char* s)
{
	ROS_INFO("%s",s);
	for(int i=0; i<vec.size(); i++){
		ROS_INFO("%lf   %lf",std::get<0>(vec[i]),std::get<1>(vec[i]));
	}
}

//Callback function	
void elaborateCallback(const sensor_msgs::LaserScan::ConstPtr& msg){
	
	std::vector<std::tuple<float,float>> cartesian_values; 				//temporary vector to keep track of all the points detected for each foot
	std::vector<std::tuple<float,float>> feet_pos;                      //vector to store the mean of the points for each foot
	std::vector<std::tuple<float,float>> people_pos; 					//final array where the x y cartesian position of each person will be stored
	float increment = msg->angle_increment; 							//each measurement has an angle in radiants equal to the previous + increment
	
	for(int i=0; i < msg->ranges.size(); i++){							//a single scan contains 720 elements
		if(!isinf(msg->ranges[i])){
				while(!isinf(msg->ranges[i]) && i < msg->ranges.size()){//record measurements for a foot 
					cartesian_values.push_back(std::make_tuple((msg->ranges[i])*cos((msg->angle_increment) * i), (msg->ranges[i])*sin((msg->angle_increment) * i))); //save a cartesian measurement of the second foot
					i++;
				}
				feet_pos.push_back(mean(cartesian_values)); 			//store just the mean of the points for each foot  and clear all cartesian values
				cartesian_values.clear();												
		}
	}
	
	people_pos = getPeopleFromFeet(feet_pos);							//convert feet positions in people positions
	
	//printing the result
	ROS_INFO("Number of detected people: %ld", people_pos.size());
	print(people_pos,"people positions: ");
	
}
int main(int argc, char* argv[])
{
  // Laser node init
  ros::init(argc, argv, "vision_node");

  // Create a ROS node handle
  ros::NodeHandle nh;

  ROS_INFO("Detecting people... ");
  
  ros::Subscriber sub = nh.subscribe("/scan", 4000, elaborateCallback);
  
  //keep program alive
  ros::spin();
}
