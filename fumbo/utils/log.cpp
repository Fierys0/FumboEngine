#include "../../fumbo.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <mutex>
#include <string>

// ============================================================
// Fumbo::Log  —  lightweight logger
//
// Features:
//   • Levels: DEBUG < INFO < WARN < ERR
//   • Timestamps: [HH:MM:SS.mmm]
//   • ANSI colour on stdout/stderr (skipped on Windows without VT mode)
//   • Parallel writes to console AND a log file
//   • Varargs printf-style helpers (Debugf / Infof / Warnf / Errorf)
//   • Thread-safe (single mutex)
// ============================================================

namespace {

// ---- internal state ----------------------------------------
std::mutex    g_mutex;
std::ofstream g_file;
Fumbo::Log::Level g_minLevel = Fumbo::Log::Level::DEBUG;

// ---- helpers -----------------------------------------------

std::string Timestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d",
                  tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms.count());
    return buf;
}

#if defined(_WIN32) && !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
    static const char* COL_RESET = "";
    static const char* COL_DBG   = "";
    static const char* COL_INFO  = "";
    static const char* COL_WARN  = "";
    static const char* COL_ERR   = "";
#else
    static const char* COL_RESET = "\033[0m";
    static const char* COL_DBG   = "\033[36m";  // cyan
    static const char* COL_INFO  = "\033[32m";  // green
    static const char* COL_WARN  = "\033[33m";  // yellow
    static const char* COL_ERR   = "\033[31m";  // red
#endif

struct LevelMeta { const char* tag; const char* color; bool useStderr; };

LevelMeta MetaFor(Fumbo::Log::Level level) {
    switch (level) {
        case Fumbo::Log::Level::DEBUG: return { "DEBUG", COL_DBG,  false };
        case Fumbo::Log::Level::INFO:  return { "INFO ", COL_INFO, false };
        case Fumbo::Log::Level::WARN:  return { "WARN ", COL_WARN, false };
        case Fumbo::Log::Level::ERR:   return { "ERROR", COL_ERR,  true  };
        default:                       return { "?????", COL_RESET,false };
    }
}

void Emit(Fumbo::Log::Level level, const std::string& msg) {
    if (level < g_minLevel) return;

    std::lock_guard<std::mutex> lock(g_mutex);
    auto meta = MetaFor(level);
    auto ts   = Timestamp();

    // Plain line for the log file (no ANSI codes)
    char plain[4096];
    std::snprintf(plain, sizeof(plain), "[%s] [%s] %s\n",
                  ts.c_str(), meta.tag, msg.c_str());

    // Colour line for the console
    char coloured[4096 + 64];
    std::snprintf(coloured, sizeof(coloured), "%s[%s] [%s]%s %s\n",
                  meta.color, ts.c_str(), meta.tag, COL_RESET, msg.c_str());

    FILE* con = meta.useStderr ? stderr : stdout;
    std::fputs(coloured, con);

    if (g_file.is_open()) {
        g_file << plain;
        g_file.flush();
    }
}

std::string Vformat(const char* fmt, va_list args) {
    char buf[4096];
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    return buf;
}

} // anonymous namespace

// ============================================================
// Public API
// ============================================================

namespace Fumbo {
namespace Log {

void Init(const std::string& logFile, Level minLevel) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_minLevel = minLevel;

    if (!logFile.empty()) {
        // Close any previously open file before reopening
        if (g_file.is_open())
            g_file.close();
        g_file.open(logFile, std::ios::out | std::ios::trunc);
        if (!g_file.is_open()) {
            std::fprintf(stderr,
                "[Fumbo::Log] WARNING: could not open log file '%s'\n",
                logFile.c_str());
        }
    }

    Emit(Level::INFO, "=== Fumbo Logger initialised ===");
}

void Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_file.is_open()) {
        g_file << "[" << Timestamp() << "] [INFO ] === Fumbo Logger shutdown ===\n";
        g_file.close();
    }
}

void SetLevel(Level minLevel) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_minLevel = minLevel;
}

bool IsFileOpen() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_file.is_open();
}

void Write(Level level, const std::string& msg) { Emit(level, msg); }

void Debug(const std::string& msg) { Emit(Level::DEBUG, msg); }
void Info (const std::string& msg) { Emit(Level::INFO,  msg); }
void Warn (const std::string& msg) { Emit(Level::WARN,  msg); }
void Error(const std::string& msg) { Emit(Level::ERR,   msg); }

void Debugf(const char* fmt, ...) {
    va_list a; va_start(a, fmt); Emit(Level::DEBUG, Vformat(fmt, a)); va_end(a);
}
void Infof(const char* fmt, ...) {
    va_list a; va_start(a, fmt); Emit(Level::INFO,  Vformat(fmt, a)); va_end(a);
}
void Warnf(const char* fmt, ...) {
    va_list a; va_start(a, fmt); Emit(Level::WARN,  Vformat(fmt, a)); va_end(a);
}
void Errorf(const char* fmt, ...) {
    va_list a; va_start(a, fmt); Emit(Level::ERR,   Vformat(fmt, a)); va_end(a);
}

} // namespace Log
} // namespace Fumbo
