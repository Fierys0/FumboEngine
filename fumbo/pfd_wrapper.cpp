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

struct OpenFileAsync::Impl {
    pfd::open_file dialog;
    Impl(const std::string &title, const std::vector<std::string> &filters, bool multiselect)
        : dialog(title, "", filters, multiselect ? pfd::opt::multiselect : pfd::opt::none) {}
};

OpenFileAsync::OpenFileAsync(const std::string &title,
                             const std::vector<std::string> &filters,
                             bool multiselect)
    : m_impl(std::make_unique<Impl>(title, filters, multiselect)) {}

OpenFileAsync::~OpenFileAsync() = default;

bool OpenFileAsync::IsReady() const {
    return m_impl->dialog.ready(0);
}

std::vector<std::string> OpenFileAsync::GetResult() {
    return m_impl->dialog.result();
}

} // namespace FileDialog
} // namespace Fumbo