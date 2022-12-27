#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <gmp.h>

constexpr unsigned long int IN_RADIX = 10;  // Change the formula for num_out_radix_digits if this is not 10
constexpr long int OUT_RADIX = 27;

void make_upper(char* str) {
    size_t i{};
    while(str[i]) {
        if(str[i] >= 'a' && str[i] <= 'z') {
            str[i] &= 0b1011111;
        }
        i++;
    }
}

void print_mpz(const mpz_t &bigint, int base) {
    char* output = mpz_get_str(nullptr, base, bigint);
    printf("%s\n", output);
    delete[] output;
}

void print_mpz_b27(const mpz_t &bigint) {
    char* output = mpz_get_str(nullptr, 27, bigint);
    for(size_t i{}; output[i]; i++) {
        if(output[i] == '0') {
            output[i] = '_';
        } else if(output[i] >= '0' && output[i] <= '9') {
            output[i] = output[i] - '1' + 'A';
        } else if(output[i] >= 'a' && output[i] <= 'z') {
            output[i] -= 23;
        }
    }
    printf("%s\n", output);
    delete[] output;
}

int main() {
    FILE *input = fopen(R"(..\large_files\pi_dec_1t_01\pi_dec_1024.txt)", "r");

    // Get file size:
    struct stat st;
    fstat(fileno(input), &st);
    size_t buffer_size = st.st_size + 1;

    // Load file into buffer:
    auto *buffer = new char[buffer_size];
    fread(buffer, buffer_size + 1, 1, input);
    fclose(input);

    // Load buffer into mpz_t:
    mpz_t pi;
    mpz_init_set_str(pi, buffer, IN_RADIX);
    delete[] buffer;

    print_mpz(pi, IN_RADIX);

    // Number of decimal digits after the decimal point
    size_t num_in_radix_digits = mpz_sizeinbase(pi, IN_RADIX) - 1;
    printf("num_in_radix_digits: %zu\n", num_in_radix_digits);

    size_t num_out_radix_digits = static_cast<double>(num_in_radix_digits) / log10(OUT_RADIX);
    printf("num_out_radix_digits: %zu\n", num_out_radix_digits);

    for(size_t i{}; i < num_out_radix_digits; i++) {
        mpz_mul_si(pi, pi, OUT_RADIX);
    }

    for(size_t i{}; i < num_in_radix_digits; i++) {
        mpz_tdiv_q_ui(pi, pi, IN_RADIX);
    }
    
    print_mpz_b27(pi);

    return 0;
}