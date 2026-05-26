#ifndef LOG_QUEUE_H
#define LOG_QUEUE_H
// #include <vector>
#include <string>

#define VERBOSE(...) if (log_queue::verbose()) { char buffer[256]; snprintf(buffer, sizeof(buffer),  __VA_ARGS__); log_queue::log(buffer); }
namespace log_queue
{
    void init();
    void loop();
    void verbose(bool set);
    void log(std::string s);
    bool verbose(void);
};
#endif // LOG_QUEUE_H
