#include <gtest/gtest.h>

#include "cpcli.hpp"

TEST(TestMain, TestMainNoArg) {
  char *argc[] = {(char *)"cpcli_app", NULL};
  cpcli_process(1, argc);
}

TEST(TestMain, TestMainOneDummyArgc) {
  char *argc[] = {(char *)"cpcli_app", (char *)"dummy_arg", NULL};
  cpcli_process(2, argc);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
