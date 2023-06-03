#include <windows.h>

#include "utils.c"
#include "solver.c"

void test(char *filename, int field_size, int xs_count, int solutions_count, bool exceeded)
{
    char* input_file_text = read_input_file(filename);
    Input* parsed_input = parse_input(input_file_text);
    Solutions solutions = find_solutions(parsed_input);

    if (
        parsed_input->field_size != field_size
            || parsed_input->field_xs_count != xs_count
            || solutions.count != solutions_count
            || solutions.exceeded != exceeded
    )
    {
        print(filename);
        print(" test: failed\n");
    }
}

int main()
{
    init_stdout();

    test("test files\\puzzle1.txt", 3, 2, 1, false);
    test("test files\\puzzle2.txt", 10, 10, 10, true);
    test("test files\\puzzle3.txt", 10, 10, 10, true);

    print("Done\n");

    return 0;
}
