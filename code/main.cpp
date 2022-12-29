#include <gmp.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cmath>
#include <cstdio>
#include <stdexcept>

// Change the formula for num_out_radix_digits if IN_RADIX is not 10
constexpr unsigned long int IN_RADIX = 10;
constexpr bool PRINT_DEBUG = true;
constexpr bool PRINT_NUMS = false;
constexpr bool PRINT_PROGRESS = true;
constexpr size_t BUFFER_SIZE = 256;

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

char *get_program_name(char *fullpath) {
    auto *program_name{fullpath};
    for (auto *current_index{fullpath}; *current_index; current_index++) {
        if (*current_index == '\\') program_name = current_index + 1;
    }
    return program_name;
}

int main(int argc, char **argv) {
    char *input_file{};
    long int out_radix = 27;
    int opt;
    while ((opt = getopt(argc, argv, "i:r:")) != -1) {
        switch (opt) {
            case 'i': {
                input_file = new char[strlen(optarg) + 1];
                strcpy(input_file, optarg);
                printf("Input file: %s\n", input_file);
            } break;
            case 'r': {
                out_radix = atoi(optarg);
                if (out_radix < 2 || out_radix > 62)
                    throw std::runtime_error{"Out radix must be in the range 2..62."};
            } break;
            default: {
                fprintf(stderr, "Usage: %s -i file... [-d] [radix...]", get_program_name(argv[0]));
            }
        }
    }
    if (input_file == nullptr)
        throw std::runtime_error{
            "Input file not specified. "
            "Please specify the path to the input file using the -i flag."};
    FILE *input = fopen(input_file, "rb");
    if constexpr (PRINT_DEBUG) printf("File opened.\n");

    // Get file size:
    struct stat st;
    fstat(fileno(input), &st);
    if constexpr (PRINT_DEBUG) printf("File size: %zu bytes\n", st.st_size);
    size_t buffer_size = st.st_size + 1;

    // Load file into buffer:
    if constexpr (PRINT_DEBUG) printf("Loading file... ");
    auto *buffer = new char[buffer_size];
    fread(buffer, buffer_size + 1, 1, input);
    fclose(input);
    if constexpr (PRINT_DEBUG) printf("Done.\n");

    // Load buffer into mpz_t:
    mpz_t pi;
    mpz_init_set_str(pi, buffer, IN_RADIX);
    delete[] buffer;

    if constexpr (PRINT_NUMS) print_mpz(pi, IN_RADIX);

    // Number of decimal digits after the decimal point
    size_t num_in_radix_digits = mpz_sizeinbase(pi, IN_RADIX) - 1;
    if constexpr (PRINT_DEBUG) printf("num_in_radix_digits: %zu\n", num_in_radix_digits);
    if (!num_in_radix_digits) throw std::runtime_error{"Error reading file."};

    // Calculate the number of output digits we can reasonably produce,
    // given the number of input digits we have:
    size_t num_out_radix_digits = num_in_radix_digits / log10(out_radix) - 1;
    if constexpr (PRINT_DEBUG) printf("num_out_radix_digits: %zu\n", num_out_radix_digits);

    // Do the actual conversion:
    if constexpr (PRINT_PROGRESS) printf("Converting to base %ld... ", out_radix);
    for (size_t i{}; i < num_out_radix_digits; i++) {
        mpz_mul_si(pi, pi, out_radix);
    }
    if constexpr (PRINT_PROGRESS) printf("Done.\nDiving by input base %lu... ", IN_RADIX);
    for (size_t i{}; i < num_in_radix_digits; i++) {
        mpz_tdiv_q_ui(pi, pi, IN_RADIX);
    }
    if constexpr (PRINT_PROGRESS) printf("Done.\n");

    // Write result to output:
    if constexpr (PRINT_PROGRESS) printf("Writing to output file... ");
    auto out = tostr_mpz_b27(pi);
    char filename[BUFFER_SIZE];
    snprintf(filename, BUFFER_SIZE, "out_%zu.txt", num_out_radix_digits);
    FILE *fout = fopen(filename, "w");
    fprintf(fout, "%s", out);
    fclose(fout);
    delete[] out;
    if constexpr (PRINT_PROGRESS) printf("Done.\n");

    return 0;
}
