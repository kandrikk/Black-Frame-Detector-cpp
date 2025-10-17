#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>
#include <cmath>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <QString>
#include <functional>

std::string formatTimestamp(double second, double fps);
std::string findBlackFrames(const std::string& videoPath, double threshold = 5, 
                                std::function<void(int, int)> progressCallback = nullptr);
std::vector<std::string> infoVideoFile(const std::string& videoPath);