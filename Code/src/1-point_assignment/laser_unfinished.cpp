#include <ros/ros.h>
#include<bits/stdc++.h>
#include <vector>
#include "sensor_msgs/LaserScan.h"
#include <utility>


//converts polar coordinates to cartesian coordinates
std::pair<float,float> polarToCartesian(const float& r,const float& theta) 
{   
    // Calculate Cartesian coordinates
    float x = r * cos(theta);
    float y = r * sin(theta);
    
    return std::make_pair(x,y);
}

//finds the median coordinate
std::pair<float,float> findMedian(const std::vector<std::pair<float,float>>& coordinates) 
{
    float x = 0;
    float y = 0;

    for (int i = 0; i < coordinates.size(); i++) {
        x += coordinates[i].first;
        y += coordinates[i].second;
    }
    x = x / coordinates.size();
    y = y / coordinates.size();

    return std::make_pair(x,y);
}

//finds closest foot for each feet and then calculating the median of the two feet to find the person position
std::pair<float,float> calcPersonPosition(const std::vector<std::pair<float,float>>& points_vector, const std::pair<float,float>& current_point)  
{
    float min = 1000000;
    float x = 0;
    float y = 0;
    int closest_index = 0;

    for (int i = 0; i < points_vector.size(); i++) {

        float distance = sqrt(pow(points_vector[i].first - current_point.first, 2) + pow(points_vector[i].second- current_point.second, 2)); //calculate distance between two feet

        if (distance < min) {
            min = distance;
            x = points_vector[i].first;
            y = points_vector[i].second;
            closest_index = i;
        }
    }

    std::pair<float,float> personPosition = {(current_point.first + x) / 2, (current_point.second + y) / 2}; //calculate median of two feet
    return personPosition;
}


void elaborateCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    int i = 0;
    const int MAX_i = msg->ranges.size(); //max number of elements in a single scan

    std::vector<std::pair<float,float>> coordinates = {}; //vector of coordinates of people
    std::vector<std::pair<float,float>> median_foot = {}; //vector of median feet coordinates
    std::vector<std::pair<float,float>> people = {};   //vector of people coordinates

    float increment = msg->angle_increment; //angle increment between two consecutive measures

    int feet = 0; //number of feet detected
    
    while(i < MAX_i)
    {
        while(isinf(msg->ranges[i])) //search for first foot by removing inf values
        { 
            if(i >= MAX_i) //if no other foot was found
            {
                ROS_INFO("No other foot was found, finished");
                break;
            }
            i++;
        }

        if(!isinf(msg->ranges[i])) //if it is not inf, it means that the first foot was found
        {
            feet++;
            int j = 0;  //counter for detected points

            while(!isinf(msg->ranges[i]))   //picking the first foot values and converting them to cartesian coordinates
            {
                if(i >= MAX_i)
                {
                    ROS_INFO("Finished scanning");
                    break;
                }
                coordinates.push_back(polarToCartesian(msg->ranges[i], increment*i)); //convert polar coordinates to cartesian coordinates
                ROS_INFO("foot number %d points found at (%lf , %lf)", feet, coordinates[j].first, coordinates[j].second); //print coordinates of foot, j is the current number of points detected
                j++;
                i++;
            }

            median_foot.push_back(findMedian(coordinates)); //find median of coordinates of first foot
            ROS_INFO("foot number %d coordinates at (%lf , %lf)", feet, median_foot[feet].first, median_foot[feet].second); 

            coordinates.clear(); //clear vector of coordinates of people
        }
        else
        {
            ROS_INFO("No other foot was found");
            break;
        }
    }

    //find closest foot for each feet and then calculating the median of the two feet to find the person position
    if(feet % 2 != 0) //if number of feet is odd, it means that one person's foot was unpon the 0 angle and so it was seen as two feet
    {
        //in this case we have to merge this two detected feet in one foot and then proceed as usual
        
    }
        

    for(int i = 0; i < median_foot.size(); i++)
    {    
        std::pair<float,float> current = median_foot[i];
        median_foot.erase(median_foot.begin() + i); //remove current foot from vector of feet
        people.push_back(calcPersonPosition(median_foot, current)); //find closest foot to the median   
    }

    //print people coordinates
    for(int i = 0; i < people.size(); i++) 
        ROS_INFO("Person number %d coordinates at (%lf , %lf)", i, people[i].first, people[i].second); 
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

