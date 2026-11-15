// Pure formatting for LookupHistory (no hardware calls; covered by host tests).
#include <ctime>

#include "util/LookupHistory.h"

namespace lookuphistory {

namespace {
constexpr time_t VALID_TIME = 1577836800;  // 2020-01-01: earlier means the clock was never set
}

std::string sanitize(const std::string& value) {
  std::string out;
  out.reserve(value.size());
  bool pendingSpace = false;
  for (const char c : value) {
    if (c == '\t' || c == '\n' || c == '\r' || c == ' ') {
      pendingSpace = !out.empty();
      continue;
    }
    if (pendingSpace) out.push_back(' ');
    pendingSpace = false;
    out.push_back(c);
  }
  if (out.size() > MAX_FIELD_BYTES) {
    size_t end = MAX_FIELD_BYTES;
    while (end > 0 && (static_cast<unsigned char>(out[end]) & 0xC0) == 0x80) end--;
    out.resize(end);
  }
  return out;
}

std::string formatTime(const time_t now) {
  if (now < VALID_TIME) return {};
  struct tm local{};
  localtime_r(&now, &local);
  char buf[32];
  if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &local) == 0) return {};
  return buf;
}

std::string formatLine(const Record& record, const time_t now) {
  std::string line;
  line.reserve(256);
  const auto field = [&line](const std::string& value) {
    line += sanitize(value);
    line += '\t';
  };
  field(formatTime(now));
  field(record.book);
  field(record.author);
  field(record.chapter);
  field(record.page > 0 ? std::to_string(record.page) : std::string());
  field(record.word);
  field(record.found ? record.headword : std::string());
  field(record.found ? "yes" : "no");
  line += sanitize(record.dictionary);
  line += '\n';
  return line;
}

}  // namespace lookuphistory
