#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#include "log_queue.h"


namespace log_queue
{
    bool b_verbose;
    std::vector<std::string> log_queue;

    void init()
    {
        verbose(false);
        log_queue.clear();
    }

    void verbose(bool set)
    {
        b_verbose = set;
    }

    bool verbose(void)
    {
        return b_verbose;
    }

    void loop()
    {
        for (auto iter = log_queue.begin(); iter != log_queue.end(); iter++)
        {
            std::cout << *iter << std::endl;
        }
        log_queue.clear();
    }

    void log(std::string s)
    {
        log_queue.push_back(s);
    }
}; // namespace log_queue
