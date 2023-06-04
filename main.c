// TODO:
// [/] rotations
// [ ] better visualization for solutions
// [ ] check for shapes overlapping more than one X
// [/] tests
// [ ] input file as command line parameter
// [ ] raylib graphics

#include <windows.h>

#include "utils.c"
#include "solver.c"

int main()
{
    init_stdout();

    char* input_file_text = read_input_file("test files\\puzzle4.txt");

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

    Solutions solutions = find_solutions(parsed_input);
    print("Found ");
    if (solutions.exceeded) { print("more than "); }
    print_number(solutions.count);
    print(" solution(s)\n\n");
    for (int j = 0; j < solutions.count; j++)
    {
        print("Solution "); print_number(j + 1); print(":\n");
        ShapePositions positions = solutions.data[j];
        for (int i = 0; i < positions.count; i++)
        {
            ShapePositionItem position = positions.items[i];
            print_char(position.shape_name);
            print(": ");
            print_number(position.x); print(", "); print_number(position.y);
            print(" at "); print_number(rotation_to_degrees(position.rotation)); print(" degrees\n");
        }
        print("\n");
    }

    return 0;
}
