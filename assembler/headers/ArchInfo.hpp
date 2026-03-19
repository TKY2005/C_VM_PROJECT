#ifndef ARCHINFO_HPP
#define ARCHINFO_HPP

#include<map>
#include<string>
#include<cstdint>

class ArchInfo {
    public:
    static int dwrstart;
    static std::map<std::string, uint8_t> insmap;
    static std::string operators;
    static std::vector<std::string> reglist;
    static std::vector<std::string> dirList;
    static std::map<std::string, uint8_t> reg_select_map, index_select_map, base_select_map;
};

#endif