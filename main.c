// TODO:
// [ ] rotations
// [ ] tests
// [ ] input file as command line parameter
// [ ] raylib graphics

#include <windows.h>

#include "utils.c"
#include "solver.c"


int main()
{
    init_stdout();

    char* input_file_text = read_input_file();

    Input* parsed_input = parse_input(input_file_text);

    print("Parsed field of size ");
    print_number(parsed_input->field_size);
    print("x");
    print_number(parsed_input->field_size);
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

    print_solutions(parsed_input);

    return 0;
}
