#include <string>
namespace rom_ram
{
    void init(void);
    void loop(void);
    bool cmd_upload_rom(std::string);
    bool cmd_page_to_rom(std::string);
    bool cmd_dump_rom(std::string);
    bool cmd_dump_ram(std::string);
    bool cmd_rom_to_page(std::string);
}
