#include "piece_assets.hpp"
#include "BoardView.hpp"
#include "../../img.hpp"
#include <algorithm>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

PieceAssets::PieceAssets(const std::string& theme_root) {
    for (const auto& code_entry : fs::directory_iterator(theme_root)) {
        if (!code_entry.is_directory()) continue;
        std::string code = code_entry.path().filename().string();
        fs::path states_dir = code_entry.path() / "states";
        if (!fs::exists(states_dir)) continue;

        for (const auto& state_entry : fs::directory_iterator(states_dir)) {
            if (!state_entry.is_directory()) continue;
            std::string state = state_entry.path().filename().string();

            fs::path sprites_dir = state_entry.path() / "sprites";
            fs::path config_path = state_entry.path() / "config.json";
            if (!fs::exists(sprites_dir) || !fs::exists(config_path)) continue;

            AnimationClip clip;
            clip.frames = load_frames(sprites_dir.string());
            load_state_config(config_path.string(), clip);
            if (clip.frames.empty() || clip.frames_per_sec <= 0.0) continue;

            clips[code][state] = std::move(clip);
        }
    }

    if (clips.empty()) {
        throw std::runtime_error("No piece animations found under: " + theme_root);
    }
}

std::vector<cv::Mat> PieceAssets::load_frames(const std::string& sprites_dir) const {
    std::vector<fs::path> frame_paths;
    for (const auto& entry : fs::directory_iterator(sprites_dir)) {
        frame_paths.push_back(entry.path());
    }
    std::sort(frame_paths.begin(), frame_paths.end(),
              [](const fs::path& a, const fs::path& b) {
                  return std::stoi(a.stem().string()) < std::stoi(b.stem().string());
              });

    std::vector<cv::Mat> frames;
    for (const auto& path : frame_paths) {
        Img loader;
        loader.read(path.string(), {BoardView::kCellPx, BoardView::kCellPx}, true);
        frames.push_back(loader.get_mat().clone());
    }
    return frames;
}

void PieceAssets::load_state_config(const std::string& config_path, AnimationClip& clip) const {
    cv::FileStorage config(config_path, cv::FileStorage::READ);
    if (!config.isOpened()) {
        throw std::runtime_error("Could not open piece config: " + config_path);
    }
    config["graphics"]["frames_per_sec"] >> clip.frames_per_sec;
    config["physics"]["next_state_when_finished"] >> clip.next_state;
}

int PieceAssets::frame_index_for(const AnimationClip& clip, double time_in_state) const {
    int frame_count = static_cast<int>(clip.frames.size());
    int raw_index = static_cast<int>(time_in_state * clip.frames_per_sec);
    return raw_index % frame_count;
}

const PieceAssets::AnimationClip& PieceAssets::clip_for(const std::string& code, const std::string& state) const {
    auto code_it = clips.find(code);
    if (code_it == clips.end()) {
        throw std::runtime_error("Unknown piece code: " + code);
    }
    auto state_it = code_it->second.find(state);
    if (state_it == code_it->second.end()) {
        throw std::runtime_error("Unknown state '" + state + "' for piece code: " + code);
    }
    return state_it->second;
}

const cv::Mat& PieceAssets::current_image_for(const std::string& code, const std::string& state, double time_in_state) const {
    const AnimationClip& clip = clip_for(code, state);
    return clip.frames[frame_index_for(clip, time_in_state)];
}

bool PieceAssets::is_state_finished(const std::string& code, const std::string& state, double time_in_state) const {
    const AnimationClip& clip = clip_for(code, state);
    double cycle_duration = clip.frames.size() / clip.frames_per_sec;
    return time_in_state >= cycle_duration;
}

std::string PieceAssets::next_state_for(const std::string& code, const std::string& state) const {
    return clip_for(code, state).next_state;
}
