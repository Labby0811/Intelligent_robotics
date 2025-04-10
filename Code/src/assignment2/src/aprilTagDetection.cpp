#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <apriltag_ros/AprilTagDetectionArray.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include "assignment2/tags.h"

int iteration = 0;  //to count the number of iterations
geometry_msgs::PoseStamped output[8];

geometry_msgs::PoseStamped transformObjectCoordinates(const geometry_msgs::PoseStamped& object_pose, const std::string& source_frame, const std::string& target_frame) 
{
    tf2_ros::Buffer tfBuffer;
    tf2_ros::TransformListener tfListener(tfBuffer);

    // Adjusted loop rate
    ros::Rate rate(1.0);  // Set an appropriate loop rate

    while (ros::ok()) {
        try {
            geometry_msgs::TransformStamped transformStamped = tfBuffer.lookupTransform(target_frame, source_frame, ros::Time(0));

            // Perform transformation: Object in target frame
            geometry_msgs::PoseStamped object_in_target_frame;
            tf2::doTransform(object_pose, object_in_target_frame, transformStamped);
            return object_in_target_frame;
        }
        catch (tf2::TransformException& ex) {
            // Handle the exception as needed
        }

        rate.sleep();
    }

    // Return an empty PoseStamped if the transformation fails after some attempts
    return geometry_msgs::PoseStamped();
}

// Original callback function with modifications
void callbackFunction(const apriltag_ros::AprilTagDetectionArray::ConstPtr& msg) {
    if (!msg->detections.empty()) {
        geometry_msgs::PoseWithCovarianceStamped poseWithCovarianceStamped = msg->detections[0].pose;

        geometry_msgs::PoseStamped object_in_camera_frame;
        object_in_camera_frame.pose = poseWithCovarianceStamped.pose.pose;
        object_in_camera_frame.header.stamp = ros::Time(0);
        object_in_camera_frame.header.frame_id = "/xtion_rgb_optical_frame";

        ROS_INFO("------------------NEW ITERATION--------------------");
        iteration++;
        
		int ID = msg->detections[0].id[0];
        ROS_INFO("ID = %d", msg->detections[0].id[0]);

        // Call the transformation function and retrieve the transformed PoseStamped
        geometry_msgs::PoseStamped object_in_world_frame = transformObjectCoordinates(object_in_camera_frame, "xtion_rgb_optical_frame", "odom");
		
        if (!object_in_world_frame.header.frame_id.empty()) {
            // Log the transformed pose
            ROS_INFO("Object Pose in World Frame:");
            ROS_INFO("Orientation: (%f, %f, %f, %f)", object_in_world_frame.pose.orientation.x,
                     object_in_world_frame.pose.orientation.y, object_in_world_frame.pose.orientation.z,
                     object_in_world_frame.pose.orientation.w);
            ROS_INFO("Position: (%f, %f, %f)", object_in_world_frame.pose.position.x,
                     object_in_world_frame.pose.position.y, object_in_world_frame.pose.position.z);
            if(output[ID] == geometry_msgs::PoseStamped())
				output[ID] = object_in_world_frame;
        } 
        else {
            ROS_WARN("Transformation failed. Object Pose in World Frame not available.");
        }
    } 
    else {
        ROS_INFO("TAG NOT FOUND");
    }
}

bool coordinates(assignment2::tags::Request &req, assignment2::tags::Response &res)
{
	bool start_detection = req.start;
	if(start_detection)
	{
		ros::NodeHandle n;
		ros::Subscriber sub = n.subscribe("/tag_detections", 200, callbackFunction);
		while(iteration < 15)
			ros::spinOnce();
		for(int i = 0; i < 8; i++)
		{
			ROS_INFO("-----------------array [%d] =  ", i);
			ROS_INFO("Orientation: (%f, %f, %f, %f)", output[i].pose.orientation.x,
                     output[i].pose.orientation.y, output[i].pose.orientation.z,
                     output[i].pose.orientation.w);
            ROS_INFO("Position: (%f, %f, %f)", output[i].pose.position.x,
                     output[i].pose.position.y, output[i].pose.position.z);
		}
		for(int i = 0; i < 8; i++)
			res.poses.push_back(output[i]); 
	}
	return true; 
}


int main(int argc, char **argv) {
    ros::init(argc, argv, "recharging_station");
    ros::NodeHandle nh_local;
	ros::ServiceServer service = nh_local.advertiseService("tag_coord", coordinates);
    ros::spin();
    return 0;
}
