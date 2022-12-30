#include <gmp.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>

// Change the formula for num_out_radix_digits if IN_RADIX is not 10
constexpr unsigned long int IN_RADIX = 10;
constexpr bool PRINT_DEBUG = true;
constexpr bool PRINT_NUMS = false;
constexpr bool PRINT_PROGRESS = true;
constexpr size_t BUFFER_SIZE = 256;  // To hold the output file name

// void make_upper(char *str) {
//     for (size_t i{}; str[i]; i++) {
//         if (str[i] >= 'a' && str[i] <= 'z') {
//             str[i] &= 0b1011111;
//         }
//     }
// }

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
        } else if (output[i] < 0x40) {
            output[i] += 16;
        } else {
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
    setbuf(stdout, NULL);
    char *input_file_path{};
    long int out_radix = 27;
    int opt;
    while ((opt = getopt(argc, argv, "i:r:")) != -1) {
        switch (opt) {
            case 'i': {
                input_file_path = new char[strlen(optarg) + 1];
                strcpy(input_file_path, optarg);
                printf("Input file: %s\n", input_file_path);
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
    if (input_file_path == nullptr)
        throw std::runtime_error{
            "Input file not specified. "
            "Please specify the path to the input file using the -i flag."};
    FILE *input = fopen(input_file_path, "rb");
    delete[] input_file_path;
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
    if constexpr (PRINT_DEBUG) printf("Loading integer into mpz_t... ");
    mpz_t pi;
    mpz_init_set_str(pi, buffer, IN_RADIX);
    delete[] buffer;
    if constexpr (PRINT_DEBUG) printf("Done.\n");

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
    if constexpr (PRINT_PROGRESS) printf("Calculating base %ld multiplier... ", out_radix);
    mpz_t temp;
    mpz_init_set_si(temp, out_radix);
    mpz_pow_ui(temp, temp, num_out_radix_digits);
    if constexpr (PRINT_PROGRESS) printf("Done.\nMultiplying by multiplier... ");
    mpz_mul(pi, pi, temp);
    if constexpr (PRINT_PROGRESS) printf("Done.\nCalculating base %lu divisor... ", IN_RADIX);
    mpz_ui_pow_ui(temp, IN_RADIX, num_in_radix_digits);
    if constexpr (PRINT_PROGRESS) printf("Done.\nDividing by divisor... ");
    mpz_tdiv_q(pi, pi, temp);
    if constexpr (PRINT_PROGRESS) printf("Done.\n");

    if constexpr (PRINT_DEBUG) printf("Generating base %ld text representation... ", out_radix);
    auto out = tostr_mpz_b27(pi);
    if constexpr (PRINT_DEBUG) printf("Done.\nWriting to output file... ");
    char filename[BUFFER_SIZE];
    snprintf(filename, BUFFER_SIZE, "out_%zu.txt", num_out_radix_digits);
    FILE *fout = fopen(filename, "w");
    fprintf(fout, "%s", out);
    fclose(fout);
    delete[] out;
    if constexpr (PRINT_DEBUG) printf("Done.\n");

    return 0;
}
