#include <gtest/gtest.h>
#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan.log");
#endif
    return RUN_ALL_TESTS();
}