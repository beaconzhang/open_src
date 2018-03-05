// @author Yu Hongjin (yuhongjin@meituan.com)

#include "logging.h"

#include <cerrno>
#include <cstdarg>
#include <system_error>

namespace logging {

static LogWriter* logger = NULL;

LogWriter::LogWriter(int level, const char* name)
  : level_(level) {
  if (name && strlen(name) > 0) {
    file_ = fopen(name, "a+");
    if (!file_) {
      throw std::system_error(
          errno, std::system_category(), "open log file failed");
    }
  } else {
    file_ = stderr;
  }
}

LogWriter::~LogWriter() {
  if (file_ != NULL && file_ != stderr) {
    fclose(file_);
  }
}

void initialize(int level, const char* name) {
  if (!logger) {
    logger = new LogWriter(level, name);
  }
}

LogWriter* get() {
  if (!logger) {
    logger = new LogWriter(DEFAULT_LOGLEVEL);
  }
  return logger;
}

} // namespace logging
