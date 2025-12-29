#define VERBOSE(...) \
if (verbose) \
{ \
    char buffer[256]; \
    sprintf(buffer, __VA_ARGS__); \
    log_queue.push_back(buffer); \
}
