#pragma once

/*

Here I put different tools that are used in RA3 engine.
(code is recreated according to reverse-engineering)

*/

#include <string>

std::string encode_inet_addr_to_string(uint32_t inet_address, bool dictionary_flag);