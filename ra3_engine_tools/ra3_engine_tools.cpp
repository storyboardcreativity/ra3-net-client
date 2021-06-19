#include <cstring>

#include "ra3_engine_tools.h"

std::string encode_inet_addr_to_string(uint32_t inet_address, bool dictionary_flag)
{
    const char* dictionary = dictionary_flag ?
        "qJ1h4N9cP3lzD0Ka" : "aFl4uOD9sfWq1vGp";
    
    char temp_buffer[9];    // we use 9 - it's max len of "%08x" string including '\0'
    sprintf(temp_buffer, "%08x", inet_address ^ 0xC3801DC7);

    auto alphabet = "0123456789abcdef";
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