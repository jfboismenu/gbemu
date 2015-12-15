#pragma once

#include <string>
#include <iostream>

namespace gbemu {

    class Logger
    {
    public:
        static void enableLogger(bool isEnabled);
        static bool isEnabled();
    private:
        static bool _isEnabled;
    };

    #define JFX_LOG(output) \
    { \
        if (Logger::isEnabled()) { \
            std::cout << output << std::endl; \
        } \
    }

    #define JFX_LOG_VAR(var) JFX_LOG(#var " " << var)
}
