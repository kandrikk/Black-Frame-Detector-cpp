#include "../include/BlackFrameDetector.h"

std::string formatTimestamp(double second, double fps) {
    int totalSecond = static_cast<int>(second);
    int hours = totalSecond / 3600;
    int minutes = (totalSecond % 3600) / 60;
    int secs = totalSecond % 60;

    int milliseconds = static_cast<int>((second - totalSecond) * 1000);
    int frame = static_cast<int>(std::round(milliseconds / (1000 / fps)));

    int last;

    if (fps == 0) last = milliseconds;
    else last = fps;

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << secs << "."
        << std::setw(2) << last;


    return oss.str();
}

std::vector<std::string> infoVideoFile(const std::string& videoPath) {
    if (!std::filesystem::exists(videoPath)) return {"File not found."};

    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) return {"Failed to open file."};

    std::string filename = std::filesystem::path(videoPath).filename().string();
    double fps = cap.get(cv::CAP_PROP_FPS);
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    std::string time = formatTimestamp(std::round(totalFrames / fps), fps);
    
    return {filename, time, std::to_string(fps), std::to_string(totalFrames)};
}

std::string findBlackFrames(const std::string& videoPath, double threshold, 
                                std::function<void(int)> progressCallback) {
    std::string result;

    if (!std::filesystem::exists(videoPath)) {
        return "File not found.";
    }

    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        return "Failed to open file.";
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    struct BlackSequence {
        double startTime;
        double endTime;
    };

    std::vector<BlackSequence> blackFrames;
    std::optional<BlackSequence> currentSequence = std::nullopt;

    cv::Mat frame;
    for(int frameCount = 0; frameCount < totalFrames; ++frameCount) {
        if (!cap.read(frame)) {
            break;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::Scalar meanVal = cv::mean(gray);
        double avgBrightness = meanVal[0];

        double timestamp = static_cast<double>(frameCount) / fps;

        if (avgBrightness < threshold) {
            if (!currentSequence.has_value()) {
                currentSequence = BlackSequence{timestamp, timestamp};
            } else {
                currentSequence->endTime = timestamp;
            }

        } else {
            if (currentSequence.has_value()) {
                blackFrames.push_back(currentSequence.value());
                currentSequence.reset();
            }
        }

            // 🔥 Вызов прогресса
        if (progressCallback && totalFrames > 0) {
            // Оптимизация: не обновлять слишком часто
            if (frameCount % std::max(1, totalFrames / 200) == 0) {
                progressCallback(frameCount);
            }
        }
    }

    if (progressCallback && totalFrames > 0) {
        progressCallback(totalFrames);
    }

    if (currentSequence.has_value()) {
        blackFrames.push_back(currentSequence.value());
        currentSequence.reset();
    }

    if (blackFrames.empty()) {
        result = "No black frames found.";
    } else {
        result = "Sequences found: " + std::to_string(blackFrames.size()) 
        + "\n"
        + "\n";

        for (size_t i = 0; i < blackFrames.size(); ++i) {
            std::string start = formatTimestamp(blackFrames[i].startTime, fps);
            std::string end = formatTimestamp(blackFrames[i].endTime, fps);
            result = result + std::to_string(i + 1) + ". " + start + " - " + end + "\n";
        }
    }

    cap.release();

    return result;
}