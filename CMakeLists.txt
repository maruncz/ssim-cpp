cmake_minimum_required(VERSION 3.5)

project(ssim-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Armadillo REQUIRED)

add_executable(ssim-cpp ssim-cpp.cpp)
target_include_directories(ssim-cpp PUBLIC ${OpenCV_INCLUDE_DIRS} ${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(ssim-cpp PRIVATE ${OpenCV_LIBS} ${ARMADILLO_LIBRARIES})
