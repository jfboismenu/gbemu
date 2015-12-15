#include <base/logger.h>

namespace gbemu {

    void Logger::enableLogger(bool isEnabled)
    {
        _isEnabled = isEnabled;
    }

    bool Logger::isEnabled()
    {
        return _isEnabled;
    }

    bool Logger::_isEnabled = false;

}
