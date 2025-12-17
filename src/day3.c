#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct joltage_banks {
    size_t banks;
    size_t bank_size;
    uint8_t* joltage;
};

void free_joltage_banks(struct joltage_banks* jb)
{
    if (jb) {
        free(jb->joltage);
        free(jb);
    }
}

struct joltage_banks* new_joltage_banks(size_t banks, size_t bank_size)
{
    struct joltage_banks* jb = malloc(sizeof(struct joltage_banks));
    jb->banks = banks;
    jb->bank_size = bank_size;
    jb->joltage = malloc(banks * bank_size * sizeof(uint8_t));
    memset(jb->joltage, 0, banks * bank_size * sizeof(uint8_t));
    return jb;
}

uint8_t* get_joltage_bank(struct joltage_banks* jb, size_t bank_index)
{
    if (bank_index >= jb->banks) {
        fprintf(stderr, "Bank index out of range: %zu\n", bank_index);
        abort();
    }
    return &jb->joltage[bank_index * jb->bank_size];
}

void print_joltage_banks(struct joltage_banks* jb)
{
    for (size_t b = 0; b < jb->banks; ++b) {
        uint8_t* bank = get_joltage_bank(jb, b);
        for (size_t s = 0; s < jb->bank_size; ++s) {
            printf("%u", bank[s]);
        }
        printf("\n");
    }
}

struct joltage_banks* parse_joltage_banks(FILE* stream)
{
    size_t bank_count = 0;
    size_t bank_size = 0;
    size_t line_length = 0;

    char buffer[256] = { 0 };
    uint8_t* joltage_data = NULL;

    while (fgets(buffer, sizeof(buffer), stream)) {

        line_length = strnlen(buffer, 256);
        if (buffer[line_length - 1] == '\n') {
            --line_length;
            buffer[line_length] = '\0';
        }
        if (bank_size != 0 && line_length != bank_size) {
            fprintf(stderr, "Inconsistent bank sizes: %zu vs %zu\n", bank_size, line_length);
            abort();
        } else {
            bank_size = line_length;
        }

        if (bank_size == 0) {
            joltage_data = malloc(1);
        } else {
            joltage_data = realloc(joltage_data, (bank_count + 1) * line_length * sizeof(uint8_t));
        }

        uint8_t* output_bank = &joltage_data[bank_count * line_length];
        for (size_t i = 0; i < line_length; ++i) {
            if (buffer[i] < '0' || buffer[i] > '9') {
                fprintf(stderr, "Invalid character in joltage data: %c\n", buffer[i]);
                abort();
            }
            output_bank[i] = (uint8_t)(buffer[i] - '0');
        }
        memset(buffer, 0, sizeof(buffer));

        ++bank_count;
    }

    struct joltage_banks* jb = new_joltage_banks(bank_count, bank_size);
    memcpy(jb->joltage, joltage_data, bank_count * bank_size * sizeof(uint8_t));
    free(joltage_data);

    return jb;
}

uint64_t part1_bank_max(uint8_t* bank, size_t bank_size)
{
    size_t first_max_index = 0;
    uint8_t first_max_value = 0;
    for (size_t i = 0; i < bank_size - 1; ++i) {
        if (bank[i] > first_max_value) {
            first_max_value = bank[i];
            first_max_index = i;
        }
    }

    uint8_t second_max_value = 0;
    for (size_t i = first_max_index + 1; i < bank_size; ++i) {
        if (bank[i] > second_max_value) {
            second_max_value = bank[i];
        }
    }

    return (uint64_t)(first_max_value * 10 + second_max_value);
}

uint64_t part2_bank_max(uint8_t* bank, size_t bank_size, size_t battery_count)
{
    uint8_t* counts = malloc(sizeof(uint8_t) * battery_count);

    size_t starting_index = 0;

    for (size_t i = 0; i < battery_count; ++i) {

        size_t max_index = 0;
        uint8_t max_value = 0;
        for (size_t j = starting_index; j < (bank_size - (battery_count - i - 1)); ++j) {
            if (bank[j] > max_value) {
                max_value = bank[j];
                max_index = j;
            }
        }
        counts[i] = max_value;
        starting_index = max_index + 1;
    }

    uint64_t result = 0;
    for (size_t i = 0; i < battery_count; ++i) {
        result = result * 10UL + (uint64_t)counts[i];
    }
    free(counts);
    return result;
}

uint64_t part1(struct joltage_banks* jb)
{
    uint64_t total_max = 0;
    for (size_t b = 0; b < jb->banks; ++b) {
        uint8_t* bank = get_joltage_bank(jb, b);
        uint64_t max_joltage = part1_bank_max(bank, jb->bank_size);
        total_max += max_joltage;
    }
    return total_max;
}

uint64_t part2(struct joltage_banks* jb)
{
    uint64_t total_max = 0;
    for (size_t b = 0; b < jb->banks; ++b) {
        uint8_t* bank = get_joltage_bank(jb, b);
        uint64_t max_joltage = part2_bank_max(bank, jb->bank_size, 12);
        total_max += max_joltage;
    }
    return total_max;
}

void parse_and_run(FILE* input_stream)
{
    struct joltage_banks* jb = parse_joltage_banks(input_stream);

    //print_joltage_banks(jb);
    uint64_t p1_result = part1(jb);
    printf("P1: Total Max Joltage: %lu\n", p1_result);
    uint64_t p2_result = part2(jb);
    printf("P2: Total Max Joltage: %lu\n", p2_result);
    free_joltage_banks(jb);
}

int main()
{
    printf("Test Input:\n");
    const char* test_input = "987654321111111\n"
                             "811111111111119\n"
                             "234234234234278\n"
                             "818181911112111";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream);
    fclose(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("inputs/day3", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream);
    fclose(real_input_stream);

    return 0;
}
