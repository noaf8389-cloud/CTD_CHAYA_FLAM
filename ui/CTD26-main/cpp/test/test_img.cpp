#include "test_framework.hpp"
#include "../src/img.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
std::string make_temp_png(int w, int h, int channels = 4) {
    fs::path path = fs::temp_directory_path() / "kfc_test_img.png";
    cv::Mat img(h, w, channels == 4 ? CV_8UC4 : CV_8UC3, cv::Scalar(10, 20, 30, 255));
    cv::imwrite(path.string(), img);
    return path.string();
}
}

TEST(img_is_not_loaded_before_read) {
    Img img;
    EXPECT_TRUE(!img.is_loaded());
}

TEST(img_read_loads_the_file) {
    Img img;
    img.read(make_temp_png(20, 20));
    EXPECT_TRUE(img.is_loaded());
}

TEST(img_read_throws_on_missing_file) {
    Img img;
    EXPECT_THROWS(img.read("this/path/does/not/exist.png"));
}

TEST(img_read_resizes_without_keeping_aspect) {
    Img img;
    img.read(make_temp_png(20, 40), {10, 10}, false);
    EXPECT_EQ(img.get_mat().cols, 10);
    EXPECT_EQ(img.get_mat().rows, 10);
}

TEST(img_read_resizes_keeping_aspect) {
    Img img;
    img.read(make_temp_png(20, 40), {10, 10}, true);
    EXPECT_EQ(img.get_mat().cols, 5);
    EXPECT_EQ(img.get_mat().rows, 10);
}

TEST(img_draw_on_throws_when_source_not_loaded) {
    Img src;
    Img dst;
    dst.read(make_temp_png(20, 20));
    EXPECT_THROWS(src.draw_on(dst, 0, 0));
}

TEST(img_draw_on_throws_when_it_does_not_fit) {
    Img src;
    src.read(make_temp_png(50, 50));
    Img dst;
    dst.read(make_temp_png(20, 20));
    EXPECT_THROWS(src.draw_on(dst, 0, 0));
}

TEST(img_draw_on_copies_pixels_for_matching_channel_count) {
    Img src;
    src.read(make_temp_png(10, 10));
    Img dst;
    dst.read(make_temp_png(20, 20));
    src.draw_on(dst, 5, 5);
    cv::Vec4b pixel = dst.get_mat().at<cv::Vec4b>(6, 6);
    EXPECT_EQ(pixel[0], 10);
    EXPECT_EQ(pixel[1], 20);
    EXPECT_EQ(pixel[2], 30);
}

TEST(img_draw_on_converts_3_channel_source_onto_4_channel_dest) {
    Img src;
    src.read(make_temp_png(10, 10, 3));
    Img dst;
    dst.read(make_temp_png(20, 20, 4));
    EXPECT_NO_THROW(src.draw_on(dst, 5, 5));
    cv::Vec4b pixel = dst.get_mat().at<cv::Vec4b>(6, 6);
    EXPECT_EQ(pixel[0], 10);
    EXPECT_EQ(pixel[1], 20);
    EXPECT_EQ(pixel[2], 30);
}

TEST(img_put_text_throws_when_not_loaded) {
    Img img;
    EXPECT_THROWS(img.put_text("hi", 0, 0, 1.0));
}

TEST(img_put_text_succeeds_when_loaded) {
    Img img;
    img.read(make_temp_png(50, 50));
    EXPECT_NO_THROW(img.put_text("hi", 5, 5, 1.0));
}
