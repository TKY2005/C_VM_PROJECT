#ifndef UNICODE_HPP
#define UNICODE_HPP

#include<cstdint>
#include<stdexcept>
#include<string>
#include<vector>

inline std::vector<uint32_t> decodeUtf8(const std::string& text)
{
    std::vector<uint32_t> characters;
    for (size_t i = 0; i < text.size();) {
        const uint8_t first = static_cast<uint8_t>(text[i]);
        uint32_t codepoint = 0;
        size_t length = 0;
        if (first <= 0x7f) {
            codepoint = first;
            length = 1;
        }
        else if (first >= 0xc2 && first <= 0xdf) {
            codepoint = first & 0x1f;
            length = 2;
        }
        else if (first >= 0xe0 && first <= 0xef) {
            codepoint = first & 0x0f;
            length = 3;
        }
        else if (first >= 0xf0 && first <= 0xf4) {
            codepoint = first & 0x07;
            length = 4;
        }
        else {
            throw std::runtime_error("string contains invalid UTF-8");
        }

        if (i + length > text.size()) {
            throw std::runtime_error("string contains incomplete UTF-8");
        }
        for (size_t continuation = 1; continuation < length; ++continuation) {
            const uint8_t byte = static_cast<uint8_t>(text[i + continuation]);
            if ((byte & 0xc0) != 0x80) {
                throw std::runtime_error("string contains invalid UTF-8");
            }
            codepoint = (codepoint << 6) | (byte & 0x3f);
        }
        if ((length == 3 && codepoint < 0x800) ||
            (length == 4 && codepoint < 0x10000) ||
            (codepoint >= 0xd800 && codepoint <= 0xdfff) ||
            codepoint > 0x10ffff) {
            throw std::runtime_error("string contains an invalid Unicode code point");
        }
        characters.push_back(codepoint);
        i += length;
    }
    return characters;
}

#endif
