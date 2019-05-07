#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <ros/ros.h>
#include <nav_msgs/Path.h>  
#include<nav_msgs/Odometry.h>
#include <image_transport/image_transport.h>  
#include <cv_bridge/cv_bridge.h>
#include <stdio.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#define ture 1
#define false 0
// #include "extra.h" // use this if in OpenCV2 
using namespace std;
using namespace cv;
Mat frame0,frame1;
/****************************************************
 * 本程序演示了如何使用2D-2D的特征匹配估计相机运动
 * **************************************************/

void find_feature_matches (
    const Mat& img_1, const Mat& img_2,
    std::vector<KeyPoint>& keypoints_1,
    std::vector<KeyPoint>& keypoints_2,
    std::vector< DMatch >& matches );

void pose_estimation_2d2d (
    std::vector<KeyPoint> keypoints_1,
    std::vector<KeyPoint> keypoints_2,
    std::vector< DMatch > matches,
    Mat& R, Mat& t );

// 像素坐标转相机归一化坐标
Point2d pixel2cam ( const Point2d& p, const Mat& K );

void imageCallback0(const sensor_msgs::ImageConstPtr& msg)  
{  

  cv_bridge::CvImagePtr cv_ptr;  
  try  
  {  
     cv_ptr = cv_bridge::toCvCopy(msg, "mono8");  
  }  
  catch (cv_bridge::Exception& e)  
  {  
     ROS_ERROR("cv_bridge exception: %s", e.what());  
     return;  
  }  

  // cv::Mat caml;
  frame0 = cv_ptr->image;  
//imshow("1",caml);
  // fail if don't have waitKey(3).
 // cv::waitKey(3);
} 
void imageCallback1(const sensor_msgs::ImageConstPtr& msg)  
{  

  cv_bridge::CvImagePtr cv_ptr;  
  try  
  {  
     cv_ptr = cv_bridge::toCvCopy(msg, "mono8");  
  }  
  catch (cv_bridge::Exception& e)  
  {  
     ROS_ERROR("cv_bridge exception: %s", e.what());  
     return;  
  }  

  // cv::Mat caml;
  frame1 = cv_ptr->image;  
//imshow("1",caml);
  // fail if don't have waitKey(3).
 // cv::waitKey(3);
} 

int main ( int argc, char** argv )
{
    // if ( argc != 3 )
    // {
    //     cout<<"usage: pose_estimation_2d2d img1 img2"<<endl;
    //     return 1;
    // }
    //-- 读取图像
    ros::init(argc, argv, "pose");  
    ros::NodeHandle nh;  
    image_transport::ImageTransport it(nh);  
   image_transport::Subscriber sub0 = it.subscribe("camera/fisheye0", 1, imageCallback0);
   image_transport::Subscriber sub1 = it.subscribe("camera/fisheye1", 1, imageCallback1);
    ros::Publisher path_pub = nh.advertise<nav_msgs::Path>("trajectory",1, true);
    ros::Publisher odom_pub = nh.advertise<nav_msgs::Odometry>("odom", 50);
    ros::Time current_time, last_time;
    current_time = ros::Time::now();
    last_time = ros::Time::now();

    nav_msgs::Path path;
    //nav_msgs::Path path;
    path.header.stamp=current_time;
    path.header.frame_id="odom";
 ros::Rate loop_rate(10);
 
 while (nh.ok())
 {
     


    // Mat img_1 = imread ( "/home/fsdh/桌面/slambook-master/ch7/1.png", CV_LOAD_IMAGE_COLOR );
    // Mat img_2 = imread ( "/home/fsdh/桌面/slambook-master/ch7/2.png", CV_LOAD_IMAGE_COLOR );
   if(!frame0.empty()&!frame1.empty()){
    vector<KeyPoint> keypoints_1, keypoints_2;
    vector<DMatch> matches;
    find_feature_matches ( frame0, frame1, keypoints_1, keypoints_2, matches );
    cout<<"一共找到了"<<matches.size() <<"组匹配点"<<endl;
     current_time = ros::Time::now();
    //-- 估计两张图像间运动
    Mat R,t;
    pose_estimation_2d2d ( keypoints_1, keypoints_2, matches, R, t );

     Eigen::Matrix3d ER;
        ER<<R.at<double> ( 0,0 ),R.at<double> ( 0,1 ),R.at<double> ( 0,2 ),R.at<double> ( 1,0 ),R.at<double> ( 1,1 ),R.at<double> ( 1,2 ),R.at<double> ( 2,0 ),R.at<double> ( 2,1 ),R.at<double> ( 2,2 );
         Eigen::Quaterniond q = Eigen::Quaterniond(ER);
    // geometry_msgs::PoseStamped this_pose_stamped;
    //     this_pose_stamped.pose.position.x =t.at<double> ( 0,0 );
    //     this_pose_stamped.pose.position.y = 0;
    // this_pose_stamped.pose.position.z =0;
       
    //     this_pose_stamped.pose.orientation.x=q.x();
    //     this_pose_stamped.pose.orientation.y=q.y();
    //     this_pose_stamped.pose.orientation.z=q.z();
    //     this_pose_stamped.pose.orientation.w=q.w();

    //     this_pose_stamped.header.stamp=current_time;
    //     this_pose_stamped.header.frame_id="odom";
    //     path.poses.push_back(this_pose_stamped);


   nav_msgs::Odometry odom;
    odom.header.stamp = current_time;
    odom.header.frame_id = "odom";
 
    //set the position
    odom.pose.pose.position.x = 0;
    odom.pose.pose.position.y = 0;
    odom.pose.pose.position.z = 0.0;
    odom.pose.pose.orientation.x =q.x();
    odom.pose.pose.orientation.y =q.y();
    odom.pose.pose.orientation.z =q.z();
    odom.pose.pose.orientation.w =q.w();
    //set the velocity
    odom.child_frame_id = "base_link";
    odom.twist.twist.linear.x = 0;
    odom.twist.twist.linear.y = 0;
    odom.twist.twist.angular.z = 0;
        //path_pub.publish(path);
   
   
   
    odom_pub.publish(odom);
     //-- 验证E=t^R*scale
    // Mat t_x = ( Mat_<double> ( 3,3 ) <<
    //             0,                      -t.at<double> ( 2,0 ),     t.at<double> ( 1,0 ),
    //             t.at<double> ( 2,0 ),      0,                      -t.at<double> ( 0,0 ),
    //             -t.at<double> ( 1,0 ),     t.at<double> ( 0,0 ),      0 );

    // cout<<"t^R="<<endl<<t_x*R<<endl;

    // //-- 验证对极约束
    // Mat K = ( Mat_<double> ( 3,3 ) << 520.9, 0, 325.1, 0, 521.0, 249.7, 0, 0, 1 );
    // for ( DMatch m: matches )
    // {
    //     Point2d pt1 = pixel2cam ( keypoints_1[ m.queryIdx ].pt, K );
    //     Mat y1 = ( Mat_<double> ( 3,1 ) << pt1.x, pt1.y, 1 );
    //     Point2d pt2 = pixel2cam ( keypoints_2[ m.trainIdx ].pt, K );
    //     Mat y2 = ( Mat_<double> ( 3,1 ) << pt2.x, pt2.y, 1 );
    //     Mat d = y2.t() * t_x * R * y1;
    //     cout << "epipolar constraint = " << d << endl;
    // }
   }
   ros::spinOnce(); 
   last_time = current_time;
     loop_rate.sleep();   /* code for loop body */
 }
   
 
    return 0;
}

void find_feature_matches ( const Mat& img_1, const Mat& img_2,
                            std::vector<KeyPoint>& keypoints_1,
                            std::vector<KeyPoint>& keypoints_2,
                            std::vector< DMatch >& matches )
{
    //-- 初始化
    Mat descriptors_1, descriptors_2;
    // used in OpenCV3 
    Ptr<FeatureDetector> detector = ORB::create();
    Ptr<DescriptorExtractor> descriptor = ORB::create();
    // use this if you are in OpenCV2 
    // Ptr<FeatureDetector> detector = FeatureDetector::create ( "ORB" );
    // Ptr<DescriptorExtractor> descriptor = DescriptorExtractor::create ( "ORB" );
    Ptr<DescriptorMatcher> matcher  = DescriptorMatcher::create ( "BruteForce-Hamming" );
    //-- 第一步:检测 Oriented FAST 角点位置
    detector->detect ( img_1,keypoints_1 );
    detector->detect ( img_2,keypoints_2 );

    //-- 第二步:根据角点位置计算 BRIEF 描述子
    descriptor->compute ( img_1, keypoints_1, descriptors_1 );
    descriptor->compute ( img_2, keypoints_2, descriptors_2 );

    //-- 第三步:对两幅图像中的BRIEF描述子进行匹配，使用 Hamming 距离
    vector<DMatch> match;
    //BFMatcher matcher ( NORM_HAMMING );
    matcher->match ( descriptors_1, descriptors_2, match );

    //-- 第四步:匹配点对筛选
    double min_dist=10000, max_dist=0;

    //找出所有匹配之间的最小距离和最大距离, 即是最相似的和最不相似的两组点之间的距离
    for ( int i = 0; i < descriptors_1.rows; i++ )
    {
        double dist = match[i].distance;
        if ( dist < min_dist ) min_dist = dist;
        if ( dist > max_dist ) max_dist = dist;
    }

    printf ( "-- Max dist : %f \n", max_dist );
    printf ( "-- Min dist : %f \n", min_dist );

    //当描述子之间的距离大于两倍的最小距离时,即认为匹配有误.但有时候最小距离会非常小,设置一个经验值30作为下限.
    for ( int i = 0; i < descriptors_1.rows; i++ )
    {
        if ( match[i].distance <= max ( 2*min_dist, 30.0 ) )
        {
            matches.push_back ( match[i] );
        }
    }
     Mat img_goodmatch;

    drawMatches ( img_1, keypoints_1, img_2, keypoints_2, matches, img_goodmatch );

    imshow ( "优化后匹配点对", img_goodmatch );
    cv::waitKey(3);
}


Point2d pixel2cam ( const Point2d& p, const Mat& K )
{
    return Point2d
           (
               ( p.x - K.at<double> ( 0,2 ) ) / K.at<double> ( 0,0 ),
               ( p.y - K.at<double> ( 1,2 ) ) / K.at<double> ( 1,1 )
           );
}


void pose_estimation_2d2d ( std::vector<KeyPoint> keypoints_1,
                            std::vector<KeyPoint> keypoints_2,
                            std::vector< DMatch > matches,
                            Mat& R, Mat& t )
{
    // 相机内参,TUM Freiburg2
    Mat K = ( Mat_<double> ( 3,3 ) << 259.8074, 0, 328.0466, 0, 261.0161, 236.2571, 0, 0, 1 );

    //-- 把匹配点转换为vector<Point2f>的形式
    vector<Point2f> points1;
    vector<Point2f> points2;

    for ( int i = 0; i < ( int ) matches.size(); i++ )
    {
        points1.push_back ( keypoints_1[matches[i].queryIdx].pt );
        points2.push_back ( keypoints_2[matches[i].trainIdx].pt );
    }

    //-- 计算基础矩阵
    Mat fundamental_matrix;
    fundamental_matrix = findFundamentalMat ( points1, points2, CV_FM_8POINT );
   // cout<<"fundamental_matrix is "<<endl<< fundamental_matrix<<endl;

    //-- 计算本质矩阵
    Point2d principal_point ( 325.1, 249.7 );	//相机光心, TUM dataset标定值
    double focal_length = 521;			//相机焦距, TUM dataset标定值
    Mat essential_matrix;
    essential_matrix = findEssentialMat ( points1, points2, focal_length, principal_point );
   // cout<<"essential_matrix is "<<endl<< essential_matrix<<endl;

    //-- 计算单应矩阵
    Mat homography_matrix;
    homography_matrix = findHomography ( points1, points2, RANSAC, 3 );
  //  cout<<"homography_matrix is "<<endl<<homography_matrix<<endl;

    //-- 从本质矩阵中恢复旋转和平移信息.
    recoverPose ( essential_matrix, points1, points2, R, t, focal_length, principal_point );
    // Eigen::Matrix3d ER;
    // ER<<R.at<double> ( 0,0 ),R.at<double> ( 0,1 ),R.at<double> ( 0,2 ),R.at<double> ( 1,0 ),R.at<double> ( 1,1 ),R.at<double> ( 1,2 ),R.at<double> ( 2,0 ),R.at<double> ( 2,1 ),R.at<double> ( 2,2 );
    //  Eigen::Quaterniond q = Eigen::Quaterniond(ER);
    cout<<"R is "<<endl<<R<<endl;
    cout<<"t is "<<endl<<t<<endl;
    // cout<<"ER is "<<endl<<q.coeffs()<<endl;
    
    
}