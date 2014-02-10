// bsd license blah blah
#include <cstring>
#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <tf_conversions/tf_kdl.h>
#include <tf/transform_datatypes.h>
#include <kdl_parser/kdl_parser.hpp>
#include <kdl/jntarray.hpp>
#include <kdl/chainiksolverpos_nr_jl.hpp>
#include <kdl/chainiksolvervel_pinv.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>
//#include <motor_control_msgs/GetPositionFK.h>
//#include <motor_control_msgs/GetPositionIK.h>
//#include <kinematics_msgs/GetKinematicSolverInfo.h>
//#include <kinematics_msgs/KinematicSolverInfo.h>
#include <arm_kinematics.h>

using std::string;

//static const std::string IK_SERVICE = "get_ik";
//static const std::string FK_SERVICE = "get_fk";
//static const std::string IK_INFO_SERVICE = "get_ik_solver_info";
//static const std::string FK_INFO_SERVICE = "get_fk_solver_info";
/*
class Kinematics {
    public:
        Kinematics();
        bool init();

    private:
        ros::NodeHandle nh, nh_private;
        std::string root_name, tip_name;
        KDL::JntArray joint_min, joint_max;
        KDL::Chain chain;
        unsigned int num_joints;

        KDL::ChainFkSolverPos_recursive* fk_solver;
        KDL::ChainIkSolverPos_NR_JL *ik_solver_pos;
        KDL::ChainIkSolverVel_pinv* ik_solver_vel;

        ros::ServiceServer ik_service,ik_solver_info_service;
        ros::ServiceServer fk_service,fk_solver_info_service;

        tf::TransformListener tf_listener;

        kinematics_msgs::KinematicSolverInfo info;

        bool loadModel(const std::string xml);
        bool readJoints(urdf::Model &robot_model);
        int getJointIndex(const std::string &name);
        int getKDLSegmentIndex(const std::string &name);

        /**
         * @brief This is the basic IK service method that will compute and return an IK solution.
         * @param A request message. See service definition for GetPositionIK for more information on this message.
         * @param The response message. See service definition for GetPositionIK for more information on this message.
         */
     //   bool getPositionIK(kinematics_msgs::GetPositionIK::Request &request,
     //                      kinematics_msgs::GetPositionIK::Response &response);

        /**
         * @brief This is the basic kinematics info service that will return information about the kinematics node.
         * @param A request message. See service definition for GetKinematicSolverInfo for more information on this message.
         * @param The response message. See service definition for GetKinematicSolverInfo for more information on this message.
         */
      //  bool getIKSolverInfo(kinematics_msgs::GetKinematicSolverInfo::Request &request,
    //                         kinematics_msgs::GetKinematicSolverInfo::Response &response);

        /**
         * @brief This is the basic kinematics info service that will return information about the kinematics node.
         * @param A request message. See service definition for GetKinematicSolverInfo for more information on this message.
         * @param The response message. See service definition for GetKinematicSolverInfo for more information on this message.
         */
   //     bool getFKSolverInfo(kinematics_msgs::GetKinematicSolverInfo::Request &request,
   //                          kinematics_msgs::GetKinematicSolverInfo::Response &response);
//
        /**
         * @brief This is the basic forward kinematics service that will return information about the kinematics node.
         * @param A request message. See service definition for GetPositionFK for more information on this message.
         * @param The response message. See service definition for GetPositionFK for more information on this message.
         */
      /*  bool getPositionFK(kinematics_msgs::GetPositionFK::Request &request,
                           kinematics_msgs::GetPositionFK::Response &response);
};*/

namespace arm_kinematics {

Kinematics::Kinematics(): nh_private ("~") {
}

bool Kinematics::init() {
    // Get URDF XML
    //std::string ns = ros::this_node::getNamespace();
  std::cout<<"The namespace of arm_kinematics is ++++++++++++++++++++++++++++"<<std::endl;
  //ROS_DEBUG(ns);

    std::string urdf_xml, full_urdf_xml;
    nh.param("urdf_xml",urdf_xml,std::string("robot_description"));
 	std::cout<<"The urdf_xml is "<<urdf_xml<<std::endl;
    nh.searchParam(urdf_xml,full_urdf_xml);
 	std::cout<<"The full_urdf_xml is "<<full_urdf_xml<<std::endl;
    ROS_DEBUG("Reading xml file from parameter server");
    std::string result;
    if (!nh.getParam(full_urdf_xml, result)) {
        ROS_FATAL("Could not load the xml from parameter server: %s", urdf_xml.c_str());
        return false;
    }
 	//std::cout<<"The result is "<<result<<std::endl;

    // Get Root and Tip From Parameter Service
    if (!nh.getParam("root_name", root_name)) {
        ROS_FATAL("GenericIK: No root name found on parameter server");
        return false;
    }
    if (!nh.getParam("tip_name", tip_name)) {
        ROS_FATAL("GenericIK: No tip name found on parameter server");
        return false;
    }

    // Load and Read Models
    if (!loadModel(result)) {
        ROS_FATAL("Could not load models!");
        return false;
    }

    // Get Solver Parameters
    int maxIterations;
    double epsilon;

    nh_private.param("maxIterations", maxIterations, 1000);
    nh_private.param("epsilon", epsilon, 1e-2);
    std::cout<<"The namespace of arm_kinematics is ++++++++++++++++++++++++++++"<<std::endl;
    // Build Solvers
    fk_solver = new KDL::ChainFkSolverPos_recursive(chain);
    ik_solver_vel = new KDL::ChainIkSolverVel_pinv(chain);
    ik_solver_pos = new KDL::ChainIkSolverPos_NR_JL(chain, joint_min, joint_max,
            *fk_solver, *ik_solver_vel, maxIterations, epsilon);

    //ROS_INFO("Advertising services");
    //fk_service = nh_private.advertiseService(FK_SERVICE,&Kinematics::getPositionFK,this);
    //ik_service = nh_private.advertiseService(IK_SERVICE,&Kinematics::getPositionIK,this);
    //ik_solver_info_service = nh_private.advertiseService(IK_INFO_SERVICE,&Kinematics::getIKSolverInfo,this);
    //fk_solver_info_service = nh_private.advertiseService(FK_INFO_SERVICE,&Kinematics::getFKSolverInfo,this);

    return true;
}

bool Kinematics::loadModel(const std::string xml) {
    urdf::Model robot_model;
    KDL::Tree tree;
	std::cout<<"The tip is "<<tip_name<<std::endl;
	std::cout<<"The root name is "<<root_name<<std::endl;
	//std::cout<<"the xml is "<<xml<<std::endl;
    if (!robot_model.initString(xml)) {
        ROS_FATAL("Could not initialize robot model");
        return -1;
    }
    if (!kdl_parser::treeFromString(xml, tree)) {
        ROS_ERROR("Could not initialize tree object");
        return false;
    }
    if (!tree.getChain(root_name, tip_name, chain)) {
        ROS_ERROR("Could not initialize chain object");
        return false;
    }

    if (!readJoints(robot_model)) {
        ROS_FATAL("Could not read information about the joints");
        return false;
    }

    return true;
}

bool Kinematics::readJoints(urdf::Model &robot_model) {
    num_joints = 0;
    // get joint maxs and mins
    boost::shared_ptr<const urdf::Link> link = robot_model.getLink(tip_name);
    boost::shared_ptr<const urdf::Joint> joint;

    while (link && link->name != root_name) {
        joint = robot_model.getJoint(link->parent_joint->name);
        if (!joint) {
            ROS_ERROR("Could not find joint: %s",link->parent_joint->name.c_str());
            return false;
        }
        if (joint->type != urdf::Joint::UNKNOWN && joint->type != urdf::Joint::FIXED) {
            ROS_INFO( "adding joint: [%s]", joint->name.c_str() );
            num_joints++;
        }
        link = robot_model.getLink(link->getParent()->name);
    }

    joint_min.resize(num_joints);
    joint_max.resize(num_joints);
    info.joint_names.resize(num_joints);
    info.link_names.resize(num_joints);
    //info.limits.resize(num_joints);

    link = robot_model.getLink(tip_name);
    unsigned int i = 0;
    while (link && i < num_joints) {
        joint = robot_model.getJoint(link->parent_joint->name);
        if (joint->type != urdf::Joint::UNKNOWN && joint->type != urdf::Joint::FIXED) {
            ROS_INFO( "getting bounds for joint: [%s]", joint->name.c_str() );

            float lower, upper;
            int hasLimits;
            if ( joint->type != urdf::Joint::CONTINUOUS ) {
                lower = joint->limits->lower;
                upper = joint->limits->upper;
                hasLimits = 1;
            } else {
                lower = -M_PI;
                upper = M_PI;
                hasLimits = 0;
            }
            int index = num_joints - i -1;

            joint_min.data[index] = lower;
            joint_max.data[index] = upper;
            info.joint_names[index] = joint->name;
            info.link_names[index] = link->name;
            //info.limits[index].joint_name = joint->name;
            //info.limits[index].has_position_limits = hasLimits;
            //info.limits[index].min_position = lower;
            //info.limits[index].max_position = upper;
            i++;
        }
        link = robot_model.getLink(link->getParent()->name);
    }
    return true;
}


int Kinematics::getJointIndex(const std::string &name) {
    for (unsigned int i=0; i < info.joint_names.size(); i++) {
        if (info.joint_names[i] == name)
            return i;
    }
    return -1;
}

int Kinematics::getKDLSegmentIndex(const std::string &name) {
    int i=0; 
	//std::cout<<"into the kdl segment index "<<std::endl;
	//std::cout<<" chain segments is  "<<(int)chain.getNrOfSegments()<<std::endl;
    while (i < (int)chain.getNrOfSegments()) {
        //if (chain.getSegment(i).getName() == name) {
	//std::cout<<"The names are "<<chain.getSegment(i).getJoint().getName()<<" and "<<name<<std::endl;
	if (chain.getSegment(i).getJoint().getName() == name) {
            return i+1;
        }
        i++;
    }
    return -1;
}


//bool Kinematics::getPositionIK(kinematics_msgs::GetPositionIK::Request &request,
  //                             kinematics_msgs::GetPositionIK::Response &response) {
bool Kinematics::getPositionIK(const geometry_msgs::PoseStamped &pose_stamp,
                              const motor_control_msgs::JointPosition &seed,
                              motor_control_msgs::JointPosition *result) {

    //geometry_msgs::PoseStamped pose_msg_in = request.ik_request.pose_stamped;
    geometry_msgs::PoseStamped pose_msg_in = pose_stamp;
    tf::Stamped<tf::Pose> transform;
    tf::Stamped<tf::Pose> transform_root;
    tf::poseStampedMsgToTF( pose_msg_in, transform );

    //Do the IK
    KDL::JntArray jnt_pos_in;
    KDL::JntArray jnt_pos_out;

    jnt_pos_in.resize(num_joints);
    for (unsigned int i=0; i < num_joints; i++) {
        //int tmp_index = getJointIndex(request.ik_request.ik_seed_state.joint_state.name[i]);
	int tmp_index = getJointIndex(seed.names[i]);
	std::cout<<"The name and corresp index are "<<seed.names[i]<<" "<<tmp_index<<std::endl;
        if (tmp_index >=0) {
            //jnt_pos_in(tmp_index) = request.ik_request.ik_seed_state.joint_state.position[i];
	    jnt_pos_in(tmp_index) = seed.angles[i];
        } else {
            ROS_ERROR("i: %d, No joint index for %s",i,seed.names[i].c_str());
        }
    }

    //Convert F to our root_frame
    try {
        tf_listener.transformPose(root_name, transform, transform_root);
    } catch (...) {
        ROS_ERROR("Could not transform IK pose to frame: %s", root_name.c_str());
        //response.error_code.val = response.error_code.FRAME_TRANSFORM_FAILURE;
       return false;
    }

    KDL::Frame F_dest;
    tf::TransformTFToKDL(transform_root, F_dest);

    int ik_valid = ik_solver_pos->CartToJnt(jnt_pos_in, F_dest, jnt_pos_out);
    std::cout<<"The ik is valid since "<<ik_valid<<std::endl;

    if (ik_valid >= 0) {
	std::cout<<"This is valid"<<std::endl;
        //response.solution.joint_state.name = info.joint_names;
        result->names=info.joint_names;
        result->angles.resize(num_joints);
        for (unsigned int i=0; i < num_joints; i++) {
            result->angles[i] = jnt_pos_out(i);
            ROS_DEBUG("IK Solution: %s %d: %f",result->names[i].c_str(),i,jnt_pos_out(i));
        }
        //response.error_code.val = response.error_code.SUCCESS;
	//response.error_code.val = response.error_code.SUCCESS;
        return true;
    } else {
	//std::cout<<"This is not valid"<<std::endl;
        ROS_DEBUG("An IK solution could not be found");
        //response.error_code.val = response.error_code.NO_IK_SOLUTION;
        return false;
    }
}

//bool Kinematics::getIKSolverInfo(kinematics_msgs::GetKinematicSolverInfo::Request &request,
//                                 kinematics_msgs::GetKinematicSolverInfo::Response &response) {
/*bool Kinematics::getIKSolverInfo() {
  
    //response.kinematic_solver_info = info;
    return true;
}*/

//bool Kinematics::getFKSolverInfo(kinematics_msgs::GetKinematicSolverInfo::Request &request,
//                                 kinematics_msgs::GetKinematicSolverInfo::Response &response) {
/*bool Kinematics::getFKSolverInfo() {
    //response.kinematic_solver_info = info;
    return true;
}*/

//bool Kinematics::getPositionFK(kinematics_msgs::GetPositionFK::Request &request,
//                               kinematics_msgs::GetPositionFK::Response &response) {
bool Kinematics::getPositionFK(std::string frame_id,const motor_control_msgs::JointPosition &joint_configuration, motor_control_msgs::FKreply &result) {
 // std::cout<<"into the arm cgetposFK"<<std::endl;
 // std::cout<<"into the arm cgetposF2112K"<<std::endl;
    KDL::Frame p_out;
    KDL::JntArray jnt_pos_in;
    geometry_msgs::PoseStamped pose;
    tf::Stamped<tf::Pose> tf_pose;

//std::cout<<"The num joints are "<<num_joints<<std::endl;
    jnt_pos_in.resize(num_joints);
    for (unsigned int i=0; i < num_joints; i++) {
        //int tmp_index = getJointIndex(request.robot_state.joint_state.name[i]);
	int tmp_index = getJointIndex(joint_configuration.names[i]);
	//std::cout<<"The tmp index is "<<tmp_index<<std::endl;
        if (tmp_index >=0)
            //jnt_pos_in(tmp_index) = request.robot_state.joint_state.position[i];
		jnt_pos_in(tmp_index) = joint_configuration.angles[i];
    }

    //response.pose_stamped.resize(request.fk_link_names.size());
    //response.fk_link_names.resize(request.fk_link_names.size());
	//std::cout<<"The joint_configuration.name size is "<<joint_configuration.names.size()<<std::endl;
    result.pose_stamped.resize(joint_configuration.names.size());
    result.names.resize(joint_configuration.names.size());

    bool valid = true;
    //for (unsigned int i=0; i < request.fk_link_names.size(); i++) {
      for (unsigned int i=0; i < joint_configuration.names.size(); i++) {
        //int segmentIndex = getKDLSegmentIndex(request.fk_link_names[i]);
	//for (int ii=0;ii<7;ii++)
	//{
		//std::cout<<"The names are "<<chain.getSegment(ii).getName()<<std::endl;
		//std::cout<<"The joints are "<<chain.getSegment(ii).getJoint().getName()<<std::endl;
//}
	//std::cout<<"the name is "<<joint_configuration.names[i]<<std::endl;
	int segmentIndex = getKDLSegmentIndex(joint_configuration.names[i]);
	//std::cout<<"the chain segments are "<<chain.getNrOfSegments()<<std::endl;
	//std::cout<<"The segment index is "<<segmentIndex<<std::endl;
        ROS_DEBUG("End effector index: %d",segmentIndex);
        ROS_DEBUG("Chain indices: %d",chain.getNrOfSegments());
	//std::cout<<"the number of joints are "<<chain.getNrOfJoints()<<std::endl;
        if (fk_solver->JntToCart(jnt_pos_in,p_out,segmentIndex) >=0) {
	//std::cout<<"The fk solver is "<<fk_solver->JntToCart(jnt_pos_in,p_out,segmentIndex)<<std::endl;
            tf_pose.frame_id_ = root_name;
            tf_pose.stamp_ = ros::Time();
            tf::PoseKDLToTF(p_out,tf_pose);
	//std::cout<<"the root name is  "<<tf_pose.frame_id_<<std::endl;
	//std::cout<<"The out pose is "<<tf_pose<<std::endl;
            try {
                tf_listener.transformPose(frame_id,tf_pose,tf_pose);//resolve frame_id
            } catch (...) {
                ROS_ERROR("Could not transform FK pose to frame: %s",frame_id.c_str());
                //response.error_code.val = response.error_code.FRAME_TRANSFORM_FAILURE;
                return false;
            }
            tf::poseStampedTFToMsg(tf_pose,pose);
	//	std::cout<<"after conversion "<<pose<<std::endl;
	//	std::cout<<"after conversion name is "<<joint_configuration.names[i]<<std::endl;
            result.pose_stamped[i]=pose;
            result.names[i]=joint_configuration.names[i];
	    //result=pose.pose;
            //response.pose_stamped[i] = pose;
            //response.fk_link_names[i] = request.fk_link_names[i];
            //response.error_code.val = response.error_code.SUCCESS;
        } else {
            //ROS_ERROR("Could not compute FK for %s",request.fk_link_names[i].c_str());
	    ROS_ERROR("Could not compute FK for %s",joint_configuration.names[i].c_str());
            //response.error_code.val = response.error_code.NO_FK_SOLUTION;
            valid = false;
        }
 //std::cout<<"over completely"<<std::endl;
    }
//std::cout<<"Before returning true"<<std::endl;
    return true;
}

} //namespace

int main(int argc, char **argv) {
    ros::init(argc, argv, "arm_kinematics");
    arm_kinematics::Kinematics k;
    if (k.init()<0) {
        ROS_ERROR("Could not initialize kinematics node");
        return -1;
    }

    ros::spin();
    return 0;
}

