#include <gtest/gtest.h>

#include "cpcli_process.hpp"

TEST(TestMain, TestMainNoArg) {
  char *argv[] = {(char *)"cpcli_app", NULL};
  cpcli_process(1, argv);
}

TEST(TestMain, TestMainOneDummyArgc) {
  char *argv[] = {(char *)"cpcli_app", (char *)"dummy_arg", NULL};
  cpcli_process(2, argv);
}

TEST(TestMain, TestMainVersion) {
  char *argv[] = {(char *)"cpcli_app",  (char *)"--help", NULL};
  EXPECT_EQ(cpcli_process(2, argv), 0);
}