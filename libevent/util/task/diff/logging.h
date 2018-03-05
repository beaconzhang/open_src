// @author Yu Hongjin (yuhongjin@meituan.com)

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ostream>
#include <streambuf>
#include <stdexcept>
#include "util.h"

#ifndef __FILENAME__
# define __FILENAME__ ((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)
#endif

namespace logging {

class FixedStreamBuf : public std::streambuf {
public:
  FixedStreamBuf(char *buf, size_t len) {
    setp(buf, buf + len);
  }

  FixedStreamBuf(const char *buf, const char *next, const char *end) {
    setg((char *)buf, (char *)next, (char *)end);
  }

  std::string str() { return std::string(pbase(), pptr()); }
};

class FixedOstream : private virtual FixedStreamBuf, public std::ostream {
public:
  typedef FixedStreamBuf StreamBuf;

  FixedOstream(char *buf, size_t len)
  : FixedStreamBuf(buf, len)
  , std::ostream(static_cast<StreamBuf *>(this)) {
  }

  char *output() { return StreamBuf::pbase(); }
  char *output_ptr() { return StreamBuf::pptr(); }
  char *output_end() { return StreamBuf::epptr(); }

  std::string str() { return StreamBuf::str(); }
};

/**
 * Output levels modelled after syslog
 */
enum Level {
  FATAL  = 0,
  ALERT  = 1,
  CRIT   = 2,
  ERROR  = 3,
  WARN   = 4,
  NOTICE = 5,
  INFO   = 6,
  DEBUG  = 7,
  TRACE  = 8,
  NOTSET = 9
};

#ifndef NDEBUG
const int DEFAULT_LOGLEVEL = DEBUG;
#else
const int DEFAULT_LOGLEVEL = WARN;
#endif

const size_t BUF_SIZE = 1024 * 10;

inline int getLevel(const char label) {
  const char* levelLabels = "FACEWNIDT";
  const char* p = strchr(levelLabels, label);
  if (!p) return DEFAULT_LOGLEVEL;
  return p - levelLabels;
}

inline const char* getLevelName(int level) {
  static const char* levelName[] = {
    "FATAL",
    "ALERT",
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "TRACE",
    "NOTSET"
  };
  return levelName[level];
}

class LogWriter {
public:
  LogWriter(int level, const char* name = NULL);
  ~LogWriter();

  void setLevel(int level) {
    level_ = level;
  }

  int getLevel() const {
    return level_;
  }

  bool isEnabled(int level) const {
    return level <= level_;
  }

  void logString(const char* message, size_t len) {
    fwrite(message, sizeof(char), len, file_);
  }

private:
  int level_;         // The current level (everything above is filtered)
  FILE *file_;        // The output file descriptor
};

/**
 * Public initialization function - creates a singleton instance of LogWriter
 */
extern void initialize(int level, const char* name = NULL);

#ifndef SET_LOGLEVEL
#define SET_LOGLEVEL(severity) \
  ::logging::initialize(::logging::getLevel(#severity[0]))
#endif

/**
 * Accessor for the LogWriter singleton instance
 */
extern LogWriter* get();

class LogMessage {
public:
  LogMessage(int level, const char* file, int line, bool abort)
  : out_(buf_, BUF_SIZE)
  , abort_(abort) {
    out_ << "[" << getLevelName(level)[0]
    << timeNowPrintf(" %y%m%d %T ")
    << file << ":" << line << "] ";
  }

  ~LogMessage() {
    out_ << std::endl;
    std::string message = out_.str();
    get()->logString(message.c_str(), message.size());
    if (abort_)
      abort();
  }

  FixedOstream& stream() { return out_; }

private:
  char buf_[BUF_SIZE];
  FixedOstream out_;
  bool abort_;
};

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
public:
  LogMessageVoidify() { }
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&) { }
};

} // namespace logging

// printf interface macro helper; do not use directly

#ifndef LOG_STREAM
#define LOG_STREAM(severity, abort)                     \
  (!::logging::get()->isEnabled(severity)) ?            \
  (void) 0 :                                            \
  ::logging::LogMessageVoidify() &                      \
  ::logging::LogMessage(                                \
      severity, __FILENAME__, __LINE__, abort).stream()
#endif

///////////////////////////////////////////////////////////////////////////
// Logging macros interface starts here

#ifndef NDEBUG
# define LOG_STREAM_DEBUG  LOG_STREAM(::logging::DEBUG, false)
# define LOG_STREAM_DFATAL LOG_STREAM(::logging::FATAL, true)
#else
# define LOG_STREAM_DEBUG  LOG_STREAM(::logging::NOTSET, false)
# define LOG_STREAM_DFATAL LOG_STREAM(::logging::NOTSET, true)
#endif

#define LOG_STREAM_INFO   LOG_STREAM(::logging::INFO, false)
#define LOG_STREAM_NOTICE LOG_STREAM(::logging::NOTICE, false)
#define LOG_STREAM_WARN   LOG_STREAM(::logging::WARN, false)
#define LOG_STREAM_ERROR  LOG_STREAM(::logging::ERROR, false)
#define LOG_STREAM_CRIT   LOG_STREAM(::logging::CRIT, false)
#define LOG_STREAM_ALERT  LOG_STREAM(::logging::ALERT, false)
#define LOG_STREAM_FATAL  LOG_STREAM(::logging::FATAL, true)

#define LOG(severity)   LOG_STREAM_ ## severity
#define PLOG(severity)  LOG_STREAM_ ## severity << strerror(errno) << " "

