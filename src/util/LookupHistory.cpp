// SD card side of LookupHistory: appends one line per dictionary lookup.
#include "util/LookupHistory.h"

#include <HalStorage.h>
#include <Logging.h>

#include <cstring>

namespace lookuphistory {

bool append(const Record& record) {
  // Same append pattern as ClippingStore's .deleted journal.
  HalFile file = Storage.open(PATH, O_WRONLY | O_CREAT | O_APPEND);
  if (!file) {
    LOG_ERR("LOOKUP", "Cannot open %s", PATH);
    return false;
  }
  size_t size = file.size();

  // Keep the live file small: move a full history aside (replacing the
  // previous archive) and start a new one.
  if (size >= MAX_BYTES) {
    file.close();
    Storage.remove(ARCHIVE_PATH);
    if (!Storage.rename(PATH, ARCHIVE_PATH)) LOG_ERR("LOOKUP", "Cannot rotate %s", PATH);
    file = Storage.open(PATH, O_WRONLY | O_CREAT | O_APPEND);
    if (!file) {
      LOG_ERR("LOOKUP", "Cannot open %s", PATH);
      return false;
    }
    size = file.size();
  }

  const std::string line = formatLine(record, time(nullptr));
  bool ok = true;
  if (size == 0) ok = file.write(HEADER, strlen(HEADER)) == strlen(HEADER);
  ok = ok && file.write(line.data(), line.size()) == line.size();
  file.close();
  if (!ok) LOG_ERR("LOOKUP", "Write failed: %s", PATH);
  return ok;
}

}  // namespace lookuphistory
