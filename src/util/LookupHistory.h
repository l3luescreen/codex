#pragma once

#include <ctime>
#include <string>

// Remembers dictionary lookups started from an EPUB page in a tab-separated
// file on the SD card, so the reader can review which words they looked up,
// in which book. Lookups that end in a read or memory error are not recorded.
//
//   /Lookup History.tsv
//   time <TAB> book <TAB> author <TAB> chapter <TAB> page <TAB> word <TAB> headword <TAB> found <TAB> dictionary
//
// One line per lookup, appended; the header is written when the file is new.
// It sits in the SD root, like "My Clippings.txt", so it shows on the
// File Transfer page.
namespace lookuphistory {

constexpr const char* PATH = "/Lookup History.tsv";
// Older lines move to this file once the history reaches MAX_BYTES, so the
// file stays quick to append to and to download.
constexpr const char* ARCHIVE_PATH = "/Lookup History old.tsv";
constexpr size_t MAX_BYTES = 2 * 1024 * 1024;
constexpr size_t MAX_FIELD_BYTES = 200;

constexpr const char* HEADER = "time\tbook\tauthor\tchapter\tpage\tword\theadword\tfound\tdictionary\n";

struct Record {
  std::string book;
  std::string author;
  std::string chapter;
  int page = 0;          // 1-based page in the chapter; 0 = unknown
  std::string word;      // what the reader selected
  std::string headword;  // the dictionary entry shown (empty when not found)
  bool found = false;
  std::string dictionary;  // dictionary folder name
};

// Make a value safe for one TSV field: tabs and line breaks become spaces,
// runs of spaces collapse, ends are trimmed, and long values are cut at a
// UTF-8 boundary.
std::string sanitize(const std::string& value);

// "YYYY-MM-DD HH:MM" in local time, or "" when the clock has not been set
// (before 2020: no Wi-Fi time sync or RTC yet).
std::string formatTime(time_t now);

// One complete line, ending in '\n'.
std::string formatLine(const Record& record, time_t now);

// Append `record` to PATH (firmware only). Returns false on SD errors; a
// failed write never affects the lookup itself.
bool append(const Record& record);

}  // namespace lookuphistory
