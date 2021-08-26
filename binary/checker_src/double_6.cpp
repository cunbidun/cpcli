#include "../../testlib.h"
#include <cmath>

using namespace std;

const double EPS = 1E-6;

int main(int argc, char *argv[]) {
  setName("compare two sequences of doubles, max absolute or relative error = %.7f", EPS);
  registerTestlibCmd(argc, argv);

  vector<double> v_ans, v_ouf;
  while (!ans.seekEof()) {
    v_ans.push_back(ans.readDouble());
  }
  while (!ouf.seekEof()) {
    v_ouf.push_back(ouf.readDouble());
  }
  if (v_ans.size() == 0) {
    quitf(_ok, "undecided\n");
  }
  if (v_ans.size() != v_ouf.size()) {
    quitf(_wa, "wrong answer\nnumber of token differs");
  }
  for (int i = 1; i <= v_ans.size(); i++) {
    double j, p;
    j = v_ans[i - 1];
    p = v_ouf[i - 1];
    if (!doubleCompare(j, p, EPS)) {
      quitf(_wa,
            "wrong answer\n%d%s numbers differ - expected: '%.7f', found: '%.7f', error = '%.7f'",
            i, englishEnding(i).c_str(), j, p, doubleDelta(j, p));
    }
  }
  quitf(_ok, "accepted\n");
}
