# 1 "/home/dcampbel/Research/build-romain/src-bobby/system/boot.bb"
#include "boot.h"

// HARDWARE INCLUDES
#include "../hw-api/hwBoot.h"

// Jumps into the bootloader section.  This function never returns.
// Note that this function only works if the BOOTRST fuse is set to Boot Loader Reset
void jumpToBootSection()
{
	jumpToHWBootSection();
}
