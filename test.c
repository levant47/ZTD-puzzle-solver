#include <windows.h>

#include "utils.c"
#include "solver.c"

void test3(char *filename, int field_width, int field_height, int xs_count, int solutions_count, bool exceeded)
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
        print(" test3: failed\n");
    }
}

void test(char* filename, int solution_count, char* some_solution)
{
    char* input_file_text = read_input_file(filename);
    Input* parsed_input = parse_input(input_file_text);
    Solutions solutions = find_solutions(parsed_input);

    bool failed = solutions.count != solution_count;
    if (!failed)
    {
        bool matching_solution_found = false;
        for (int i = 0; i < solutions.count; i++)
        {
            if (are_strings_equal(some_solution, visualize_solution(solutions.data[i], parsed_input)))
            {
                matching_solution_found = true;
                break;
            }
        }
        failed = !matching_solution_found;
    }
    if (failed)
    {
        print(filename); print(" test3 failed\n");
    }
}

int main()
{
    init_stdout();

    test("test files\\puzzle1.txt", 1,
        "AXA\n"
        ".A.\n"
        "XBB\n"
    );
    test("test files\\puzzle2.txt", 10,
        "....AAAAAX\n"
        "BXBBB.D..C\n"
        "..F..DX..X\n"
        ".FXF..D..C\n"
        ".....III..\n"
        ".....IXI..\n"
        ".....GXG..\n"
        ".EEXE.G...\n"
        "HH........\n"
        "XH.....X..\n"
    );
    test("test files\\puzzle3.txt", 10,
        ".........X\n"
        ".X........\n"
        "......X..X\n"
        "..X.......\n"
        "..........\n"
        "......X...\n"
        "......X...\n"
        "...X......\n"
        "..........\n"
        "X......X..\n"
    );
    test("test files\\puzzle4.txt", 4,
        ".B.\n"
        "XBX\n"
        ".BA\n"
    );
    test("test files\\biolab.txt", 1,
        "XCXCCCAAAA\n"
        "BCCCCCAAXA\n"
        "BBXDDDDDAA\n"
        "BBEDDDDDDX\n"
        "GGGDDDDDHH\n"
        "GGGXXHHHXH\n"
        "GGGGFHHHHH\n"
        "GGGFFHHHHH\n"
    );
    test("test files\\trash disposal room.txt", 1,
        "XGGGGGG..F\n"
        "XDG.BBG..F\n"
        "DDG.BXG..F\n"
        "DDGGGGGX.X\n"
        "DD....EEEF\n"
        ".AAA...ECC\n"
        "AXACCCCCCX\n"
        ".AAA....CC\n"
    );

    print("Done\n");

    return 0;
}
