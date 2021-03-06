#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

#include "ra3_engine_tools.h"
#include "../client_info.hpp"

std::string encode_inet_addr_to_string(uint32_t inet_address, bool dictionary_flag)
{
    const char* dictionary = dictionary_flag ?
        "qJ1h4N9cP3lzD0Ka" : "aFl4uOD9sfWq1vGp";
    
    char temp_buffer[9];    // we use 9 - it's max len of "%08x" string including '\0'
    sprintf(temp_buffer, "%08x", inet_address ^ 0xC3801DC7);

    const auto alphabet = "0123456789abcdef";
    for (int i = 0; i < 8; ++i)
    {
        // Get HEX digit and convert to decimal
        auto decimal = strchr(alphabet, temp_buffer[i]) - alphabet;

        // This will never happen (only if code contains errors)
        // temp_buffer must contain only HEX symbols
        if (decimal > 15)
        {
            strcpy(temp_buffer, "14saFv19");
            break;
        }
        
        // Save new (changed) symbol according to our dictionary
        temp_buffer[i] = dictionary[decimal];
    }

    // Return encoded string
    return std::string(temp_buffer);
}

std::string convert_to_hex_string(std::string str)
{
    const auto alphabet = "0123456789abcdef";

    // Each symbol will be converted to 2 HEX symbols + null terminator
    uint32_t len = str.size() * 2;
    char* dst = new char[len + 1];

    // Convert all symbols to HEX
    for (uint32_t i = 0; i < str.size(); ++i)
    {
        dst[i * 2 + 0] = alphabet[str[i] >> 4];
        dst[i * 2 + 1] = alphabet[str[i] & 0x0F];
    }

    // Don't forget to set null terminator
    dst[len] = '\0';
    
    // Create string for our result
    auto result = std::string(dst);

    // Free allocated memory
    delete[] dst;

    return result;
}

std::string get_gpcm_challenge_response_magic(ra3_client_info& client_info)
{
    return md5(convert_to_hex_string(client_info.preauth__challenge__get())) +
           "                                                " +
           client_info.preauth__ticket__get() +
           client_info.gpcm__our_challenge__get() +
           client_info.gpcm__initial_challenge__get() +
           md5(convert_to_hex_string(client_info.preauth__challenge__get()));
}

std::string get_gpcm_challenge_response(ra3_client_info& client_info)
{
    return md5(convert_to_hex_string(get_gpcm_challenge_response_magic(client_info)));
}

std::vector<std::string> split_string(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}