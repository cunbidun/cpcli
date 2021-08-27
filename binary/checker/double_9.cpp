#include "testlib.h"
#include <cmath>

using namespace std;

const double EPS = 1E-9;

int main(int argc, char *argv[]) {
  setName("compare two sequences of doubles, max absolute or relative error = %.10f", EPS);
  registerTestlibCmd(argc, argv);
  if (argv[2] == std::string("___test_case/___na___")) {
    quitf(_fail, "undecided");
  }

  int i = 0;
  while (!ans.seekEof() && !ouf.seekEof()) {
    ++i;
    double p = ouf.readDouble(), j = ans.readDouble();
    if (!doubleCompare(j, p, EPS)) {
      quitf(
          _wa,
          "%d%s numbers differ - expected: '%.10f', found: '%.10f', error = '%.10f'",
          i, englishEnding(i).c_str(), j, p, doubleDelta(j, p));
    }
  }
  if (!ans.seekEof() || !ouf.seekEof()) {
    quitf(_wa, "number of token differs");
  }
  quitf(_ok, "passed");
}
