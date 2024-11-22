#ifndef KRIT_PLATFORM
#define KRIT_PLATFORM

#include <filesystem>

#include <memory>
#include <optional>
#include <vector>

namespace krit {

struct Platform {
    virtual ~Platform() = default;
    virtual const char *name() = 0;
    virtual std::string dataDir() = 0;
    virtual std::string configDir() = 0;

    virtual std::optional<std::string>
    saveFileDialog(const std::string &title,
                   std::vector<std::string> filters) {
        return {};
    }
    virtual std::optional<std::string>
    openFileDialog(const std::string &title,
                   std::vector<std::string> filters) {
        return {};
    }

    // standard, std::filesystem based API below
    bool exists(const std::filesystem::path &path);
    bool isDir(const std::filesystem::path &path);
    std::string readFile(const std::filesystem::path &path);
    void writeFile(const std::filesystem::path &path,
                   const std::string &content);
    bool copyFile(const std::filesystem::path &src,
                  const std::filesystem::path &dest);
    bool moveFile(const std::filesystem::path &src,
                  const std::filesystem::path &dest);
    bool createDir(const std::filesystem::path &path, bool recursive = false);
    std::vector<std::string> readDir(const std::filesystem::path &path);
    bool remove(const std::filesystem::path &path, bool recursive = false);
    std::string joinPaths(const std::filesystem::path &a,
                          const std::filesystem::path &b) {
        return (a / b).string();
    }

    std::string getClipboardText();
    void setClipboardText(const std::string &);
};

std::unique_ptr<Platform> platform();

}

#endif
