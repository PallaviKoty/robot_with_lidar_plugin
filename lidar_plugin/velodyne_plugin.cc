#ifndef _VELODYNE_PLUGIN_HH_
#define _VELODYNE_PLUGIN_HH_

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <thread>
#include <functional>
#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "ros/subscribe_options.h"
#include "std_msgs/Float32.h"
#include <gazebo/common/common.hh>
#include <ignition/math/Vector3.hh>
#include <gazebo/gazebo_client.hh>
#include <ignition/math/Pose3.hh>
#include <iostream>

namespace gazebo
{
/// \brief A plugin to control a Velodyne sensor.
class VelodynePlugin : public ModelPlugin
{
    /// \brief Constructor
public:
    double velocity = 0;

    /// \brief Pointer to the model.
private:
    physics::ModelPtr model;
    ignition::math::Pose3d pose;
    /// \brief Pointer to the joint.
private:
    physics::JointPtr joint;

    /// \brief A PID controller for the joint.
private:
    common::PID pid;

    /// \brief A node used for transport
private:
    transport::NodePtr node;

    /// \brief A subscriber to a named topic.
private:
    transport::SubscriberPtr sub;

    /// \brief A node use for ROS transport
private:
    std::unique_ptr<ros::NodeHandle> rosNode;

    /// \brief A ROS subscriber
private:
    ros::Subscriber rosSub;

    /// \brief A ROS callbackqueue that helps process messages
private:
    ros::CallbackQueue rosQueue;

    /// \brief A thread the keeps running the rosQueue
private:
    std::thread rosQueueThread;

private:
    event::ConnectionPtr updateConnection;

    double x, y, z;

public:
    VelodynePlugin() {}

    /// \brief Set the velocity of the Velodyne
    /// \param[in] _vel New target velocity
    void SetVelocity(const double &_vel)
    {
        // Set the joint's target velocity.
        this->model->GetJointController()->SetVelocityTarget(
            this->joint->GetScopedName(), _vel);
    }

    /// \brief The load function is called by Gazebo when the plugin is
    /// inserted into simulation
    /// \param[in] _model A pointer to the model that this plugin is
    /// attached to.
    /// \param[in] _sdf A pointer to the plugin's SDF element.
    virtual void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
    {
        // Safety check
        if (_model->GetJointCount() == 0)
        {
            std::cerr << "Invalid joint count, Velodyne plugin not loaded\n";
            return;
        }

        // Store the model pointer for convenience.
        this->model = _model;

        // Get the first joint. We are making an assumption about the model
        // having one joint that is the rotational joint.
        this->joint = _model->GetJoints()[1];

        // Setup a P-controller, with a gain of 0.1.
        this->pid = common::PID(0.1, 0, 0);

        // Apply the P-controller to the joint.
        this->model->GetJointController()->SetVelocityPID(
            this->joint->GetScopedName(), this->pid);

        // Set the joint's target velocity. This target velocity is just
        // for demonstration purposes.
        // Check that the velocity element exists, then read the value
        // if (_sdf->HasElement("velocity"))
        //     velocity = _sdf->Get<double>("velocity");

        // Set the joint's target velocity. This target velocity is just
        // for demonstration purposes.
        // this->SetVelocity(velocity);

        // Create the node
        //         this->node = transport::NodePtr(new transport::Node());
        // #if GAZEBO_MAJOR_VERSION < 8
        //         this->node->Init(this->model->GetWorld()->GetName());
        // #else
        //         this->node->Init(this->model->GetWorld()->Name());
        // #endif

        // Subscribe to the topic, and register a callback
        // this->sub = this->node->Subscribe("/lidar_plugin_topic",
        //   &VelodynePlugin::OnMsg, this);

        // Initialize ros, if it has not already bee initialized.
        if (!ros::isInitialized())
        {
            int argc = 0;
            char **argv = NULL;
            ros::init(argc, argv, "gazebo_client",
                      ros::init_options::NoSigintHandler);
        }

        // Create our ROS node. This acts in a similar manner to
        // the Gazebo node
        this->rosNode.reset(new ros::NodeHandle("gazebo_client"));

        // Create a named topic, and subscribe to it.
        ros::SubscribeOptions so =
            ros::SubscribeOptions::create<std_msgs::Float32>(
                "/lidar_plugin_topic",
                1,
                boost::bind(&VelodynePlugin::OnRosMsg, this, _1),
                ros::VoidPtr(), &this->rosQueue);
        this->rosSub = this->rosNode->subscribe(so);

        // Spin up the queue helper thread.
        this->rosQueueThread =
            std::thread(std::bind(&VelodynePlugin::QueueThread, this));

        // Listen to the update event. This event is broadcast every
        // simulation iteration.


        this->updateConnection = event::Events::ConnectWorldUpdateBegin(
            std::bind(&VelodynePlugin::OnUpdate, this));

    }


public:
    void OnUpdate()
    {
        pose = this->model->WorldPose();
        std::cout << "World Pose is : " << pose << std::endl;
    }

    /// \brief Handle incoming message
    /// \param[in] _msg Repurpose a vector3 message. This function will
    /// only use the x component.
    // private:
    //     void OnMsg(ConstVector3dPtr &_msg)
    //     {
    //         this->SetVelocity(_msg->x());
    //     }

    /// \brief Handle an incoming message from ROS
    /// \param[in] _msg A float value that is used to set the velocity
    /// of the Velodyne.
public:
    void OnRosMsg(const std_msgs::Float32ConstPtr &_msg)
    {
        this->SetVelocity(_msg->data);
    }

    /// \brief ROS helper function that processes messages
private:
    void QueueThread()
    {
        static const double timeout = 0.01;
        while (this->rosNode->ok())
        {
            this->rosQueue.callAvailable(ros::WallDuration(timeout));
        }
    }
};

// Tell Gazebo about this plugin, so that Gazebo can call Load on this plugin.
GZ_REGISTER_MODEL_PLUGIN(VelodynePlugin)
} // namespace gazebo
#endif