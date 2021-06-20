#pragma once

#include <string>

void            preauth__ticket__set(std::string ticket);
std::string     preauth__ticket__get();

void            preauth__challenge__set(std::string challenge);
std::string     preauth__challenge__get();

void            gpcm__initial_challenge__set(std::string challenge);
std::string     gpcm__initial_challenge__get();

void            gpcm__our_challenge__set(std::string challenge);
std::string     gpcm__our_challenge__get();

void            nuloginpersona__profile_id__set(std::string profile_id);
std::string     nuloginpersona__profile_id__get();
