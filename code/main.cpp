#include <gmp.h>
#include <sys/stat.h>

#include <cmath>
#include <cstdio>
#include <stdexcept>

// Change the formula for num_out_radix_digits if IN_RADIX is not 10
constexpr unsigned long int IN_RADIX = 10;
constexpr long int OUT_RADIX = 27;
constexpr bool DEBUG_PRINT = false;

void make_upper(char *str) {
  for (size_t i{}; str[i]; i++) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      str[i] &= 0b1011111;
    }
  }
}

void print_mpz(const mpz_t &bigint, int base) {
  char *output = mpz_get_str(nullptr, base, bigint);
  printf("%s\n", output);
  delete[] output;
}

void print_mpz_b27(const mpz_t &bigint) {
  char *output = mpz_get_str(nullptr, 27, bigint);
  for (size_t i{}; output[i]; i++) {
    if (output[i] == '0') {
      output[i] = '_';
    } else if (output[i] >= '0' && output[i] <= '9') {
      output[i] = output[i] - '1' + 'A';
    } else if (output[i] >= 'a' && output[i] <= 'z') {
      output[i] -= 23;
    }
  }
  printf("%s\n", output);
  delete[] output;
}

char *tostr_mpz_b27(const mpz_t &bigint) {
  char *output = mpz_get_str(nullptr, 27, bigint);
  for (size_t i{}; output[i]; i++) {
    if (output[i] == '0') {
      output[i] = '_';
    } else if (output[i] >= '0' && output[i] <= '9') {
      output[i] = output[i] - '1' + 'A';
    } else if (output[i] >= 'a' && output[i] <= 'z') {
      output[i] -= 23;
    }
  }
  return output;
}

int main() {
  FILE *input = fopen(R"(..\large_files\pi_dec_1t_01\pi_dec_16MiB.txt)", "r");

  // Get file size:
  struct stat st;
  fstat(fileno(input), &st);
  size_t buffer_size = st.st_size + 1;

  // Load file into buffer:
  auto *buffer = new char[buffer_size];
  if (!fread(buffer, buffer_size + 1, 1, input)) {
    throw std::runtime_error{"Error reading file"};
  }
  fclose(input);

  // Load buffer into mpz_t:
  mpz_t pi;
  mpz_init_set_str(pi, buffer, IN_RADIX);
  delete[] buffer;

  if constexpr (DEBUG_PRINT) {
    print_mpz(pi, IN_RADIX);
  }

  // Number of decimal digits after the decimal point
  size_t num_in_radix_digits = mpz_sizeinbase(pi, IN_RADIX) - 1;
  printf("num_in_radix_digits: %zu\n", num_in_radix_digits);

  // Calculate the number of output digits we can reasonably produce,
  // given the number of input digits we have:
  size_t num_out_radix_digits = num_in_radix_digits / log10(OUT_RADIX);
  printf("num_out_radix_digits: %zu\n", num_out_radix_digits);

  // Do the actual conversion:
  for (size_t i{}; i < num_out_radix_digits; i++) {
    mpz_mul_si(pi, pi, OUT_RADIX);
  }
  for (size_t i{}; i < num_in_radix_digits; i++) {
    mpz_tdiv_q_ui(pi, pi, IN_RADIX);
  }

  // Write result to output:
  auto out = tostr_mpz_b27(pi);
  FILE *fout = fopen("out.txt", "w");
  fprintf(fout, "%s", out);
  fclose(fout);
  delete[] out;

  return 0;
}