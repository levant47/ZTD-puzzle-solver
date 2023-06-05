#include <windows.h>

#include "utils.c"
#include "solver.c"

void test(char *filename, int field_width, int field_height, int xs_count, int solutions_count, bool exceeded)
{
    char* input_file_text = read_input_file(filename);
    Input* parsed_input = parse_input(input_file_text);
    Solutions solutions = find_solutions(parsed_input);

    if (
        parsed_input->field_width != field_width
            || parsed_input->field_height != field_height
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

    test("test files\\puzzle1.txt", 3, 3, 2, 1, false);
    test("test files\\puzzle2.txt", 10, 10, 10, 10, true);
    test("test files\\puzzle3.txt", 10, 10, 10, 10, true);
    test("test files\\puzzle4.txt", 3, 3, 2, 4, false);
    test("test files\\biolab.txt", 10, 8, 8, 1, false);
    test("test files\\trash disposal room.txt", 10, 8, 7, 1, false);

    print("Done\n");

    return 0;
}
