#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>
#include <cmath>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <QString>

std::string formatTimestamp(double second, double fps);
std::string findBlackFrames(const std::string& videoPath, double threshold);