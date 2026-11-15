#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>

#include "util/LookupHistory.h"

using namespace lookuphistory;

namespace {
class UtcClock : public ::testing::Test {
 protected:
  void SetUp() override {
    setenv("TZ", "UTC0", 1);
    tzset();
  }
};

std::string field(const std::string& line, int index) {
  size_t start = 0;
  for (int i = 0; i < index; i++) start = line.find('\t', start) + 1;
  const size_t end = line.find_first_of("\t\n", start);
  return line.substr(start, end - start);
}
}  // namespace

TEST(LookupHistorySanitize, CollapsesTabsAndLineBreaks) {
  EXPECT_EQ(sanitize("  put\tup \n with\r\n "), "put up with");
  EXPECT_EQ(sanitize(""), "");
  EXPECT_EQ(sanitize("\t\n"), "");
}

TEST(LookupHistorySanitize, CutsLongValuesAtUtf8Boundary) {
  std::string thai;
  for (int i = 0; i < 100; i++) thai += "\xE0\xB8\x81";  // 3-byte Thai letter
  const std::string out = sanitize(thai);
  EXPECT_LE(out.size(), MAX_FIELD_BYTES);
  EXPECT_EQ(out.size() % 3, 0u);
}

TEST_F(UtcClock, TimeIsLocalAndBlankWhenClockUnset) {
  EXPECT_EQ(formatTime(0), "");
  EXPECT_EQ(formatTime(1500000000), "");                  // 2017: clock not set
  EXPECT_EQ(formatTime(1794700800), "2026-11-15 00:00");  // 2026-11-15T00:00:00Z
  setenv("TZ", "ICT-7", 1);                               // Bangkok, UTC+7
  tzset();
  EXPECT_EQ(formatTime(1794700800), "2026-11-15 07:00");
}

TEST_F(UtcClock, FoundLookupHasAllFields) {
  Record r;
  r.book = "The Hound of the Baskervilles";
  r.author = "Arthur Conan Doyle";
  r.chapter = "Chapter 1\tMr. Sherlock Holmes";
  r.page = 12;
  r.word = "lingered";
  r.headword = "linger";
  r.found = true;
  r.dictionary = "yaitron-en-th";
  const std::string line = formatLine(r, 1794700800);
  ASSERT_EQ(line.back(), '\n');
  EXPECT_EQ(field(line, 0), "2026-11-15 00:00");
  EXPECT_EQ(field(line, 1), "The Hound of the Baskervilles");
  EXPECT_EQ(field(line, 3), "Chapter 1 Mr. Sherlock Holmes");  // tab inside a value never splits a column
  EXPECT_EQ(field(line, 4), "12");
  EXPECT_EQ(field(line, 5), "lingered");
  EXPECT_EQ(field(line, 6), "linger");
  EXPECT_EQ(field(line, 7), "yes");
  EXPECT_EQ(field(line, 8), "yaitron-en-th");
  // Same column count as the header.
  const auto tabs = [](const std::string& s) { return std::count(s.begin(), s.end(), '\t'); };
  EXPECT_EQ(tabs(line), tabs(HEADER));
}

TEST_F(UtcClock, MissedLookupHasNoHeadwordAndUnknownPageIsBlank) {
  Record r;
  r.book = "Book";
  r.word = "dining-room";
  r.headword = "should not appear";
  r.found = false;
  const std::string line = formatLine(r, 0);
  EXPECT_EQ(field(line, 0), "");
  EXPECT_EQ(field(line, 4), "");
  EXPECT_EQ(field(line, 6), "");
  EXPECT_EQ(field(line, 7), "no");
}
