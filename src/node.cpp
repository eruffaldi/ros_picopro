#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <sys/stat.h>

#include <ros/ros.h>
#include <sensor_msgs/image_encodings.h>

#include <image_transport/image_transport.h>
#include <camera_info_manager/camera_info_manager.h>

#include <ros/package.h>

#include <opencv2/opencv.hpp>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "am7xxx.h"
#include "yuv422_to_nv12.h"

struct Am7xxxScope
{
	Am7xxxScope	()
	{
		inited_ = am7xxx_init(&ctx) >= 0;
	}

	operator bool() const { return inited_; }
	~Am7xxxScope()
	{	
		if(inited_)
		{
			am7xxx_shutdown(ctx);
		}
	}

	bool inited_;
	am7xxx_context *ctx = 0;
};

struct Am7xxxScopeApp: public Am7xxxScope
{

	Am7xxxScopeApp():
		private_node_handle_("~")
	{
		if(!*this)
			return;


		// init
	    private_node_handle_.param("device_index", device_index, 0);
	    private_node_handle_.param("power", power_mode, AM7XXX_POWER_LOW);
	    private_node_handle_.param("zoom", zoom, AM7XXX_ZOOM_ORIGINAL);
	    private_node_handle_.param("format", format, AM7XXX_IMAGE_FORMAT_JPEG);
	    private_node_handle_.param("width", width, 800);
	    private_node_handle_.param("height", height, 480);

		sub = it.subscribe("/image", 1, boost::bind(&Am7xxxScopeApp::imageCallback, this, _1));

		am7xxx_set_log_level(ctx, (am7xxx_log_level)log_level);

		int ret = am7xxx_open_device(ctx, &dev, device_index);
		if(ret < 0)
		{
			return ;
		}
		ret = am7xxx_get_device_info(dev, &device_info);
		if (ret < 0) 
		{
			return ;
		}
		printf("Native resolution: %dx%d\n",
		       device_info.native_width, device_info.native_height);

		ret = am7xxx_set_zoom_mode(dev, (am7xxx_zoom_mode)zoom);
		if (ret < 0) 
		{
			return ;
		}

		ret = am7xxx_set_power_mode(dev, (am7xxx_power_mode)power_mode);
		if (ret < 0) 
		{
			return ;
		}
	}
	/*
	 Reference
	  
	 https://github.com/opencv/opencv/blob/master/modules/imgproc/src/color.cpp
	 https://github.com/ros-perception/vision_opencv/blob/kinetic/cv_bridge/src/cv_bridge.cpp
	 https://github.com/ros-perception/image_transport_plugins/blob/indigo-devel/compressed_image_transport/src/compressed_subscriber.cpp

	 OpenCV and ROS support only YUV422 as it is in some cameras and JPEG
	 Half horizontal resolution for chrominance
	 NV12 is planar Y then packed UV (LSB U, MSB V)
	 */

	void imageCallback(const sensor_msgs::ImageConstPtr& msg)
	{
		// check empty image => black

		// resize => jpeg if format ...
		// hard AM7XXX_IMAGE_FORMAT_NV12
		if(format == AM7XXX_IMAGE_FORMAT_JPEG)
		{
			namespace enc = sensor_msgs::image_encodings;
			cv_bridge::toCvShare(msg, enc::BGR8);
			// TODO encode
			int ret = am7xxx_send_image(dev, format, width, height, image, (unsigned int)size);
		}
		else
		{

			// NV12 is planar: Y full, CbCr half all dim
			// CV_RGB2YCrCb => as JPEG
			// hal::cvtBGRtoYUV(src.data, src.step, dst.data, dst.step, src.cols, src.rows,
  	        //                   depth, scn, swapBlue(code), code == CV_BGR2YCrCb || code == CV_RGB2YCrCb);

  	       	// usa: RGB2YCrCb_i

			// NV12 but without libav/ffmpeg
			// https://github.com/cohenrotem/Rgb2NV12
			// NV12 is a quasi-planar format and has all the Y components first in the memory, followed by an array of UV packed components. 
			// For a 640x480 NV12 image, the layout is as follows:
			// UV
		}
	}


	~Am7xxxScopeApp()
	{
		int ret = am7xxx_close_device(dev);
	}

	am7xxx_device *dev;
	image_transport::Subscriber sub;
	am7xxx_device_info device_info;
    ros::NodeHandle node_handle_;
    ros::NodeHandle private_node_handle_;
	image_transport::ImageTransport it(node_handle_);
	int log_level = AM7XXX_LOG_INFO;
	int device_index = 0;
	int power_mode = AM7XXX_POWER_LOW;
	int zoom = AM7XXX_ZOOM_ORIGINAL;
	int format = AM7XXX_IMAGE_FORMAT_JPEG;
	int width = 800;
	int height = 480;
	unsigned char *image;
	off_t size;
};



int main(int argc, char const *argv[])
{
	ros::init(argc, argv, "ros_picopro");
  	signal(SIGINT, [](int sig) { ros::shutdown(); });
	Am7xxxScopeApp node;

	if(!node)
		return 0;

	ros::spin();

	return 0;
}