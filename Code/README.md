# README #

## GROUP 01   

Giuseppe Labate, giuseppe.labate.1@studenti.unipd.it  

Greta Piai, greta.piai@studenti.unipd.it  

Francesco Bordignon, francesco.bordignon.5@studenti.unipd.it  

## ASSIGNMENT_1
# WARNING!!! #
We accidentally uploaded the tiago_ias_simulation file twice in the repository and we were not able to remove it safely with a git rm,
so it they are still there.
Don't copy the first tiago_iaslab_simulation but leave the one in the /src directory


### Running Commands ###

* First: do a catkin build
* To open map on Gazebo : roslaunch tiago_iaslab_simulation start_simulation.launch world_name:=robotics_library
* To start navigation.launch : roslaunch tiago_iaslab_simulation navigation.launch
* To start server : rosrun assignment1 server_rbt (it may happen that in some cases the server does not close correctly: in that case close the terminal window and open a new one)
* To start client : rosrun assignment1 client_usr x y z or_x or_y or_z or_w (x, y, z are the position's values, or_x, or_y, or_z, or_w are the position's orietnation values)
* To start rviz : rviz and then ctrl+O and select robotConfig.rviz

## ASSIGNMENT_2

### Running Commands ###

* First: do a catkin build
* At the beginning of each terminal do: 
* $> start_tiago 
* $> source /opt/ros/noetic/setup.bash
* $> source /tiago_public_ws/devel/setup.bash
* $> source /package_path/devel/setup.bash

* •	In the first terminal start the simulation, the navigation stack, MoveIt and the AprilTag with our custom launcher:
* $> roslaunch assignment2 project2.launch

* •	In the second terminal start the Assistant.cpp node:
* $> rosrun assignment2 Assistant 

* •	In the third terminal start the HumanService node:
* $> rosrun tiago_iaslab_simulation human_node

* •	In the fourth terminal start the server_rbt.cpp node:
* $> rosrun assignment1 server_rbt

* •	In the fifth terminal start the pick_and_place.cpp node:
* $> rosrun assignment2 pick_and_place

* •	In the sixth terminal start the aprilTagDetection.cpp node:
* $> rosrun assignment2 aprilTagDetection




