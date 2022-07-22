#ifndef KRIT_UTILS_BROWSER
#define KRIT_UTILS_BROWSER

namespace krit {

struct Browser {
    static void openUrl(const std::string &url) {
#if defined(KRIT_WINDOWS)
#elif defined(KRIT_LINUX)
        int pid = fork();
        if (pid == 0) {
            execl("/usr/bin/xdg-open", "xdg-open", url.c_str(), (char *)0);
            exit(1);
        }
#endif
    }
};

}

#endif
