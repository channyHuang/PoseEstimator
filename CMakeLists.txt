CMAKE_MINIMUM_REQUIRED(VERSION 3.12..6.0)

Project(PoseEstimator)

# dependency
find_package(colmap REQUIRED)
find_package(Eigen3 3.4 QUIET)
message(STATUS "Eigen3_VERSION: ${Eigen3_VERSION}")

# Check C++20 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  add_definitions(-DCOMPILEDWITHC20)
  message(STATUS "Using flag -std=c++20.")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

if(TARGET Eigen3::Eigen)
else()
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/eigen3
    )
endif()

FILE(GLOB FILES_C "*.cpp" "*.cc")
FILE(GLOB FILES_H "*.h" "*.hpp" "*.inl")

add_executable(${PROJECT_NAME}
    ${FILES_C}
    ${FILES_H}
)

target_link_libraries(${PROJECT_NAME} 
	colmap::colmap

    Geographic
)

if(TARGET Eigen3::Eigen)
    target_link_libraries(${PROJECT_NAME}
        Eigen3::Eigen
    )
endif()
