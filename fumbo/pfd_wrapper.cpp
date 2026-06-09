// ===========================================================================
// ALL Windows.h pollution is contained in THIS file only.
// portable-file-dialogs.h is included NOWHERE else in the project.
// ===========================================================================

#include "pfd_wrapper.hpp"
#include "external/portable-file-dialogs.h"

namespace Fumbo {
namespace FileDialog {

std::string OpenFile(const std::string &title,
                     const std::vector<std::string> &filters) {
    auto results = pfd::open_file(title, "", filters).result();
    return results.empty() ? "" : results[0];
}

std::vector<std::string> OpenFiles(const std::string &title,
                                   const std::vector<std::string> &filters) {
    return pfd::open_file(title, "", filters, pfd::opt::multiselect).result();
}

std::string SaveFile(const std::string &title,
                     const std::vector<std::string> &filters) {
    return pfd::save_file(title, "", filters).result();
}

} // namespace FileDialog
} // namespace Fumbo