#pragma once

/*

Here I put different tools that are used in RA3 engine.
(code is recreated according to reverse-engineering)

*/

#include <string>
#include <vector>

std::string encode_inet_addr_to_string(uint32_t inet_address, bool dictionary_flag);
std::string convert_to_hex_string(std::string str);
std::string get_gpcm_challenge_response();
std::string md5(const std::string str);
std::vector<std::string> split_string(std::string str);