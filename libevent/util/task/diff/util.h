// @author Yu Hongjin (yuhongjin@meituan.com)

#pragma once

#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <sys/time.h>
#include <stdint.h>

inline std::string to(int i) { return std::to_string((long long)i); }

inline std::string to(const std::string& s) { return s; }

template <class T>
std::string join(std::string delimiter, const std::vector<T>& v) {
    std::string out;
    auto begin = v.begin();
    auto end = v.end();
    if (begin == end) {
        return out;
    }
    out.append(to(*begin));
    while (++begin != end) {
        out.append(to(delimiter));
        out.append(to(*begin));
    }
    return out;
}

inline std::string timeNowPrintf(const char *format) {
  std::string output;

  time_t t = std::time(NULL);
  const struct tm *tm = std::localtime(&t);

  size_t formatLen = strlen(format);
  size_t remaining = std::max(32UL, formatLen * 2);
  output.resize(remaining);
  size_t bytesUsed = 0;

  do {
    bytesUsed = strftime(&output[0], remaining, format, tm);
    if (bytesUsed == 0) {
      remaining *= 2;
      if (remaining > formatLen * 16) {
        throw std::invalid_argument("Maybe a non-output format given");
      }
      output.resize(remaining);
    } else {  // > 0, there was enough room
      break;
    }
  } while (bytesUsed == 0);

  output.resize(bytesUsed);

  return output;
}

inline uint64_t timestampNow() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1e6 + tv.tv_usec;
}

inline bool hexlify(const std::string& input, std::string& output) {
  output.clear();

  static char hexValues[] = "0123456789abcdef";
  output.resize(2 * input.size());
  for (size_t i = 0, j = 0; i < input.size(); ++i) {
    int ch = input[i];
    output[j++] = hexValues[(ch >> 4) & 0xf];
    output[j++] = hexValues[ch & 0xf];
  }
  return true;
}

inline int unhex(char c) {
  return c >= '0' && c <= '9' ? c - '0' :
         c >= 'A' && c <= 'F' ? c - 'A' + 10 :
         c >= 'a' && c <= 'f' ? c - 'a' + 10 :
         -1;
}

inline bool unhexlify(const std::string& input, std::string& output) {
  if (input.size() % 2 != 0) {
    return false;
  }
  output.resize(input.size() / 2);
  for (size_t i = 0, j = 0; i < input.size(); i += 2) {
    int highBits = unhex(input[i]);
    int lowBits = unhex(input[i + 1]);
    if (highBits < 0 || lowBits < 0) {
      return false;
    }
    output[j++] = (highBits << 4) + lowBits;
  }
  return true;
}

inline uint32_t string_int(const std::string& str,uint32_t start,uint32_t end,uint32_t& result){
    if(start>=str.size()||end>str.size()){
        return start;
    }
    result=0;
    for(;start<end;start++){
        if(__builtin_expect((str[start]>='0'&&str[start]<='9'),1)){
            result=str[start]-'0'+result*10;
        }else{
            break;
        }
    }
    return start;
}
