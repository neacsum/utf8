#pragma once

// Work around missing/deleted char32_t stream operator
// This allows utpp's CheckEqual to print char32_t values properly
// Must be defined BEFORE including utpp to ensure it's found by ADL

#include <ostream>  // IWYU pragma: keep
#include <chrono>   // IWYU pragma: keep
#include <sstream>  // IWYU pragma: keep
#include <iomanip>  // IWYU pragma: keep

// Provide the operator for compilers/standards that need it:
// - MSVC 2022+ deletes the char32_t stream operator
// - Clang with C++20 also seems to need this fix
#if (defined(_MSC_VER) && _MSC_VER >= 1930) || \
    (defined(__clang__) && __cplusplus >= 202002L)

    inline std::ostream& operator<<(std::ostream& os, char32_t c) {
        return os << static_cast<uint32_t>(c);
    }

#endif

// C++17 compatibility shims for utpp
#if __cplusplus < 202002L

    // Add std::chrono::milliseconds stream operator for C++17
    namespace std {
        namespace chrono {
            inline std::ostream& operator<<(std::ostream& os, const milliseconds& ms) {
                return os << ms.count() << "ms";
            }
            
            inline std::ostream& operator<<(std::ostream& os, const duration<float>& dur) {
                return os << dur.count() << "s";
            }
            
            // Provide utc_clock as alias to system_clock for C++17
            using utc_clock = system_clock;
        }
        
        // Simple format replacement for C++17
        template<typename... Args>
        inline std::string format(const std::string& fmt, Args&&... args) {
            std::ostringstream oss;
            // For time formatting, just return a simple ISO-like format
            if (fmt.find(":%Y-%m-%dT%H:%M:%S") != std::string::npos) {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                
#ifdef _WIN32
                struct tm tm_buf;
                gmtime_s(&tm_buf, &time_t);
                oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
#else
                oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
#endif
            } else {
                oss << "formatted_output";
            }
            return oss.str();
        }
    }

#endif

#include <utpp/utpp.h>  // IWYU pragma: keep
