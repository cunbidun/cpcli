#include "testlib.h"

using namespace std;

int main(int argc, char *argv[]) {
  registerTestlibCmd(argc, argv);
  if (argv[3] == std::string("___test_case/___na___")) {
    quitf(_fail, "undecided");
  }

  int i = 0;
  while (!ans.seekEof() && !ouf.seekEof()) {
    ++i;
    string p = ouf.readToken(), j = ans.readToken();
    if (j != p) {
      quitf(_wa, "%d%s words differ - expected: '%s', found: '%s'", i,
            englishEnding(i).c_str(),
            compress(j).c_str(),
            compress(p).c_str());
    }
  }
  if (!ans.seekEof() || !ouf.seekEof()) {
    quitf(_wa, "number of token differs");
  }
  quitf(_ok, "passed");
}
