#pragma once

#define TRUE 1
#define FALSE 0

#include <stdint.h>
#include <stddef.h>

// We set own SO_NBIO for linux
#define SO_NBIO 0xFFAACC01