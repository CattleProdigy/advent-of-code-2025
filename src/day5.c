#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct range {
    uint64_t start;
    uint64_t end;
};

struct ingredients {
    struct range* fresh_ranges;
    uint64_t* required;

    size_t num_fresh_ranges;
    size_t num_required;
};

int range_start_comp(const void* a, const void* b)
{
    const struct range* ra = (const struct range*)a;
    const struct range* rb = (const struct range*)b;

    if (ra->start < rb->start) {
        return -1;
    } else if (ra->start > rb->start) {
        return 1;
    } else {
        return 0;
    }
}

int range_start_val_comp(const void* val, const void* range)
{
    const uint64_t* rval = (const uint64_t*)val;
    const struct range* rrange = (const struct range*)range;

    if (*rval < rrange->start) {
        return -1;
    } else if (*rval > rrange->start) {
        return 1;
    } else {
        return 0;
    }
}

void* upper_bound(const void* key, const void* base, size_t nmemb, size_t size, __compar_fn_t compar)
{
    size_t l = 0;
    size_t u = nmemb;

    // smallest X s.t. key <= base[X]

    while (l < u) {
        size_t idx = (l + u) / 2;
        const void* cur = (const void*)(((const char*)base) + (idx * size));
        int comparison = (*compar)(key, cur);
        if (comparison >= 0){
            // key is less than cur, set upper bound here and search below
            l = idx + 1;
        } else {
            // key is greater than cur, set lower bound above and search above
            u = idx;
        }
    }

    if (l > 0) {
        return (void*)(((const char*)base) + ((l) * size));
    }

    return NULL;
}


bool contained_in_range(const struct ingredients* ing, uint64_t value)
{
    void* maybe_matching
        = upper_bound(&value, ing->fresh_ranges, ing->num_fresh_ranges, sizeof(struct range), &range_start_val_comp);

    if (maybe_matching) {

        struct range* next_lower = ((struct range*)maybe_matching) - 1;
        if (next_lower < ing->fresh_ranges) {
            return false;
        }
        struct range* matching_range = (struct range*)next_lower;

        if (value >= matching_range->start && value <= matching_range->end) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool contained_in_range2(const struct ingredients* ing, uint64_t value) {
    for (size_t i = 0; i < ing->num_fresh_ranges; ++i) {
        if (value >= ing->fresh_ranges[i].start && value <= ing->fresh_ranges[i].end) {
            return true;
        }
    }
    return false;
}

void print_ingredients(const struct ingredients* ing)
{
    printf("Fresh Ranges: %lu\n", ing->num_fresh_ranges);
    for (size_t i = 0; i < ing->num_fresh_ranges; ++i) {
        printf("  %lu-%lu\n", ing->fresh_ranges[i].start, ing->fresh_ranges[i].end);
    }

    printf("Required Ingredients:\n");
    for (size_t i = 0; i < ing->num_required; ++i) {
        printf("  %lu\n", ing->required[i]);
    }
}

void sort_ranges(struct ingredients* ing)
{
    if (ing->num_fresh_ranges < 2) {
        return;
    }

    qsort(ing->fresh_ranges, ing->num_fresh_ranges, sizeof(struct range), &range_start_comp);

    size_t merged_count = 0;

    struct range* merged_ranges = malloc(sizeof(struct range) * ing->num_fresh_ranges);

    for (size_t i = 0; i < ing->num_fresh_ranges - 1; ++i) {
        size_t cur_merged_count = 0;
        uint64_t start_to_write = ing->fresh_ranges[i].start;
        uint64_t end_to_write = ing->fresh_ranges[i].end;
        for (size_t j = i + 1; j < ing->num_fresh_ranges; ++j) {
            uint64_t merge_candidate_start = ing->fresh_ranges[j].start;
            uint64_t merge_candidate_end = ing->fresh_ranges[j].end;
            if (merge_candidate_start <= end_to_write + 1) {
                if (merge_candidate_end > end_to_write) {
                    end_to_write = merge_candidate_end;
                }
                ++cur_merged_count;
            } else {
                break;
            }
        }

        merged_ranges[merged_count] = (struct range) { .start = start_to_write, .end = end_to_write };
        ++merged_count;

        i += cur_merged_count;
    }

    ing->num_fresh_ranges = merged_count;
    memcpy(ing->fresh_ranges, merged_ranges, sizeof(struct range) * merged_count);
    free(merged_ranges);
}

void free_ingredients(struct ingredients* ing)
{
    if (ing) {
        free(ing->fresh_ranges);
        free(ing->required);
        free(ing);
    }
}

struct ingredients* parse_ingredients(FILE* input_stream)
{
    struct range* ranges = NULL;
    size_t ranges_count = 0;

    char line_buffer[256];

    while (true) {
        uint64_t start, end;
        char* line_buffer_ret = fgets(line_buffer, sizeof(line_buffer), input_stream);
        if (!line_buffer_ret || line_buffer[0] == '\n') {
            break;
        }
        int scanf_ret = sscanf(line_buffer_ret, "%lu-%lu", &start, &end);
        if (scanf_ret != 2) {
            break;
        }

        ranges = realloc(ranges, sizeof(struct range) * (ranges_count + 1));
        ranges[ranges_count] = (struct range) { .start = start, .end = end };
        ++ranges_count;
    }

    uint64_t* required_list = NULL;
    size_t required_count = 0;
    while (true) {
        uint64_t required;
        char* line_buffer_ret = fgets(line_buffer, sizeof(line_buffer), input_stream);
        if (!line_buffer_ret || line_buffer[0] == '\n') {
            break;
        }

        int scanf_ret = sscanf(line_buffer_ret, "%lu", &required);

        if (scanf_ret != 1) {
            break;
        }

        required_list = realloc(required_list, sizeof(uint64_t) * (required_count + 1));
        required_list[required_count] = required;
        ++required_count;
    }

    struct ingredients* result = malloc(sizeof(struct ingredients));
    *result = (struct ingredients) {
        .fresh_ranges = ranges,
        .required = required_list,
        .num_fresh_ranges = ranges_count,
        .num_required = required_count,
    };
    return result;
}

void part1(const struct ingredients* const ing)
{
    size_t count = 0;
    for (size_t i = 0; i < ing->num_required; ++i) {
        if (contained_in_range(ing, ing->required[i])) {
            ++count;
        } else {
        }
    }
    printf("Part 1: %lu\n", count);
}

void part2(const struct ingredients* const ing)
{
    size_t count = 0;
    for (size_t i = 0; i < ing->num_fresh_ranges; ++i) {
        count += ing->fresh_ranges[i].end - ing->fresh_ranges[i].start + 1;
    }
    printf("Part 2: %lu\n", count);
}

void parse_and_run(FILE* input_stream)
{
    struct ingredients* is = parse_ingredients(input_stream);
    sort_ranges(is);
    part1(is);
    part2(is);

    free_ingredients(is);
}

int main()
{
    printf("Test Input:\n");
    const char* test_input = "3-5\n"
                             "10-14\n"
                             "16-20\n"
                             "12-18\n"
                             "\n"
                             "1\n"
                             "5\n"
                             "8\n"
                             "11\n"
                             "17\n"
                             "32";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream);
    fclose(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("../inputs/day5", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream);
    fclose(real_input_stream);

    return 0;
}
