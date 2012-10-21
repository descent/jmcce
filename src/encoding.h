#ifndef ENCODING_H
#define ENCODING_H

#include <string>

#include <cstring>

#include "convertencode.h"

inline int utf8_to_big5(const char *utf8_str, std::string &str)
{
  DS::ConvertEncode convert_encode("big5","utf8");
  return convert_encode.convert(utf8_str, strlen(utf8_str), str);
}    

#endif
