# CMakeLists.txt in your tests directory

cmake_minimum_required(VERSION 3.10)
project(YourTestProjectName)

# Locate Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Add the source files
file(GLOB SOURCE_FILES "../*.cpp")

# Get the absolute path to kierki-serwer.cpp
file(GLOB_RECURSE MAIN_FILE_TO_REMOVE_SERWER "../kierki-serwer.cpp")

# Get the absolute path to kierki-klient.cpp
file(GLOB_RECURSE MAIN_FILE_TO_REMOVE_KLIENT "../kierki-klient.cpp")

# Remove the source files with the main() function
list(REMOVE_ITEM SOURCE_FILES ${MAIN_FILE_TO_REMOVE_SERWER})
list(REMOVE_ITEM SOURCE_FILES ${MAIN_FILE_TO_REMOVE_KLIENT})

# Add the test files
file(GLOB TEST_SRC "*.cpp")

# Create a list with all the source files
list(APPEND ALL_FILES ${SOURCE_FILES} ${TEST_SRC})

# Create an executable based on the source files
add_executable(runTests ${ALL_FILES} client_game_state_test.cpp network_common_test.cpp)

# Link with Google Test
target_link_libraries(runTests ${GTEST_BOTH_LIBRARIES})

# Link with the necessary libraries
target_link_libraries(runTests ${REQUIRED_LIBS})