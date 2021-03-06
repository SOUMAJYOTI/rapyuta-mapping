/*
 * test_diem.cpp
 *
 *  Created on: Jul 29, 2013
 *      Author: vsu
 */

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <keyframe_map.h>

int main(int argc, char **argv) {
	keyframe_map map1, map2;

	map1.load("corridor_map3");
	map2.load("corridor_map4");

	//cv::imshow("img1", map1.get_panorama_image());
	//cv::imshow("img2", map2.get_panorama_image());
	//cv::waitKey();

	int iteration = 0;
	Sophus::SE3f transform;
	while(!map1.find_transform(map2, transform)) {
		std::cout << iteration << std::endl;
		iteration++;
	}

	map1.merge(map2, transform);
	map1.save("keyframe_map_merged");

	return 0;

}
