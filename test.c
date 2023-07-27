#include <windows.h>

#include "utils.c"
#include "solver/geometry.c"
#include "solver/models.c"
#include "solver/solver.c"
#include "solver/parser.c"

void test(char* filename, int expected_solution_count, char* expected_one_of_solutions)
{
    char* input_file_text = read_input_file(filename);
    GameBoard* board = parse_input(input_file_text);
    Solutions solutions = find_solutions(board);

    bool failed = solutions.count != expected_solution_count;
    if (!failed)
    {
        bool matching_solution_found = false;
        for (int i = 0; i < solutions.count; i++)
        {
            if (are_strings_equal(expected_one_of_solutions, visualize_solution_to_text(solutions.data[i], board)))
            {
                matching_solution_found = true;
                break;
            }
        }
        failed = !matching_solution_found;
    }
    if (failed) { print(filename); print(": failed\n"); }
}

int main()
{
    test("test files\\puzzle1.txt", 1,
        "AXA\n"
        ".A.\n"
        "XBB\n"
    );
    test("test files\\puzzle2.txt", 10,
        ".C.......X\n"
        ".X...III.D\n"
        ".C...IXIDX\n"
        ".BXBBB...D\n"
        "..........\n"
        ".AAAAAX...\n"
        "...F.GXG..\n"
        "..FXF.G...\n"
        "HH........\n"
        "XH...EEXE.\n"
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
