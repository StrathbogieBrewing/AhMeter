#include "bit_rev.h"

uint8_t bit_rev(uint8_t x)
{
  uint8_t result;
    asm("mov __tmp_reg__, %[in] \n\t"
      "lsl __tmp_reg__  \n\t"   /* shift out high bit to carry */
      "ror %[out] \n\t"  /* rotate carry __tmp_reg__to low bit (eventually) */
      "lsl __tmp_reg__  \n\t"   /* 2 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 3 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 4 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 5 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 6 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 7 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 8 */
      "ror %[out] \n\t"
      : [out] "=r" (result) : [in] "r" (x));
      return(result);
}