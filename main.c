// TODO:
// [/] rotations
// [/] better visualization for solutions
// [/] check for shapes overlapping with more than one X
// [/] support non-square fields
// [/] comments in input files
// [/] tests
// [/] check for actual solutions in tests
// [/] input file as command line parameter
// [ ] ignore different rotations of symmetrical shapes (right now power room and puzzle 3 has duplicate solutions because of this)
// [ ] raylib graphics
// [ ] readme

#include <windows.h>

#include "utils.c"
#include "solver.c"

int main(int argument_count, char** argument_data)
{
    init_stdout();

    if (argument_count < 1) { return 1; } // we expect at least one argument which is the program name
    if (argument_count != 2)
    {
        print("USAGE: "); print(argument_data[0]); print(" <path to input file>\n");
        return 1;
    }

    char* input_file_path = argument_data[1];
    char* input_file_text = read_input_file(input_file_path);

    Input* parsed_input = parse_input(input_file_text);

    print("Parsed field of size ");
    print_number(parsed_input->field_width);
    print("x");
    print_number(parsed_input->field_height);
    print(" and ");
    print_number(parsed_input->shapes_count);
    print(" shapes\n");
    for (int i = 0; i < parsed_input->shapes_count; i++)
    {
        print("Shape ");
        print_char(parsed_input->shapes[i].name);
        print(" of size ");
        print_number(parsed_input->shapes[i].width);
        print("x");
        print_number(parsed_input->shapes[i].height);
        print("\n");
    }
    print("\n");

    Solutions solutions = find_solutions(parsed_input);
    print("Found ");
    if (solutions.exceeded) { print("more than "); }
    print_number(solutions.count);
    print(" solution(s)\n\n");
    for (int j = 0; j < solutions.count; j++)
    {
        print("Solution "); print_number(j + 1); print(":\n");
        print(visualize_solution(solutions.data[j], parsed_input));
        print("\n");
    }

    return 0;
}
