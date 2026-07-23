#include "test_framework.hpp"
#include "../src/game/graphics/piece_assets.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {
fs::path make_theme(const std::string& name) {
    fs::path root = fs::temp_directory_path() / name;
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

// כל פריים i מקבל צבע אחיד בערך (i-1) - כדי שאפשר יהיה "לזהות" איזה
// פריים הוחזר, בלי צורך בהשוואת תמונה מלאה.
void make_piece_state(const fs::path& theme_root, const std::string& code, const std::string& state,
                       int frame_count, double fps, const std::string& next_state,
                       int w = 40, int h = 40) {
    fs::path state_dir = theme_root / code / "states" / state;
    fs::path sprites_dir = state_dir / "sprites";
    fs::create_directories(sprites_dir);

    for (int i = 1; i <= frame_count; ++i) {
        cv::Mat frame(h, w, CV_8UC3, cv::Scalar(i - 1, i - 1, i - 1));
        cv::imwrite((sprites_dir / (std::to_string(i) + ".png")).string(), frame);
    }

    std::ofstream config(state_dir / "config.json");
    config << "{\"graphics\": {\"frames_per_sec\": " << fps << ", \"is_loop\": true},"
           << " \"physics\": {\"next_state_when_finished\": \"" << next_state << "\"}}";
}
}

TEST(piece_assets_skips_non_piece_folders) {
    fs::path theme = make_theme("kfc_test_theme_skip");
    make_piece_state(theme, "AA", "idle", 1, 1.0, "idle");
    fs::create_directories(theme / "not_a_piece");   // בלי states/ בכלל

    EXPECT_NO_THROW(PieceAssets(theme.string()));
}

TEST(piece_assets_skips_state_without_config) {
    fs::path theme = make_theme("kfc_test_theme_no_config");
    make_piece_state(theme, "VALID", "idle", 1, 1.0, "idle");   // כדי שהבנאי לא ייכשל לגמרי

    fs::path sprite_dir = theme / "VALID" / "states" / "nocfg" / "sprites";
    fs::create_directories(sprite_dir);
    cv::imwrite((sprite_dir / "1.png").string(), cv::Mat(10, 10, CV_8UC3, cv::Scalar(1, 2, 3)));
    // בכוונה בלי config.json למצב "nocfg"

    PieceAssets assets(theme.string());
    EXPECT_THROWS(assets.current_image_for("VALID", "nocfg", 0.0));
}

TEST(piece_assets_throws_for_unknown_code) {
    fs::path theme = make_theme("kfc_test_theme_unknown_code");
    make_piece_state(theme, "AA", "idle", 1, 1.0, "idle");
    PieceAssets assets(theme.string());

    EXPECT_THROWS(assets.current_image_for("ZZ", "idle", 0.0));
}

TEST(piece_assets_throws_for_unknown_state) {
    fs::path theme = make_theme("kfc_test_theme_unknown_state");
    make_piece_state(theme, "AA", "idle", 1, 1.0, "idle");
    PieceAssets assets(theme.string());

    EXPECT_THROWS(assets.current_image_for("AA", "jump", 0.0));
}

TEST(piece_assets_throws_when_theme_has_no_valid_pieces) {
    fs::path theme = make_theme("kfc_test_theme_empty");
    fs::create_directories(theme / "junk");

    EXPECT_THROWS(PieceAssets(theme.string()));
}

TEST(piece_assets_keeps_aspect_ratio) {
    fs::path theme = make_theme("kfc_test_theme_aspect");
    make_piece_state(theme, "TALL", "idle", 1, 1.0, "idle", 40, 80);   // 1:2, לא ריבועי

    PieceAssets assets(theme.string());
    const cv::Mat& img = assets.current_image_for("TALL", "idle", 0.0);
    EXPECT_TRUE(img.cols != img.rows);
}

TEST(piece_assets_selects_frame_by_time_in_state) {
    fs::path theme = make_theme("kfc_test_theme_frames");
    make_piece_state(theme, "AA", "idle", 4, 2.0, "idle");   // 4 פריימים, 2 fps -> 0.5 שנייה לפריים
    PieceAssets assets(theme.string());

    EXPECT_EQ(assets.current_image_for("AA", "idle", 0.0).at<cv::Vec3b>(0, 0)[0], 0);
    EXPECT_EQ(assets.current_image_for("AA", "idle", 0.4).at<cv::Vec3b>(0, 0)[0], 0);
    EXPECT_EQ(assets.current_image_for("AA", "idle", 0.5).at<cv::Vec3b>(0, 0)[0], 1);
    EXPECT_EQ(assets.current_image_for("AA", "idle", 1.5).at<cv::Vec3b>(0, 0)[0], 3);
}

TEST(piece_assets_loops_frame_selection) {
    fs::path theme = make_theme("kfc_test_theme_loop");
    make_piece_state(theme, "AA", "idle", 4, 2.0, "idle");   // מחזור מלא = 2 שניות
    PieceAssets assets(theme.string());

    EXPECT_EQ(assets.current_image_for("AA", "idle", 2.0).at<cv::Vec3b>(0, 0)[0], 0);
    EXPECT_EQ(assets.current_image_for("AA", "idle", 2.5).at<cv::Vec3b>(0, 0)[0], 1);
}

TEST(piece_assets_loads_multiple_states_independently) {
    fs::path theme = make_theme("kfc_test_theme_multi_state");
    make_piece_state(theme, "AA", "idle", 1, 1.0, "idle");
    make_piece_state(theme, "AA", "move", 1, 5.0, "long_rest");
    PieceAssets assets(theme.string());

    EXPECT_NO_THROW(assets.current_image_for("AA", "idle", 0.0));
    EXPECT_NO_THROW(assets.current_image_for("AA", "move", 0.0));
    EXPECT_EQ(assets.next_state_for("AA", "move"), std::string("long_rest"));
}

TEST(piece_assets_is_state_finished_respects_cycle_duration) {
    fs::path theme = make_theme("kfc_test_theme_cycle");
    make_piece_state(theme, "AA", "jump", 2, 4.0, "short_rest");   // 2 פריימים / 4fps = 0.5 שנייה
    PieceAssets assets(theme.string());

    EXPECT_TRUE(!assets.is_state_finished("AA", "jump", 0.49));
    EXPECT_TRUE(assets.is_state_finished("AA", "jump", 0.5));
}

TEST(piece_assets_next_state_for_returns_configured_value) {
    fs::path theme = make_theme("kfc_test_theme_next_state");
    make_piece_state(theme, "AA", "short_rest", 1, 1.0, "idle");
    PieceAssets assets(theme.string());

    EXPECT_EQ(assets.next_state_for("AA", "short_rest"), std::string("idle"));
}
