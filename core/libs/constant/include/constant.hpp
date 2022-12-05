#ifndef _cpcli_constant_hpp_
#define _cpcli_constant_hpp_

#include <set>
#include <string>

static inline const std::string VERSION = "0.6.0";

static inline const std::string DASH_SEPERATOR = "\033[1;34m---------------------------\033[0m"; // blue color
static inline const std::string EQUA_SEPERATOR = "===========================";

static inline const int ARG_ERR = 1029741;
static inline const int OPERATION_ERR = 87122;
static inline const int FILE_NOT_FOUND_ERR = 53234;
static inline const int INVALID_CONFIG_ERROR = 9320850;

static inline const int RETURN_OK = 0;
static inline const int RETURN_WA = 10;
static inline const int RETURN_STOP_ON_WA = 11;
static inline const std::set<std::string> SUPPORTED_EXTENSIONS = {".cpp", ".py"};

#endif
