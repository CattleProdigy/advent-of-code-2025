#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct product_id {
    uint64_t start;
    uint64_t end;
};

size_t parse_product_ids(FILE* stream, struct product_id** product_ids)
{

    size_t count = 0;
    int res = 0;
    do {
        uint64_t first;
        uint64_t second;

        int int_read_status = fscanf(stream, "%lu-%lu", &first, &second);
        if (int_read_status != 2) {
            fprintf(stderr, "Failed to read product ID range\n");
            abort();
        }

        if (first >= second) {
            fprintf(stderr, "Invalid product ID range: %lu-%lu\n", first, second);
            abort();
        }

        if (count == 0) {
            *product_ids = malloc(sizeof(struct product_id));
        } else {
            *product_ids = realloc(*product_ids, (count + 1) * sizeof(struct product_id));
        }
        (*product_ids)[count].start = first;
        (*product_ids)[count].end = second;
        ++count;
        res = fgetc(stream);

    } while (res != EOF && res != '\n');

    return count;
}

bool is_invalid(uint64_t value, bool (*match_predicate)(size_t))
{
    // Extract digis
    unsigned char digits[20];
    size_t digit_count = 0;
    uint64_t temp_value = value;
    while (temp_value > 0) {
        digits[digit_count++] = (unsigned char)(temp_value % 10UL);
        temp_value /= 10;
    }

    unsigned char pattern[20];
    bool valid = true;

    // Consider each possible pattern length
    for (size_t i = 1; i <= digit_count / 2; ++i) {
        const size_t pattern_length = i;
        // Pattern length must divide digit count evenly to be a matching invalid paattern
        if (digit_count % pattern_length != 0) {
            continue;
        }

        // Extract the pattern
        for (size_t j = 0; j < pattern_length; ++j) {
            pattern[j] = digits[j];
        }

        // Check that all pattern_length segments match the pattern
        bool all_matches = true;
        size_t matches = 1;
        for (size_t j = pattern_length; j < digit_count; j += pattern_length) {
            bool this_matches = true;
            for (size_t k = 0; k < pattern_length; ++k) {
                if (digits[j + k] != pattern[k]) {
                    this_matches = false;
                    break;
                }
            }
            if (!this_matches) {
                all_matches = false;
                break;
            } else {
                ++matches;
            }
        }

        // If all segments match, check the match predicate to see if this is invalid
        if (all_matches && match_predicate(matches)) {
            return true;
        }
    }
    return !valid;
}

bool match_exactly_two(size_t count) { return count == 2; }
bool match_at_least_two(size_t count) { return count >= 2; }

void part1(size_t n, struct product_id* product_ids)
{
    uint64_t invalid_sum = 0;
    for (size_t i = 0; i < n; ++i) {
        // printf("Checking product ID range: %lu-%lu\n", product_ids[i].start, product_ids[i].end);
        for (uint64_t v = product_ids[i].start; v <= product_ids[i].end; ++v) {
            if (is_invalid(v, match_exactly_two)) {
                // printf("Found invalid product ID: %lu\n", v);
                invalid_sum += v;
            }
        }
    }
    printf("P1: Sum of invalid product IDs: %lu\n", invalid_sum);
}

void part2(size_t n, struct product_id* product_ids)
{
    uint64_t invalid_sum = 0;
    for (size_t i = 0; i < n; ++i) {
        // printf("Checking product ID range: %lu-%lu\n", product_ids[i].start, product_ids[i].end);
        for (uint64_t v = product_ids[i].start; v <= product_ids[i].end; ++v) {
            if (is_invalid(v, match_at_least_two)) {
                // printf("Found invalid product ID: %lu\n", v);
                invalid_sum += v;
            }
        }
    }
    printf("P2: Sum of invalid product IDs: %lu\n", invalid_sum);
}

void parse_and_run(FILE* input_stream)
{
    struct product_id* product_ids = NULL;
    size_t count = parse_product_ids(input_stream, &product_ids);
    part1(count, product_ids);
    part2(count, product_ids);
}

int main()
{
    printf("Test Input:\n");
    const char* test_input = "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,"
                             "1698522-1698528,446443-446449,38593856-38593862,565653-565659,"
                             "824824821-824824827,2121212118-2121212124";
    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("../inputs/day2", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream);

    return 0;
}
