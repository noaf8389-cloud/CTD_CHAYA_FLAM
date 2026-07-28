#pragma once
#include <opencv2/opencv.hpp>
#include <map>
#include <string>
#include <vector>

class PieceAssets {
public:
    // Loads every piece's sprite frames and per-state config from the given theme folder.
    explicit PieceAssets(const std::string& theme_root);

    // Returns the sprite frame a piece/state should show at the given elapsed time.
    const cv::Mat& current_image_for(const std::string& code, const std::string& state, double time_in_state) const;
    // Checks whether a (non-looping) animation state has played through all its frames.
    bool is_state_finished(const std::string& code, const std::string& state, double time_in_state) const;
    // Returns the state a piece should transition to once its current animation finishes.
    std::string next_state_for(const std::string& code, const std::string& state) const;

private:
    struct AnimationClip {
        std::vector<cv::Mat> frames;
        double frames_per_sec = 0.0;
        std::string next_state;
    };

    std::vector<cv::Mat> load_frames(const std::string& sprites_dir) const;
    void load_state_config(const std::string& config_path, AnimationClip& clip) const;
    int frame_index_for(const AnimationClip& clip, double elapsed_seconds) const;
    const AnimationClip& clip_for(const std::string& code, const std::string& state) const;

    std::map<std::string, std::map<std::string, AnimationClip>> clips;
};
