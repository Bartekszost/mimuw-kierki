#include <gtest/gtest.h>
#include <regex>

#include "network-common.h"

TEST(CurrentTimeToStringTest, ReturnsCorrectFormat)
{
    std::string time_str = current_time_to_string();

    std::regex time_format("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}");
    std::cout << time_str << '\n';
    ASSERT_TRUE(std::regex_match(time_str, time_format));
}