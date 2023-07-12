// TODO:
// [/] rotations
// [/] better visualization for solutions
// [/] check for shapes overlapping with more than one X
// [/] support non-square fields
// [/] comments in input files
// [/] tests
// [/] check for actual solutions in tests
// [/] input file as command line parameter
// [/] ignore different rotations of symmetrical shapes (otherwise, power room and puzzle 3 have duplicate solutions)
// [X] raylib graphics (raylib turned out to be too hard to use because of all of the linking problems)
// [/] WinAPI graphics
// [ ] clean-up
// [ ] readme
// [ ] release a statically linked executable

#include <windows.h>

#include "utils.c"
#include "solver.c"
#include "graphics/bitmap.c"
#include "graphics/renderer.c"

typedef struct
{
    char* path_to_input_file;
    bool output_to_terminal;
    bool debug_mode;
} CliParameters;

typedef struct
{
    bool success;
    char* error;
    CliParameters cli_parameters;
} ParseCliParametersResult;

ParseCliParametersResult make_failed_parse_cli_parameters_result(char* error)
{
    ParseCliParametersResult result;
    result.success = false;
    result.error = error;
    return result;
}

ParseCliParametersResult parse_cli_parameters(int argument_count, char** argument_data)
{
    if (argument_count < 2) { return make_failed_parse_cli_parameters_result("Too few parameters"); }

    CliParameters result;
    set_memory(sizeof(result), &result, 0);

    for (int i = 1 /* skip first parameter which is the program name */; i < argument_count; i++)
    {
        if (are_strings_equal("--text-output", argument_data[i]))
        {
            result.output_to_terminal = true;
        }
        else if (are_strings_equal("--debug", argument_data[i]))
        {
            result.debug_mode = true;
        }
        else if (result.path_to_input_file == NULL)
        {
            result.path_to_input_file = argument_data[i];
        }
        else
        {
            return make_failed_parse_cli_parameters_result("Unrecognized parameter");
        }
    }

    if (result.path_to_input_file == NULL)
    {
        return make_failed_parse_cli_parameters_result("Path to input file is required");
    }

    ParseCliParametersResult parse_result;
    parse_result.success = true;
    parse_result.cli_parameters = result;
    return parse_result;
}

int main(int argument_count, char** argument_data)
{
    init_stdout();

    ParseCliParametersResult parse_cli_parameters_result = parse_cli_parameters(argument_count, argument_data);
    if (!parse_cli_parameters_result.success)
    {
        print("Error: "); print(parse_cli_parameters_result.error); print("\n");
        print("USAGE: "); print(argument_data[0]); print(" [--text-output] [--debug] <path to input file>\n");
        return 1;
    }
    CliParameters cli_parameters = parse_cli_parameters_result.cli_parameters;

    char* input_file_text = read_input_file(cli_parameters.path_to_input_file);

    Input* parsed_input = parse_input(input_file_text);

    if (cli_parameters.debug_mode)
    {
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
    }

    Solutions solutions = find_solutions(parsed_input);

    if (cli_parameters.output_to_terminal)
    {
        print("Found ");
        if (solutions.exceeded) { print("more than "); }
        print_number(solutions.count);
        print(" solution(s)\n\n");
        for (int j = 0; j < solutions.count; j++)
        {
            print("Solution "); print_number(j + 1); print(":\n");
            print(visualize_solution_to_text(solutions.data[j], parsed_input));
            print("\n");
        }
    }
    else
    {
        show_solution_bitmap_in_window(solutions, parsed_input);
    }

    return 0;
}
