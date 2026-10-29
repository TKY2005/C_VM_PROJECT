#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

#include<cstdint>
#include<optional>
#include<string>

int disassembleBinary(
    const std::string& sourceFile,
    const std::string& destinationFile,
    const std::optional<uint32_t>& imageBase = std::nullopt
);

#endif
