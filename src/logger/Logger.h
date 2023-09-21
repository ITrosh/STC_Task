#ifndef STC_TASK_LOGGER_H
#define STC_TASK_LOGGER_H

#include <string>

/// Logger without a full singleton for simplicity
class Logger {
public:
    enum class level_e {
        Info,
        Warn,
        Error
    };

    static void log(level_e level, const std::string& message);

private:
    static std::string currentDateTime();
};

#endif //STC_TASK_LOGGER_H
