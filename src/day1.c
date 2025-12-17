#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Direction { LEFT, RIGHT };

struct Instruction {
    enum Direction direction;
    unsigned int distance;
};

size_t parse_instructions(FILE* stream, struct Instruction** instruction)
{
    size_t count = 0;

    while (ferror(stream) == 0) {
        enum Direction dir;
        unsigned int distance;

        int dir_char = fgetc(stream);
        if (dir_char == 'L') {
            dir = LEFT;
        } else if (dir_char == 'R') {
            dir = RIGHT;
        } else if (isspace(dir_char)) {
            // Skip whitespace characters
            continue;
        } else if (dir_char == EOF) {
            // End of file reached
            break;
        } else {
            fprintf(stderr, "Found instruction %c\n", dir_char);
            abort();
        }

        int int_read_status = fscanf(stream, "%u", &distance);

        if (int_read_status != 1) {
            fprintf(stderr, "Failed to read distance\n");
            abort();
        }

        if (distance == 0) {
            fprintf(stderr, "Distance cannot be zero\n");
            abort();
        }

        if (count == 0) {
            *instruction = malloc(sizeof(struct Instruction));
        } else {
            *instruction = realloc(*instruction, (count + 1) * sizeof(struct Instruction));
        }
        (*instruction)[count].direction = dir;
        (*instruction)[count].distance = distance;
        ++count;
    }

    return count;
}

struct Result {
    int final_position;
    size_t zero_counts;
    size_t part2_zero_counts;
};

struct Result run_dial(int starting_point, struct Instruction* instructions, size_t instr_count)
{
    int current_position = starting_point;
    size_t zero_counts = 0;
    size_t part2_counts = 0;
    for (size_t i = 0; i < instr_count; ++i) {
        bool starting_at_zero = (current_position == 0);
        if (instructions[i].direction == LEFT) {
            current_position = (current_position - (int)instructions[i].distance);
        } else {
            current_position = (current_position + (int)instructions[i].distance);
        }

        size_t rollovers = 0;
        while (current_position < 0) {
            current_position += 100;
            ++rollovers;
        }

        while (current_position > 99) {
            current_position -= 100;
            ++rollovers;
        }

        if (!starting_at_zero && rollovers == 0) {
            if (current_position == 0) {
                ++part2_counts;
                ++zero_counts;
            }
        } else if (!starting_at_zero && rollovers > 0) {
            part2_counts += rollovers;
            if (current_position == 0) {
                if (instructions[i].direction == LEFT) {
                    ++part2_counts;
                }
                ++zero_counts;
            }
        } else if (starting_at_zero && rollovers == 0) {
            if (current_position == 0) {
                abort(); // Cannot start at zero and end at zero with no rollovers
            } else {
                // Starting at zero, ending away from zero, no rollovers, do not
                // increment
            }
        } else if (starting_at_zero && rollovers > 0) {
            part2_counts += rollovers;
            if (current_position == 0) {
                ++zero_counts;
            } else if (instructions[i].direction == LEFT) {
                --part2_counts;
            }
        } else {
            abort();
        }
    }

    struct Result result
        = { .final_position = current_position, .zero_counts = zero_counts, .part2_zero_counts = part2_counts };
    return result;
}

void part1(FILE* input_stream)
{
    struct Instruction* instructions = NULL;
    size_t instr_count = parse_instructions(input_stream, &instructions);

    struct Result result = run_dial(50, instructions, instr_count);
    printf("Final Position: %d\n", result.final_position);
    printf("Zero Counts: %zu\n", result.zero_counts);
    printf("Part2: Zero Counts: %zu\n", result.part2_zero_counts);

    if (instructions) {
        free(instructions);
    }
}

int main()
{
    // const char* test_input = "L68\nL30\nR48\nL5\nR60\nL55\nL1\nL99\nR14\nL82";

    const char* test_input = "L50\nR250\nR50\nR100\nL100\nL50\nR50\nR200\nL200\nR50\nL150\nR50\nR1000"
                             "\nR150\nL68\nL30\nR48\nL5\nR60\nL55\nL1\nL99\nR14\nL82";

    FILE* stream = fmemopen((void*)test_input, strlen(test_input), "r");
    printf("Test Input:\n");
    part1(stream);
    fclose(stream);

    FILE* real_input_stream = fopen("../inputs/day1", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }

    printf("Real Input:\n");
    part1(real_input_stream);

    fclose(real_input_stream);

    return 0;
}
