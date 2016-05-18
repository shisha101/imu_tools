/*
 *  Copyright (C) 2010, CCNY Robotics Lab
 *  Ivan Dryanovski <ivan.dryanovski@gmail.com>
 *
 *  http://robotics.ccny.cuny.edu
 *
 *  Based on implementation of Madgwick's IMU and AHRS algorithms.
 *  http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IMU_FILTER_MADWICK_IMU_FILTER_ROS_H
#define IMU_FILTER_MADWICK_IMU_FILTER_ROS_H

#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/MagneticField.h>
#include <geometry_msgs/Vector3Stamped.h>
#include "tf2_ros/transform_broadcaster.h"
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <dynamic_reconfigure/server.h>

#include "imu_filter_madgwick/imu_filter.h"
#include "imu_filter_madgwick/ImuFilterMadgwickConfig.h"
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <std_srvs/Empty.h>

class ImuFilterRos
{
  typedef sensor_msgs::Imu              ImuMsg;
  typedef sensor_msgs::MagneticField    MagMsg;
  typedef geometry_msgs::Vector3Stamped MagVectorMsg;

  typedef message_filters::sync_policies::ApproximateTime<ImuMsg, MagMsg> SyncPolicy;
  typedef message_filters::Synchronizer<SyncPolicy> Synchronizer;
  typedef message_filters::Subscriber<ImuMsg> ImuSubscriber;
  typedef message_filters::Subscriber<MagMsg> MagSubscriber;
  typedef message_filters::Subscriber<MagVectorMsg> MagVectorSubscriber;

  typedef imu_filter_madgwick::ImuFilterMadgwickConfig   FilterConfig;
  typedef dynamic_reconfigure::Server<FilterConfig>   FilterConfigServer;

  public:

    ImuFilterRos(ros::NodeHandle nh, ros::NodeHandle nh_private);
    virtual ~ImuFilterRos();

  private:

    // **** ROS-related

    ros::NodeHandle nh_;
    ros::NodeHandle nh_private_;

    boost::shared_ptr<Synchronizer> sync_;
    boost::shared_ptr<ImuSubscriber> imu_subscriber_;
    boost::shared_ptr<MagSubscriber> mag_subscriber_;

    // Adapter to support the use_magnetic_field_msg param.
    boost::shared_ptr<MagVectorSubscriber> vector_mag_subscriber_;
    ros::Publisher mag_republisher_;

    ros::Publisher rpy_filtered_debug_publisher_;
    ros::Publisher rpy_raw_debug_publisher_;
    ros::Publisher imu_publisher_;
    tf2_ros::TransformBroadcaster tf_broadcaster_;
    ros::ServiceServer home_service_;

    boost::shared_ptr<FilterConfigServer> config_server_;

    // **** paramaters
    bool use_mag_;
    bool use_magnetic_field_msg_;
    bool publish_tf_;
    bool reverse_tf_;
    std::string fixed_frame_;
    std::string imu_frame_;
    double constant_dt_;
    bool publish_debug_topics_;
    geometry_msgs::Vector3 mag_bias_;
    double orientation_variance_;
    std::string mag_topic_sub_;
    std::string mag_topic_pub_;
    std::string imu_topic_sub_;
    std::string imu_topic_pub_;

    // AHRS calibration parameters
    std::vector<double> gyro_offset_;
    std::vector<double> acc_offset_;
    std::vector<double> acc_scaling_;
    std::vector<double> mag_sens_;   // magnetometer sensitivity
    std::vector<double> mag_bias_s_; // bias with sensitivty
    std::vector<double> mag_bias_r_; // raw bias with out sensitivity
    std::vector<double> mag_bias_e_; // ellipsoid bias
    std::vector<double> mag_trans_e_; // ellipsoid transformation
    tf2::Matrix3x3 mag_trans_mat; //  the trasformation matrix used to correct magnetometer information
    tf2::Vector3 bias_vec;


    // **** state variables
    boost::mutex mutex_;
    bool initialized_;
    ros::Time last_time_;
    tf2::Quaternion home_;

    // **** filter implementation
    ImuFilter filter_;

    // **** member functions
    void imuMagCallback(const ImuMsg::ConstPtr& imu_msg_raw,
                        const MagMsg::ConstPtr& mav_msg);

    void imuCallback(const ImuMsg::ConstPtr& imu_msg_raw);

    void imuMagVectorCallback(const MagVectorMsg::ConstPtr& mag_vector_msg);

    void publishFilteredMsg(const ImuMsg::ConstPtr& imu_msg_raw);
    void publishTransform(const ImuMsg::ConstPtr& imu_msg_raw);

    void publishRawMsg(const ros::Time& t,
                       float roll, float pitch, float yaw);

    void reconfigCallback(FilterConfig& config, uint32_t level);

    void computeRPY(float ax, float ay, float az,
                    float mx, float my, float mz,
                    float& roll, float& pitch, float& yaw);
    // **** service for home position
    bool set_home(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res);
};

#endif // IMU_FILTER_IMU_MADWICK_FILTER_ROS_H
