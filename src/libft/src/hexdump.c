//
// Created by loumouli on 12/15/23.
//

#include "libft.h"

void hexdump(const void *data, const size_t len, const int32_t row) {
  if (row == 0) {
    for (size_t i = 0; i < len; i++)
      dprintf(1, "%02x ", ((uint8_t *) data)[i]);
    dprintf(1, "\n");
    return;
  }
  for (size_t i = 0; i < len; i += row) {
    for (size_t j = i; j < i + row; j++) {
      if (j == len)
        break;
      dprintf(1, "%02x ", ((uint8_t *) data)[j]);
    }
    dprintf(1, "\n");
  }
  dprintf(1, "\n");
}
