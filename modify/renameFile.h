#ifndef _RENAME_FILE_H_
#define _RENAME_FILE_H_
#include <map>
#include <string>
namespace hal {
    std::map<std::string, std::string> ingestRenameFile(std::string tsvPath);
}
#endif // _RENAME_FILE_H_
// Local Variables:
// mode: c++
// End:
