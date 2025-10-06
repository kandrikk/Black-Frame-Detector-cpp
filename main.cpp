#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <filesystem>

// Проверка существования файла
bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

// Форматирование временной метки в формат ЧЧ:ММ:СС.ммм
std::string formatTimestamp(double seconds, double fps) {
    int totalSeconds = static_cast<int>(seconds);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;
    
    double fractionalPart = seconds - totalSeconds;
    int milliseconds = static_cast<int>(fractionalPart * 1000);
    int frame = static_cast<int>(std::round(milliseconds / (1000.0 / fps)));
    
    std::ostringstream oss;
    oss << "time: "
        << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << secs << "."
        << frame;
    
    return oss.str();
}

// Поиск черных кадров в видео
std::string findBlackFrames(const std::string& videoPath, double threshold = 5.0) {
    // Проверяем существование файла
    if (!fileExists(videoPath)) {
        return "❌ Файл " + videoPath + " не найден!";
    }
    
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        return "❌ Не удалось открыть видеофайл";
    }
    
    // Получаем информацию о видео
    double fps = cap.get(cv::CAP_PROP_FPS);
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    
    std::ostringstream result;
    result << std::fixed << std::setprecision(2);
    result << "FPS: " << fps << ", Кадров: " << totalFrames << "\n\n";
    
    struct BlackSequence {
        double startTime;
        double endTime;
    };
    
    std::vector<BlackSequence> blackFramesTime;
    std::optional<BlackSequence> currentSequence = std::nullopt;
    
    cv::Mat frame;
    for (int frameCount = 0; frameCount < totalFrames; ++frameCount) {
        if (!cap.read(frame)) {
            break;
        }
        
        // Проверяем, является ли кадр черным
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
                blackFramesTime.push_back(currentSequence.value());
                currentSequence.reset();
            }
        }
    }
    
    // Добавляем последнюю последовательность, если видео заканчивается на черном кадре
    if (currentSequence.has_value()) {
        blackFramesTime.push_back(currentSequence.value());
    }
    
    // Формируем результат
    if (blackFramesTime.empty()) {
        result << "✅ Черные кадры не обнаружены";
    } else {
        result << "🔍 Найдено последовательностей: " << blackFramesTime.size() << "\n\n";
        for (size_t i = 0; i < blackFramesTime.size(); ++i) {
            std::string start = formatTimestamp(blackFramesTime[i].startTime, fps);
            std::string end = formatTimestamp(blackFramesTime[i].endTime, fps);
            result << (i + 1) << ": " << start << " - " << end << "\n";
        }
    }
    
    cap.release();
    return result.str();
}

int main(int argc, char* argv[]) {
    std::string videoPath;

    if (argc == 2) {
        std::cerr << "Использование: " << argv[0] << " <путь_к_видеофайлу>\n";
        videoPath = argv[1];
    } else {
        std::cout << "Введите путь к видеофайлу: ";
        std::cin >> videoPath;
    }

    std::string result = findBlackFrames(videoPath);
    std::cout << result << std::endl;
    
    return 0;
}