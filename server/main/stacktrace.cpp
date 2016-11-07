// ==========================================================================
// Project:   Xy Server
// Copyright: ©2016 Xy Group Ltd. All rights reserved.
// ==========================================================================

// stacktrace.h (c) 2008, Timo Bingmann from https://panthema.net/2008/0901-stacktrace-demangled/
// published under the WTFPL v2.0

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

#include <iostream>
#include <sstream>
#include <string>

#include <string.h>

#include "stacktrace.h"

namespace xy {

auto stacktrace(unsigned int max_frames) -> char * {
  std::stringstream ss;

  // storage array for stack trace address data
  void* addrlist[max_frames+1];

  // retrieve current stack addresses
  int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

  if (addrlen == 0) {
    ss << "<empty, possibly corrupt>" << std::endl;
  	return strdup(ss.str().c_str());
  }

  // resolve addresses into strings containing "filename(function+address)",
  // this array must be free()-ed
  char** symbollist = backtrace_symbols(addrlist, addrlen);

  // allocate string which will be filled with the demangled function name
  size_t funcnamesize = 256;
  char* funcname = (char*)malloc(funcnamesize);

  // iterate over the returned symbol lines. skip the first, it is the
  // address of this function.
  for (int i = 1; i < addrlen; i++) {
  	char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

  	for (char *p = symbollist[i]; *p; ++p) {
#ifdef __APPLE__
      // find parentheses and +address offset surrounding the mangled name:
      // "1   main                                0x0000000100004272 _Z19throwWithStackTracev + 18"
      if (*p == ' ' && *(p+1) == '_') {
        begin_name = p;

      } else if (*p == '+') {
        begin_offset = p-1;

      } else if (*(p+1) == '\0' && begin_offset) {
        end_offset = p+1;
        break;
      }
#else
      // find parentheses and +address offset surrounding the mangled name:
      // ./module(function+0x15c) [0x8048a6d]
	    if (*p == '(') {
        begin_name = p;

	    } else if (*p == '+') {
        begin_offset = p;

	    } else if (*p == ')' && begin_offset) {
    		end_offset = p;
    		break;
	    }
#endif
  	}

  	if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
	    *begin_name++ = '\0';
	    *begin_offset++ = '\0';
	    *end_offset = '\0';

	    // mangled name is now in [begin_name, begin_offset) and caller
	    // offset in [begin_offset, end_offset). now apply __cxa_demangle():

	    int status;
	    char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
	    if (status == 0) {
    		funcname = ret; // use possibly realloc()-ed string
#ifdef __APPLE__
        ss << symbollist[i] << " " << funcname << " " << begin_offset << std::endl;
#else
        ss << symbollist[i] << " : " << funcname << "+" << begin_offset << std::endl;
#endif

	    } else {
    		// demangling failed. Output function name as a C function with
    		// no arguments.
#ifdef __APPLE__
        ss << symbollist[i] << " " << begin_name << "() " << begin_offset << std::endl;
#else
        ss << symbollist[i] << " : " << begin_name << "()+" << begin_offset << std::endl;
#endif
	    }

  	} else {
      // couldn't parse the line? print the whole line.
      ss << symbollist[i] << std::endl;
    }
  }

  free(funcname);
  free(symbollist);

  return strdup(ss.str().c_str());
}

} // xy namespace
