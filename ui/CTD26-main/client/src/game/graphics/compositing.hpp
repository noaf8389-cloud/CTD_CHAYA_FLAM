#pragma once
#include <opencv2/opencv.hpp>

// Draws src onto dst at (x, y), honoring src's alpha channel so transparent pixels don't overwrite dst.
void blit_with_alpha(cv::Mat& dst, const cv::Mat& src, int x, int y);
