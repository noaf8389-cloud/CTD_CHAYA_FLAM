#include "test_framework.hpp"
#include "../src/game/graphics/compositing.hpp"
#include <opencv2/opencv.hpp>

TEST(blit_3channel_source_writes_into_4channel_dest_in_place) {
    cv::Mat dst(200, 200, CV_8UC4, cv::Scalar(0, 0, 0, 255));
    cv::Mat src(50, 50, CV_8UC3, cv::Scalar(200, 100, 50));

    blit_with_alpha(dst, src, 10, 10);

    cv::Vec4b pixel = dst.at<cv::Vec4b>(30, 30);
    EXPECT_EQ(pixel[0], 200);
    EXPECT_EQ(pixel[1], 100);
    EXPECT_EQ(pixel[2], 50);
}

TEST(blit_respects_transparent_pixels_in_4channel_source) {
    cv::Mat dst(200, 200, CV_8UC4, cv::Scalar(1, 2, 3, 255));
    cv::Mat src(50, 50, CV_8UC4, cv::Scalar(200, 100, 50, 255));
    src.at<cv::Vec4b>(0, 0) = cv::Vec4b(200, 100, 50, 0);   // פינה שקופה

    blit_with_alpha(dst, src, 0, 0);

    EXPECT_EQ(dst.at<cv::Vec4b>(0, 0)[0], 1);     // שקוף - נשאר כמו שהיה
    EXPECT_EQ(dst.at<cv::Vec4b>(30, 30)[0], 200);  // אטום - נדרס
}
