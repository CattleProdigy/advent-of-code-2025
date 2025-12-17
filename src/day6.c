#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Operation {
    OP_ADD,
    OP_MUL,
};

struct operand {
    uint64_t value;
    size_t prepadding;
    size_t postpadding;
};

struct problem {
    struct operand operands[4];
    enum Operation operation;
};

struct math_homework {
    struct problem* problems;
    size_t num_problems;
    size_t num_operands;
};

void free_math_homework(struct math_homework* mh) { free(mh->problems); }

enum read_codes {
    READ_ERROR = -1,
    READ_NEWLINE = -2,
    READ_OK_NO_NEWLINE = 0,
    READ_OK_NEWLINE = 1,
};

size_t read_spaces(FILE* input_stream)
{
    int ch = ' ';
    size_t spaces = 0;
    do {
        ch = fgetc(input_stream);
        if (ch == ' ') {
            spaces++;
        }
    } while (ch == ' ');
    ungetc(ch, input_stream);
    return spaces;
}

int read_number(FILE* input_stream, uint64_t* out_value)
{
    char buffer[64];
    size_t len = 0;

    int ch = fgetc(input_stream);
    while (isdigit(ch) && len < sizeof(buffer) - 1) {
        buffer[len++] = (char)ch;
        ch = fgetc(input_stream);
    }
    buffer[len] = '\0';
    ungetc(ch, input_stream);

    if (len == 0) {
        return -1;
    }
    int errno = 0;
    *out_value = strtoull(buffer, NULL, 10);
    if (errno != 0) {
        return -1;
    }
    return 0;
}

struct math_homework* parse_math_homework(FILE* input_stream, size_t num_operands)
{
    struct math_homework* mh = malloc(sizeof(struct math_homework));
    memset(mh, 0, sizeof(struct math_homework));

    // first line
    size_t num_problems = 0;
    int ret = 0;
    size_t last_whitespace = SIZE_MAX;
    while (true) {
        size_t prepadding;
        if (last_whitespace == SIZE_MAX) {
            prepadding = read_spaces(input_stream);
        } else {
            prepadding = last_whitespace;
        }
        uint64_t value;
        ret = read_number(input_stream, &value);
        if (ret == -1) {
            fprintf(stderr, "Error reading number\n");
            abort();
        }
        size_t postpadding = read_spaces(input_stream);
        last_whitespace = postpadding;

        mh->problems = realloc(mh->problems, sizeof(struct problem) * (num_problems + 1));
        mh->problems[num_problems].operands[0]
            = (struct operand) { .value = value, .prepadding = prepadding, .postpadding = postpadding };
        ++num_problems;

        int ch = fgetc(input_stream);
        if (ch == '\n') {
            break;
        } else {
            ungetc(ch, input_stream);
        }
    }

    mh->num_problems = num_problems;
    mh->num_operands = num_operands;

    for (size_t op_idx = 1; op_idx < num_operands; ++op_idx) {
        last_whitespace = SIZE_MAX;
        for (size_t i = 0; i < num_problems; ++i) {
            size_t prepadding;
            if (last_whitespace == SIZE_MAX) {
                prepadding = read_spaces(input_stream);
            } else {
                prepadding = last_whitespace;
            }
            uint64_t value;
            ret = read_number(input_stream, &value);
            if (ret == -1) {
                break;
            }
            size_t postpadding = read_spaces(input_stream);
            last_whitespace = postpadding;

            mh->problems[i].operands[op_idx]
                = (struct operand) { .value = value, .prepadding = prepadding, .postpadding = postpadding };
            int ch = fgetc(input_stream);
            if (ch == '\n') {
                continue;
            } else {
                ungetc(ch, input_stream);
            }
        }
    }

    for (size_t i = 0; i < num_problems; ++i) {

        int op_char = ' ';
        do {
            op_char = fgetc(input_stream);
        } while (op_char != '+' && op_char != '*' && op_char != EOF);

        if (op_char == '+') {
            mh->problems[i].operation = OP_ADD;
        } else if (op_char == '*') {
            mh->problems[i].operation = OP_MUL;
        } else {
            // unknown operation
        }
    }

    return mh;
}

void print_math_homework(struct math_homework* mh)
{
    for (size_t i = 0; i < mh->num_problems; ++i) {
        struct problem prob = mh->problems[i];
        char op_char = (prob.operation == OP_ADD) ? '+' : '*';
        for (size_t j = 0; j < mh->num_operands; ++j) {
            printf("%lu %c ", prob.operands[j].value, op_char);
        }
        printf("\n");
    }
}

void part1(struct math_homework* mh)
{
    uint64_t total = 0;
    for (size_t i = 0; i < mh->num_problems; ++i) {
        struct problem prob = mh->problems[i];
        uint64_t result = prob.operands[0].value;
        for (size_t j = 1; j < mh->num_operands; ++j) {
            if (prob.operation == OP_ADD) {
                result += prob.operands[j].value;
            } else if (prob.operation == OP_MUL) {
                result *= prob.operands[j].value;
            }
        }
        total += result;
    }
    printf("Part 1 total: %lu\n", total);
}

size_t count_digits(uint64_t value)
{
    size_t digit_count = 0;
    do {
        digit_count++;
        value /= 10;
    } while (value > 0);
    return digit_count;
}

uint64_t get_digit(uint64_t value, size_t digit_index)
{
    for (size_t i = 0; i < digit_index; ++i) {
        value /= 10;
    }
    return value % 10;
}

void part2(struct math_homework* mh)
{
    // Get max digit counts per problem
    size_t* max_digit_counts = calloc(mh->num_problems, sizeof(uint64_t));
    for (size_t i = 0; i < mh->num_problems; ++i) {
        struct problem prob = mh->problems[i];
        for (size_t j = 0; j < mh->num_operands; ++j) {
            uint64_t value = prob.operands[j].value;
            size_t digit_count = count_digits(value);
            if (digit_count > max_digit_counts[i]) {
                max_digit_counts[i] = digit_count;
            }
        }
    }

    // Disambiguate pre- and post-padding
    for (size_t op_idx = 0; op_idx < mh->num_operands; ++op_idx) {
        size_t carry_postpad = 0;
        bool first = true;
        for (size_t i = 0; i < mh->num_problems; ++i) {
            struct problem prob = mh->problems[i];
            size_t raw_prepad = prob.operands[op_idx].prepadding;

            size_t effective_prepad = raw_prepad - carry_postpad - (first ? 0 : 1);
            first = false;
            size_t digit_count = count_digits(prob.operands[op_idx].value);
            size_t spaces = max_digit_counts[i] - digit_count;
            size_t effective_postpad = spaces - effective_prepad;
            mh->problems[i].operands[op_idx].prepadding = effective_prepad;
            mh->problems[i].operands[op_idx].postpadding = effective_postpad;
            carry_postpad = effective_postpad;
        }
    }

    uint64_t grand_total = 0;
    for (size_t i = 0; i < mh->num_problems; ++i) {
        size_t max_dc = max_digit_counts[i];
        enum Operation op_type = mh->problems[i].operation;
        uint64_t accumulated_result = op_type == OP_ADD ? 0 : 1;

        // For each digit column ...
        for (size_t d = 0; d < max_dc; ++d) {

            // build a "vertical" operand from that digit column
            uint64_t vert_operand = 0;
            uint64_t place_scalar = 1;
            for (size_t jj = mh->num_operands; jj > 0; --jj) {
                size_t j = jj - 1;

                struct operand op = mh->problems[i].operands[j];
                uint64_t value = op.value;
                size_t digit_count = count_digits(value);
                size_t left_shift = max_digit_counts[i] - digit_count - op.prepadding;

                if (d < left_shift) {
                    continue;
                }
                if (d > digit_count + left_shift - 1) {
                    continue;
                }
                size_t effective_read_digit = d - left_shift;

                uint64_t digit = get_digit(value, effective_read_digit);
                vert_operand += digit * place_scalar;
                place_scalar *= 10;
            }

            // Do the operation with the vertical operand
            if (op_type == OP_ADD) {
                accumulated_result += vert_operand;
            } else if (op_type == OP_MUL) {
                accumulated_result *= vert_operand;
            }
        }

        // Accumulate the result
        grand_total += accumulated_result;
    }
    printf("Part 2 total: %lu\n", grand_total);

    free(max_digit_counts);
}

void parse_and_run(FILE* input_stream, size_t num_operands)
{
    struct math_homework* mh = parse_math_homework(input_stream, num_operands);
    // print_math_homework(mh);
    part1(mh);
    part2(mh);

    free_math_homework(mh);
}

int main()
{
    printf("Test Input:\n");
    const char* test_input = "123 328  51 64 \n"
                             " 45 64  387 23 \n"
                             "  6 98  215 314\n"
                             "*   +   *   +  \n";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream, 3);
    fclose(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("../inputs/day6", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream, 4);
    fclose(real_input_stream);

    return 0;
}
