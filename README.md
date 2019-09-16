# robot_with_lidar_plugin
Four wheeled robot with lidar on it

## Background:

A four wheel robot is created with a chassis link and four wheel links namely left front wheel, right front wheel, left back wheel and right back wheel.

A lidar is placed on top of this robot and the model is created using a base cylinder and top cylinder model. The ray sensor is embedded into the top cylinder with the specifications taken from velodyne lidar.

A lidar plugin is created to control the velocity of the rotational top cylinder which gathers the data points by emitting lasers in all directions (360 degrees)

```
Folder structure:
lidar_plugin/
├── CMakeLists.txt
├── velodyne_on_robot.world
└── velodyne_plugin.cc
```

Initial compilation of the plugin:

```
source /opt/ros/melodic/setup.bash
cd lidar_plugin
mkdir build
cd build
cmake ../
make
```
This would have created 
`
liblidar_plugin.so
`
 Binary file which is used in the world file.


To run this, go to the below mentioned folder (load melodic in each of the terminals):

### Terminal 1:
```
cd lidar_plugin/build
roscore
```

### Terminal 2:
```
gazebo --verbose ../velodyne_on_robot.world
```

To test,
### Terminal 3:
```
rostopic pub /lidar_plugin_topic std_msgs/Float32 1
```

### Logic:
The velocity parameter is published using rostopic pub command over `/lidar_plugin_topic` 
`velodyne_plugin.cc` has a subscriber which subscribes to this topic `/lidar_plugin_topic` and callback is activated which sets the velocity for the obtained joint in the code.


