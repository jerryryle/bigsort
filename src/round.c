#include "round.h"

size_t round_up_to_multiple_of_4(size_t num)
{
    // This rounds up by adding 3 and then clearing the lowest two bits, ensuring a multiple of 4
    return (num + 3) & ~0x3;
}
