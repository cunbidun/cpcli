#include "operations.hpp"
#include <iostream>

using std::cout, std::endl;

void print_usage() {
  cout << endl;
  cout << "usage: cpcli -p path/to/project_config.json [OPTION]" << endl;
  cout << endl;

  cout << "Required: all options require this to be set" << endl;
  cout << "   -p, --project-config          path to the golbal config file" << endl;
  cout << endl;

  cout << "Optional: except for creating new task, all options require this to be set" << endl;
  cout << "   -r, --root-dir                the folder directory" << endl;
  cout << endl;

  cout << "Optional flags: those arguments do not require an arguments" << endl;
  cout << "   -a, --archive                 archive current root-dir" << endl;
  cout << "   -b, --build                   build solution file with normal flags" << endl;
  cout << "   -B, --build-with-term         build solution file with terminal (this option will not use cpcli)" << endl;
  cout << "   -d, --build-with-debug        build solution file with debug flags" << endl;
  cout << "   -n, --new                     create new task (this option does not require --root-dir)" << endl;
  cout << endl;

  cout << "Info: those arguments do not require an arguments" << endl;
  cout << "   -D, --debug                   run cpcli_app with debug flags (this option will print debug logs)" << endl;
  cout << "                                 when running with this flag, it is best to put this before others" << endl;
  cout << "   -h, --help                    show this help" << endl;
  cout << "   -v, --version                 print cpcli_app version" << endl;
  cout << endl;

  cout << "Examples:" << endl;
  cout << "   cpcli_app --new --project-config=./project_config.json" << endl;
  cout << "       for creating new task (the location is determine in the config file)" << endl;
  cout << endl;

  cout << "   cpcli_app --root-dir='$ROOT' --project-config=./project_config.json --build-with-debug" << endl;
  cout << "       for building and running task at '$ROOT' with debug flags" << endl;
  cout << endl;

  cout << "   cpcli_app -D --root-dir='$ROOT' --project-config=./project_config.json --build-with-debug" << endl;
  cout << "       for building and running task at '$ROOT' with debug flags, also show logs from cpcli_app" << endl;
  cout << endl;

  cout << "Check out https://github.com/cunbidun/cpcli for more info";
  cout << endl;
}
