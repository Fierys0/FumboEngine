#pragma once

// This header is the ONLY place in the entire project that touches
// portable-file-dialogs and windows.h. It exposes a clean interface
// so no other file ever needs to see Windows macros.

#include <string>
#include <vector>

namespace Fumbo {
namespace FileDialog {

    // Open a single file. Returns empty string if cancelled.
    std::string OpenFile(const std::string &title,
                         const std::vector<std::string> &filters = {"All Files", "*"});

    // Open multiple files. Returns empty vector if cancelled.
    std::vector<std::string> OpenFiles(const std::string &title,
                                       const std::vector<std::string> &filters = {"All Files", "*"});

    // Save a file. Returns empty string if cancelled.
    std::string SaveFile(const std::string &title,
                         const std::vector<std::string> &filters = {"All Files", "*"});

} // namespace FileDialog
} // namespace Fumbo