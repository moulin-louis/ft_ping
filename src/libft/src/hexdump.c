//
// Created by loumouli on 12/15/23.
//

#include "libft.h"

void hexdump(const void *data, const size_t len, const int32_t row) {
  if (row == 0) {
    for (size_t i = 0; i < len; i++)
      fprintf(stdout, "%02x ", ((uint8_t *) data)[i]);
    fprintf(stdout, "\n");
    return;
  }
  for (size_t i = 0; i < len; i += row) {
    for (size_t j = i; j < i + row; j++) {
      if (j == len)
        break;
      fprintf(stdout, "%02x ", ((uint8_t *) data)[j]);
    }
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");
}
