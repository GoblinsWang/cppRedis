#ifndef REDIS_COMMON_H
#define REDIS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <execinfo.h>

#include <iostream>
#include <iterator>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <memory>

#include <hiredis/hiredis.h>
#include "../log/logger.h"
using namespace cpplog;

#endif // REDIS_COMMON_H