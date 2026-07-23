#include "compositing.hpp"
#include <vector>

void blit_with_alpha(cv::Mat& dst, const cv::Mat& src, int x, int y) {
    cv::Mat roi = dst(cv::Rect(x, y, src.cols, src.rows));
    
    cv::Mat source = src;
    if (source.channels() == 3 && dst.channels() == 4) {
        cv::cvtColor(source, source, cv::COLOR_BGR2BGRA);
    } else if (source.channels() == 4 && dst.channels() == 3) {
        cv::cvtColor(source, source, cv::COLOR_BGRA2BGR);
    }

    if (source.channels() == 4) {
        std::vector<cv::Mat> channels;
        cv::split(source, channels);
        source.copyTo(roi, channels[3]);
    } else {
        source.copyTo(roi);
    }
}
