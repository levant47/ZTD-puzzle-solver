#include <windows.h>

#include "utils.c"
#include "solver/geometry.c"
#include "solver/solver.c"
#include "solver/parser.c"

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
            if (are_strings_equal(some_solution, visualize_solution_to_text(solutions.data[i], parsed_input)))
            {
                matching_solution_found = true;
                break;
            }
        }
        failed = !matching_solution_found;
    }
    if (failed)
    {
        print(filename); print(": failed\n");
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
    test("test files\\puzzle5.txt", 1,
        "AAA\n"
        "AXA\n"
        "AAA\n"
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
    test("test files\\power room.txt", 3,
        "BXSSSAA...\n"
        "BBSSSAACCC\n"
        "BBXSXAAAX.\n"
        "BBSSS.XCCC\n"
        "XGSSSFFF..\n"
        "GPXP.FFFX.\n"
        "GPPPOOFDDD\n"
        "..P.XO.DDD\n"
    );

    print("Done\n");

    return 0;
}
