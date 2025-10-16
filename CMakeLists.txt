cmake_minimum_required(VERSION 3.16)
project(Black-Frame-Detector-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Сначала найдём все зависимости
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)
find_package(OpenCV REQUIRED)

qt_standard_project_setup()

# Создаём цель — имя должно совпадать везде!
qt_add_executable(Black-Frame-Detector-cpp src/main.cpp src/BlackFrameDetector.cpp)

# Подключаем библиотеки к ТОЙ ЖЕ цели
target_link_libraries(Black-Frame-Detector-cpp PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    ${OpenCV_LIBS}
)

# Подключаем заголовки OpenCV
target_include_directories(Black-Frame-Detector-cpp PRIVATE ${OpenCV_INCLUDE_DIRS})