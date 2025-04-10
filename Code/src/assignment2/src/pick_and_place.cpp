#include <ros/ros.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit/move_group_interface/move_group_interface.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <control_msgs/FollowJointTrajectoryAction.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h> 
#include <assignment2/pickAndPlace.h>
#include <string>
#include <map>
#include <vector>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <tf2_ros/transform_listener.h>
#include "assignment2/tags.h"
#include <gazebo_ros_link_attacher/Attach.h>

// Function to virtually attach the object to the gripper
bool attachGripper(int target_id)
{
	ros::NodeHandle nh1; // Create a node handle for the link_attacher_node service
	ros::ServiceClient attach_client = nh1.serviceClient<gazebo_ros_link_attacher::Attach>("/link_attacher_node/attach");
	gazebo_ros_link_attacher::Attach attach_srv;  // Create an object of the Attach service
	// Set the names of the link to be attached and the object to be attached
	if(target_id == 1) 
	{
		attach_srv.request.model_name_1 = "Hexagon";        // If ID == 1 then attach the blue hexagon
		attach_srv.request.link_name_1 = "Hexagon_link";
	}
	else if(target_id == 2) 
	{
		attach_srv.request.model_name_1 = "Triangle";    // If ID == 2 then attach the green triangle
		attach_srv.request.link_name_1 = "Triangle_link";
	}
	else if(target_id == 3) 
	{
		attach_srv.request.model_name_1 = "cube";    // If ID == 3 then attach the red cube
		attach_srv.request.link_name_1 = "cube_link";
	}
	attach_srv.request.model_name_2 = "tiago";
	attach_srv.request.link_name_2 = "arm_7_link";
	// Call the link_attacher service
	if(attach_client.call(attach_srv))
	{
		ROS_INFO("Object successfully attached to the gripper");
		return true;
	}
	else
	{
		ROS_INFO("Failed to attach the object to the gripper");
		return false;
	}
}
// Function to detach the object from the gripper
bool detachGripper(int target_id)
{
	ros::NodeHandle nh1;  // Create a node handle for the link_detacher_node service
	ros::ServiceClient detach_client = nh1.serviceClient<gazebo_ros_link_attacher::Attach>("/link_attacher_node/detach");  // Create a client for the link_detacher_node service
	gazebo_ros_link_attacher::Attach detach_srv;
	// Set the names of the link to be attached and the object to be attached
	if(target_id == 1) 
	{
		detach_srv.request.model_name_1 = "Hexagon";    // If ID == 1 then detach the blue hexagon
		detach_srv.request.link_name_1 = "Hexagon_link";
	}
	else if(target_id == 2) 
	{
		detach_srv.request.model_name_1 = "Triangle";    // If ID == 2 then detach the green triangle
		detach_srv.request.link_name_1 = "Triangle_link";
	}
	else if(target_id  == 3) 
	{
		detach_srv.request.model_name_1 = "cube";        // If ID == 3 then detach the red cube
		detach_srv.request.link_name_1 = "cube_link";
	}
	detach_srv.request.model_name_2 = "tiago";
	detach_srv.request.link_name_2 = "arm_7_link";
	// Call the link_attacher service
	if(detach_client.call(detach_srv))
	{
		ROS_INFO("Object successfully detached to the gripper");
		return true;
	}
	else
	{
		ROS_INFO("Failed to detach the object to the gripper");
		return false;
	}
}


// Function to transform the coordinates of the object from the source frame to the target frame
geometry_msgs::PoseStamped transformObjectCoordinates(const geometry_msgs::PoseStamped& object_pose, const std::string& source_frame, const std::string& target_frame) 
{
    tf2_ros::Buffer tfBuffer;
    tf2_ros::TransformListener tfListener(tfBuffer);

    // Adjusted loop rate
    ros::Rate rate(1.0);  // Set an appropriate loop rate

    while (ros::ok()) {
        try {
            geometry_msgs::TransformStamped transformStamped = tfBuffer.lookupTransform(target_frame, source_frame, ros::Time(0));  // Get the transformation from source frame to target frame

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


//opening the gripper
void openGripper(){
	//connect to topic for the gripper
	actionlib::SimpleActionClient<control_msgs::FollowJointTrajectoryAction> gripper_client("/parallel_gripper_controller/follow_joint_trajectory", true);
	gripper_client.waitForServer();
	control_msgs::FollowJointTrajectoryGoal gripper_goal;
	gripper_goal.trajectory.joint_names.push_back("gripper_finger_joint");
	trajectory_msgs::JointTrajectoryPoint open;
	//ask to open the gripper
	open.positions.push_back(1.0);
	open.time_from_start = ros::Duration(1.0);
	gripper_goal.trajectory.points.push_back(open);
	
	gripper_client.sendGoal(gripper_goal);
	gripper_client.waitForResult(ros::Duration(5.0));
}

//close the gripper
void closeGripper(){
	//connect to topic for the gripper
	actionlib::SimpleActionClient<control_msgs::FollowJointTrajectoryAction> gripper_client("/parallel_gripper_controller/follow_joint_trajectory", true);
	gripper_client.waitForServer();
	control_msgs::FollowJointTrajectoryGoal gripper_goal;
	gripper_goal.trajectory.joint_names.push_back("gripper_finger_joint");
	trajectory_msgs::JointTrajectoryPoint close;
	//asks to close the gripper
	close.positions.push_back(0.0);
	close.time_from_start = ros::Duration(1.0);
	gripper_goal.trajectory.points.push_back(close);
	
	gripper_client.sendGoal(gripper_goal);
	gripper_client.waitForResult(ros::Duration(5.0));
}

//defining safe position for the arm
void safePosition(moveit::planning_interface::MoveGroupInterface& group_arm_torso) {
    std::map<std::string, double> target_position;
    // defining joints position to reach object
    target_position["torso_lift_joint"] = 0.150;
    target_position["arm_1_joint"] = 0.2;
    target_position["arm_2_joint"] = -1.3;
    target_position["arm_3_joint"] = -0.2;
    target_position["arm_4_joint"] = 1.9;
    target_position["arm_5_joint"] = -1.6;
    target_position["arm_6_joint"] = 1.4;
    target_position["arm_7_joint"] = 0;

    // applying position to Tiago's arm
    std::vector<std::string> torso_arm_joint_names;
    torso_arm_joint_names = group_arm_torso.getJoints();
    group_arm_torso.setStartStateToCurrentState();
    group_arm_torso.setMaxVelocityScalingFactor(1.0);

    // set the target position for each joint
    for (const auto& joint_name : torso_arm_joint_names) {  
        if (target_position.count(joint_name) > 0) {
            group_arm_torso.setJointValueTarget(joint_name, target_position[joint_name]);
        }
    }

    // execute the planned motion
    ROS_INFO("the problem is here for the safe position 02");
    group_arm_torso.move();
    ROS_INFO("the problem is here for the safe position 03");
}


//setting an intermediate position for the arm in order to avoid collision with the table and make the picking easier
void intermediatePose(moveit::planning_interface::MoveGroupInterface& group_arm_torso){
	std::map<std::string, double> target_position;
	
    //defining joints position to reach object
	target_position["torso_lift_joint"] = 0.182;
	target_position["arm_1_joint"] = 0.244346;
	target_position["arm_2_joint"] = 0.610865;
    target_position["arm_3_joint"] = -3.24631;
    target_position["arm_4_joint"] = 1.46608;
    target_position["arm_5_joint"] = -1.18682;
    target_position["arm_6_joint"] = -0.122173;
    target_position["arm_7_joint"] = -1.309;

	//applicating position to tiago's arm
	 std::vector<std::string> torso_arm_joint_names;
	torso_arm_joint_names = group_arm_torso.getJoints();
	group_arm_torso.setStartStateToCurrentState();   
	group_arm_torso.setMaxVelocityScalingFactor(1.0);  // Set the max velocity scaling factor
	for (unsigned int i = 0; i < torso_arm_joint_names.size(); ++i)
		 if ( target_position.count(torso_arm_joint_names[i]) > 0 ){
			group_arm_torso.setJointValueTarget(torso_arm_joint_names[i], target_position[torso_arm_joint_names[i]]);
		 }

	//prepare moveit plan and execution
	 moveit::planning_interface::MoveGroupInterface::Plan my_plan1; 
     group_arm_torso.setPlanningTime(5.0);
     group_arm_torso.plan(my_plan1);
	ros::Time start = ros::Time::now();
	 group_arm_torso.move();
	 

}

//position to place objects on cylinders (hardcoded)
void placePosition(moveit::planning_interface::MoveGroupInterface& group_arm_torso, int id){
	geometry_msgs::PoseStamped cylinder;
	tf2::Quaternion orientation;
	
	cylinder.header.frame_id = "odom";
	if(id == 4) //blue
		cylinder.pose.position.x = 12.55;
	if(id == 5) //green
		cylinder.pose.position.x = 11.55;
	if(id == 6) //red
		cylinder.pose.position.x = 10.55;	
	cylinder.pose.position.y = -0.365;
	cylinder.pose.position.z = 0.85;
	
	orientation.setRPY(M_PI/2,0,0);  //setting orientation of the gripper
	cylinder.pose.orientation = tf2::toMsg(orientation);
	
    //transforming coordinates from odom to base_footprint in order to aim to the correct coordinates with the arm
	cylinder = transformObjectCoordinates(cylinder, "odom", "base_footprint");  
	cylinder.header.frame_id = "base_footprint";
	
    //setting pose target for the arm
	group_arm_torso.setPoseTarget(cylinder);
	
    //prepare moveit plan and execution
	moveit::planning_interface::MoveGroupInterface::Plan my_plan2;
	group_arm_torso.setPlanningTime(30.0);
	group_arm_torso.plan(my_plan2);
	ros::Time start = ros::Time::now();
	
	group_arm_torso.move();
	
	detachGripper(id-3);  //detach the object from the gripper
	openGripper();
	
	safePosition(group_arm_torso);
	
}

//to remove collision object when picked
void removeObst(std::string id_o, moveit::planning_interface::PlanningSceneInterface& scene_plan){
	moveit_msgs::CollisionObject remove_object;
	remove_object.id = id_o;
	remove_object.operation = remove_object.REMOVE;
	scene_plan.applyCollisionObject(remove_object);
}

//pick function
void pick(moveit::planning_interface::MoveGroupInterface& group_arm_torso, std::vector<geometry_msgs::PoseStamped> aprilTagInfo, int id, moveit::planning_interface::PlanningSceneInterface& scene_plan){
	openGripper(); //to remove when added the place procedure
	intermediatePose(group_arm_torso);  //set intermediate pose
	geometry_msgs::PoseStamped toBaseFootprint;
	
	toBaseFootprint = (transformObjectCoordinates(aprilTagInfo[id], "odom", "base_footprint")); //transforming coordinates from odom to base_footprint in order to aim to the correct coordinates with the arm
	if(toBaseFootprint == geometry_msgs::PoseStamped()) //if apriltag id is not detected
		ROS_INFO("Not found");
	
	
	tf2::Quaternion quaternion(toBaseFootprint.pose.orientation.x, toBaseFootprint.pose.orientation.y, toBaseFootprint.pose.orientation.z, toBaseFootprint.pose.orientation.w);  // Esempio di quaterniono

    // Creation of the rotation matrix from the quaternion
    tf2::Matrix3x3 rotation_matrix(quaternion);

    // Extracting the roll, pitch and yaw from the rotation matrix
    double roll, pitch, yaw;
    rotation_matrix.getRPY(roll, pitch, yaw);
    
	//Populate with the info from apriltag for position and orientation (USE BASE_LINK AS REFERENCE FRAME!!!!)
	geometry_msgs::PoseStamped target_pose;
	target_pose.header.frame_id = "base_footprint";
	tf2::Quaternion orientation;
	if(id == 3){
		//set pose
		target_pose.pose.position.x = toBaseFootprint.pose.position.x;
		target_pose.pose.position.y = toBaseFootprint.pose.position.y-0.2;
		target_pose.pose.position.z = toBaseFootprint.pose.position.z + 0.1;
		ROS_INFO("SEND IN POSITION: ---->   x %lf  y %lf  z %lf   yaw %lf", target_pose.pose.position.x, target_pose.pose.position.y, target_pose.pose.position.z, yaw);
		// Set orientation
		orientation.setRPY(M_PI/2,0,M_PI/2); //M_PI is pi, it needs double values 
	}
	else if(id == 2){
		target_pose.pose.position.x = toBaseFootprint.pose.position.x-0.2;
		target_pose.pose.position.y = toBaseFootprint.pose.position.y;
		target_pose.pose.position.z = toBaseFootprint.pose.position.z + 0.13;
		ROS_INFO("SEND IN POSITION: ---->   x %lf  y %lf  z %lf   yaw %lf", target_pose.pose.position.x, target_pose.pose.position.y, target_pose.pose.position.z, yaw);
		// Set orientation
		orientation.setRPY(M_PI/2,0,0); //M_PI is pi, it needs double values 
	}
	else{ //id == 1
		//set pose
		target_pose.pose.position.x = toBaseFootprint.pose.position.x-0.2;
		target_pose.pose.position.y = toBaseFootprint.pose.position.y;
		target_pose.pose.position.z = toBaseFootprint.pose.position.z + 0.1;
		ROS_INFO("SEND IN POSITION: ---->   x %lf  y %lf  z %lf   yaw %lf", target_pose.pose.position.x, target_pose.pose.position.y, target_pose.pose.position.z, yaw);
		// Set orientation
		orientation.setRPY(M_PI/2,0,0); //M_PI is pi, it needs double values 
	}
	target_pose.pose.orientation = tf2::toMsg(orientation);
	

	group_arm_torso.setPoseReferenceFrame("base_footprint");
	ROS_INFO("Rf frame: %s", group_arm_torso.getPlanningFrame().c_str());
	group_arm_torso.setPoseTarget(target_pose);
	ROS_INFO("PASSED THIS SCOGLIO");

	moveit::planning_interface::MoveGroupInterface::Plan my_plan2;
     group_arm_torso.setPlanningTime(30.0);
     group_arm_torso.plan(my_plan2);
	ros::Time start = ros::Time::now();
	
	group_arm_torso.move();
	//spinner.stop();
	ROS_INFO("PASSED also this one");
	
	removeObst(std::to_string(id), scene_plan);
	
	
	geometry_msgs::PoseStamped target_pose1;
	target_pose1.header.frame_id = "base_footprint";
	target_pose1.pose.position.x = target_pose.pose.position.x;
	target_pose1.pose.position.y =  target_pose.pose.position.y;
	target_pose1.pose.position.z =  target_pose.pose.position.z - 0.11;
	
	// Set orientation
	target_pose1.pose.orientation.x = target_pose.pose.orientation.x;
	target_pose1.pose.orientation.y = target_pose.pose.orientation.y;
	target_pose1.pose.orientation.z =  target_pose.pose.orientation.z;
	target_pose1.pose.orientation.w = target_pose.pose.orientation.w;	
	group_arm_torso.setPoseReferenceFrame("base_footprint");
	ROS_INFO("Rf frame: %s", group_arm_torso.getPlanningFrame().c_str());
	group_arm_torso.setPoseTarget(target_pose1);
	ROS_INFO("PASSED THIS SCOGLIO");

  
	moveit::planning_interface::MoveGroupInterface::Plan my_plan3;
    group_arm_torso.setPlanningTime(5.0);
    group_arm_torso.plan(my_plan3);
	start = ros::Time::now();
	
	group_arm_torso.move();

	closeGripper();
	attachGripper(id);
	intermediatePose(group_arm_torso);
	
	 
	
}


void addCylinderCollision(moveit::planning_interface::PlanningSceneInterface& scene_plan){
	//In this vector, we put all the coordinates that represent the columns 
	//which the robot will interact with
	std::vector<moveit_msgs::CollisionObject> collision_objects;
	collision_objects.resize(3);
	
	//Pick table definition
	collision_objects[0].id = "placeG";
	collision_objects[0].header.frame_id = "map";
	
	//define primitive dimension, position of pick table
	collision_objects[0].primitives.resize(1);
	collision_objects[0].primitives[0].type = collision_objects[0].primitives[0].CYLINDER;
	collision_objects[0].primitives[0].dimensions.resize(2);
	collision_objects[0].primitives[0].dimensions[0] = 0.72; 
	collision_objects[0].primitives[0].dimensions[1] = 0.26; 
	
	//defining position of pick table
	collision_objects[0].primitive_poses.resize(1);
	collision_objects[0].primitive_poses[0].position.x = 11.55;
	collision_objects[0].primitive_poses[0].position.y = -0.365;
	collision_objects[0].primitive_poses[0].position.z = 0.37;
	collision_objects[0].primitive_poses[0].orientation.w = 1.0;
	
	collision_objects[0].operation = collision_objects[0].ADD;
	
		//Pick table definition
	collision_objects[1].id = "placeR";
	collision_objects[1].header.frame_id = "map";
	
	//define primitive dimension, position of pick table
	collision_objects[1].primitives.resize(1);
	collision_objects[1].primitives[0].type = collision_objects[0].primitives[0].CYLINDER;
	collision_objects[1].primitives[0].dimensions.resize(2);
	collision_objects[1].primitives[0].dimensions[0] = 0.72; 
	collision_objects[1].primitives[0].dimensions[1] = 0.26; 
	
	//defining position of pick table
	collision_objects[1].primitive_poses.resize(1);
	collision_objects[1].primitive_poses[0].position.x = 10.55;
	collision_objects[1].primitive_poses[0].position.y = -0.365;
	collision_objects[1].primitive_poses[0].position.z = 0.37;
	collision_objects[1].primitive_poses[0].orientation.w = 1.0;
	
	collision_objects[1].operation = collision_objects[1].ADD;
	
			//Pick table definition
	collision_objects[2].id = "placeB";
	collision_objects[2].header.frame_id = "map";
	
	//define primitive dimension, position of pick table
	collision_objects[2].primitives.resize(1);
	collision_objects[2].primitives[0].type = collision_objects[0].primitives[0].CYLINDER;
	collision_objects[2].primitives[0].dimensions.resize(2);
	collision_objects[2].primitives[0].dimensions[0] = 0.72; 
	collision_objects[2].primitives[0].dimensions[1] = 0.26; 
	
	//defining position of pick table
	collision_objects[2].primitive_poses.resize(1);
	collision_objects[2].primitive_poses[0].position.x = 12.55;
	collision_objects[2].primitive_poses[0].position.y = -0.365;
	collision_objects[2].primitive_poses[0].position.z = 0.37;
	collision_objects[2].primitive_poses[0].orientation.w = 1.0;
	
	collision_objects[2].operation = collision_objects[2].ADD;
	
	
	scene_plan.applyCollisionObjects(collision_objects);
}


//ADD COLLISION OBJECTS (USE POSITION OF APRIL TAG WITH WORLD REFERENCE FRAME)
//here you have the table and 3 objects already displayed
void addCollisionObject(moveit::planning_interface::PlanningSceneInterface& scene_plan, std::vector<geometry_msgs::PoseStamped> aprilTagInfo){
	
	//In this vector, we put all the coordinates that represent the tables 
	//which the robot will interact with
	std::vector<moveit_msgs::CollisionObject> collision_objects;
	collision_objects.resize(8);

	//Pick table definition
	collision_objects[0].id = "pickT";
	collision_objects[0].header.frame_id = "map";
	
	//define primitive dimension, position of pick table
	collision_objects[0].primitives.resize(1);
	collision_objects[0].primitives[0].type = collision_objects[0].primitives[0].BOX;
	collision_objects[0].primitives[0].dimensions.resize(3);
	collision_objects[0].primitives[0].dimensions[0] = 0.98; 
	collision_objects[0].primitives[0].dimensions[1] = 0.98; 
	collision_objects[0].primitives[0].dimensions[2] = 0.74; 
	
	//defining position of pick table
	collision_objects[0].primitive_poses.resize(1);
	collision_objects[0].primitive_poses[0].position.x = 7.8;
	collision_objects[0].primitive_poses[0].position.y = -3.0;
	collision_objects[0].primitive_poses[0].position.z = 0.39;
	collision_objects[0].primitive_poses[0].orientation.w = 1.0;
	
	collision_objects[0].operation = collision_objects[0].ADD;

	for(int i = 1; i < 8; i++){

		if(aprilTagInfo[i] != geometry_msgs::PoseStamped()){
			collision_objects[i].id = std::to_string(i); 
			collision_objects[i].header.frame_id = "map";

			collision_objects[i].primitives.resize(1);
			collision_objects[i].primitives[0].type = collision_objects[0].primitives[0].CYLINDER;
			collision_objects[i].primitives[0].dimensions.resize(2);
			collision_objects[i].primitive_poses.resize(1);
			if(std::stoi(collision_objects[i].id) != 1 && std::stoi(collision_objects[i].id) != 2 && std::stoi(collision_objects[i].id) != 3){
				collision_objects[i].primitives[0].dimensions[0] = 0.2; 
				collision_objects[i].primitives[0].dimensions[1] = 0.05;
				collision_objects[i].primitive_poses[0].position.z = 0.88;	 
			}
			else{
				collision_objects[i].primitives[0].dimensions[0] = 0.09; 
				collision_objects[i].primitives[0].dimensions[1] = 0.02;
				collision_objects[i].primitive_poses[0].position.z = 0.83; //maybe we need a different value for blue, red, green.... 
			}

			
			collision_objects[i].primitive_poses[0].position.x = aprilTagInfo[i].pose.position.x; 	//8.099; value x from apriltag WRT MAP!!!!!
			collision_objects[i].primitive_poses[0].position.y = aprilTagInfo[i].pose.position.y;	//-2.646; value y from apriltag WRT MAP!!! 
			collision_objects[i].primitive_poses[0].orientation.w = 1.0;
		
			collision_objects[i].operation = collision_objects[i].ADD;
		}

	}
	
	scene_plan.applyCollisionObjects(collision_objects);

}


void headMovement(double joint_1, double joint_2)
{
	// Set up action client for joint trajectory action
    actionlib::SimpleActionClient<control_msgs::FollowJointTrajectoryAction> ac("/head_controller/follow_joint_trajectory", true);

    // Wait for the action server to start
    ac.waitForServer();

    // Create a trajectory goal
    control_msgs::FollowJointTrajectoryGoal goal;

    // Fill in the details of the trajectory goal
    // Specify joint names and target positions/velocities/efforts
    goal.trajectory.joint_names = {"head_1_joint", "head_2_joint"};
    goal.trajectory.points.resize(1);  // Number of waypoints

    // Specify positions, velocities, and effort values for each joint
    goal.trajectory.points[0].positions = {joint_1, joint_2};  // setting positions
    goal.trajectory.points[0].velocities = {0.0, 0.0};  // Default velocities
    goal.trajectory.points[0].effort = {0.0, 0.0};      // Default effort

    // Specify time from start for the trajectory
    goal.trajectory.points[0].time_from_start = ros::Duration(2.0);  // duration

    // Send the goal to the action server
    ac.sendGoal(goal);

    // Wait for the action to finish
    bool actionFinished = ac.waitForResult();

	//Check of the goal-state
    if (actionFinished) {
        // Action finished
        actionlib::SimpleClientGoalState state = ac.getState();
        ROS_INFO("Action finished: %s", state.toString().c_str());

        if (state == actionlib::SimpleClientGoalState::SUCCEEDED) {
            ROS_INFO("Action succeeded!");
        } else {
            ROS_ERROR("Action failed with result: %s", state.toString().c_str());
        }
    } else {
        // Action did not finish within the specified timeout
        ROS_ERROR("Action did not finish within the specified timeout");
    }

}


void liftTorso(double lift)
{	
    // Set up action client for joint trajectory action
    actionlib::SimpleActionClient<control_msgs::FollowJointTrajectoryAction> ac("/torso_controller/follow_joint_trajectory", true);

    // Wait for the action server to start
    ac.waitForServer();

    // Create a trajectory goal
    control_msgs::FollowJointTrajectoryGoal goal;

    // Fill in the details of the trajectory goal
    // Specify joint names and target positions/velocities/efforts
    goal.trajectory.joint_names = {"torso_lift_joint"};
    goal.trajectory.points.resize(1);  // Number of waypoints

    // Specify positions, velocities, and effort values for each joint
    goal.trajectory.points[0].positions = {lift};  // Example positions
    goal.trajectory.points[0].velocities = {0.0};  // Default velocities
    goal.trajectory.points[0].effort = {0.0};      // Default effort

    // Specify time from start for the trajectory
    goal.trajectory.points[0].time_from_start = ros::Duration(2.0);  // duration

    // Send the goal to the action server
    ac.sendGoal(goal);

    // Wait for the action to finish
    bool actionFinished = ac.waitForResult();

	//Check of the goal-state
    if (actionFinished) {
        // Action finished
        actionlib::SimpleClientGoalState state = ac.getState();
        ROS_INFO("Action finished: %s", state.toString().c_str());

        if (state == actionlib::SimpleClientGoalState::SUCCEEDED) {
            ROS_INFO("Action succeeded!");
        } else {
            ROS_ERROR("Action failed with result: %s", state.toString().c_str());
        }
    } else {
        // Action did not finish within the specified timeout
        ROS_ERROR("Action did not finish within the specified timeout");
    }

}

bool pick_and_place_Callback(assignment2::pickAndPlace::Request& req, assignment2::pickAndPlace::Response& res)
{
	//ROS_INFO("I entered in the pick and place callback");
    if(req.ready){
		std::vector<geometry_msgs::PoseStamped> aprilTagInfo;
		moveit::planning_interface::PlanningSceneInterface scene_plan;
		moveit::planning_interface::MoveGroupInterface group("arm_torso");
		group.setPlanningTime(30.0);

		
		ros::Duration(3.0).sleep();
		
		if(req.action == 1 || req.action == 2 || req.action == 3){ //requested to pick blue
			if(req.action == 1){
				headMovement(-0.27,-0.79);
				liftTorso(0.35);
			}
			if(req.action == 2){
				headMovement(-0.20,-0.81);
				liftTorso(0.35);
			}
			if(req.action == 3){
				headMovement(0.62,-0.78);
				liftTorso(0.35);
			}
			////////////////////////////////////////////////////




			ros::NodeHandle n_det;
			ros::ServiceClient client = n_det.serviceClient<assignment2::tags>("tag_coord");
			assignment2::tags srv;
			int done = false;
			int counter = 0;
			while(!done && counter<3){
			    srv.request.start = true;
			
			
				while (ros::ok() && !client.call(srv)) {			
					ROS_INFO("WAITNG DETECTION SERVICE TO BE DONE");
					ros::Duration(1.0).sleep(); 
				}
				if (client.call(srv))
				{
					for(int i = 0; i < 8; i++)
					{
						ROS_INFO("RECIVED THE DETECTION RESPONSE");
						aprilTagInfo.push_back(srv.response.poses[i]);
					}
						if(aprilTagInfo[req.action] == geometry_msgs::PoseStamped())
							done = false;
						else
							done = true;
				}
				else
				{
					ROS_ERROR("Failed to call service");
					return 1;
				}
				counter ++;
			}
			///////////////////////////////////////////////////
			
			addCollisionObject(scene_plan, aprilTagInfo);
			
			pick(group, aprilTagInfo, req.action, scene_plan);
			safePosition(group);
		}
		else{
			addCylinderCollision(scene_plan);
			placePosition(group,req.action);
			
		}
	}
	res.status = true;


		
    return true;  // Indica che il servizio è stato eseguito con successo
}



int main(int argc, char** argv){
	
	ros::init(argc, argv, "pickandplace");
    ros::NodeHandle nh;
	
	ros::AsyncSpinner spinner(2);
	
	spinner.start();

    ros::ServiceServer server = nh.advertiseService("/human_objects_srv", pick_and_place_Callback);
	ROS_INFO("My Service Server is ready.");

	ros::waitForShutdown();
    return 0;
}
