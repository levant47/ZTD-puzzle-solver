#define MAX_INPUT_SIZE 1000
#define MAX_FIELD_SIZE 10 // meaning 10 by 10
#define MAX_SHAPE_COUNT 10
#define MAX_X_COUNT 10

typedef struct { int x, y; } Position;

typedef enum
{
    PointInSpaceEmpty = 0,
    PointInSpaceShapeBody,
    PointInSpaceX,
} PointInSpace;

typedef struct
{
    int width;
    int height;
    char name;
    PointInSpace body[MAX_FIELD_SIZE * MAX_FIELD_SIZE];
} Shape;

PointInSpace get_shape_point(int x, int y, Shape shape)
{
    if (x < 0 || y < 0 || x >= shape.width || y >= shape.height) { return PointInSpaceEmpty; }
    return shape.body[y * MAX_FIELD_SIZE + x];
}

Position get_shape_x_position(Shape shape)
{
    for (int x = 0; x < shape.width; x++)
    {
        for (int y = 0; y < shape.height; y++)
        {
            if (get_shape_point(x, y, shape) == PointInSpaceX)
            {
                Position result;
                result.x = x;
                result.y = y;
                return result;
            }
        }
    }
    print("Failed to find an X on shape ");
    print_char(shape.name);
    print("\n");
    ExitProcess(1);
}

// relies on body being initialized to 0 which is PointInSpaceEmpty
void add_point_to_shape(int x, int y, PointInSpace point, Shape* shape)
{
    if (x > MAX_FIELD_SIZE || y > MAX_FIELD_SIZE)
    {
        print("Point shape coordinates out of range, max allowed dimensions are ");
        print_number(MAX_FIELD_SIZE); print("x"); print_number(MAX_FIELD_SIZE);
        print(", got ");
        print_number(x); print("x"); print_number(y);
        print("\n");
        ExitProcess(1);
    }
    shape->body[y * MAX_FIELD_SIZE + x] = point;
    if (x + 1 > shape->width) { shape->width = x + 1; }
    if (y + 1 > shape->height) { shape->height = y + 1; }
}

typedef struct
{
    int field_size;
    int field_xs_count; // TODO: This and shapes_count is supposed to be the same. Maybe add a check in the parsing code for this?
    Position field_xs[MAX_X_COUNT];
    int shapes_count;
    Shape shapes[MAX_SHAPE_COUNT];
} Input;

void add_field_x(int x, int y, Input* input)
{
    if (input->field_xs_count == MAX_X_COUNT) { print("max amount of field Xs exceeded"); ExitProcess(1); }
    input->field_xs[input->field_xs_count].x = x;
    input->field_xs[input->field_xs_count].y = y;
    input->field_xs_count++;
}

void add_shape_to_input(Shape shape, Input* input)
{
    if (input->shapes_count == MAX_SHAPE_COUNT)
    {
        print("Exceeded max amount of shapes: ");
        print_number(MAX_SHAPE_COUNT);
        print("\n");
        ExitProcess(1);
    }
    input->shapes[input->shapes_count] = shape;
    input->shapes_count++;
}

Shape get_shape_by_name(char name, Input input)
{
    for (int i = 0; i < input.shapes_count; i++)
    {
        Shape shape = input.shapes[i];
        if (shape.name == name) { return shape; }
    }
    print("Shape with name ");
    print_char(name);
    print(" not found\n");
    ExitProcess(1);
}

typedef struct
{
    char* source;
    int index;
    int line;
} Parser;

Parser make_parser(char* source)
{
    Parser result;
    result.source = source;
    result.index = 0;
    result.line = 1;
    return result;
}

void next_plus(int count, Parser* parser)
{
    for (int i = 0; i < count; i++)
    {
        if (parser->source[parser->index + i] == '\n')
        {
            parser->line++;
        }
        else if (parser->source[parser->index + i] == '\r' && parser->source[parser->index + i + 1] == '\n')
        {
            parser->line++;
            i++;
            if (i == count)
            {
                print("Stepped over newline\n");
                ExitProcess(1);
            }
        }
    }
    parser->index += count;
}

void next(Parser* parser) { next_plus(1, parser); }

char current_plus(int i, Parser parser) { return parser.source[parser.index + i]; }

char current(Parser parser) { return current_plus(0, parser); }

bool expect(char* prefix, Parser* parser)
{
    int i = 0;
    while (prefix[i] != '\0')
    {
        if (prefix[i] != current_plus(i, *parser)) { return false; }
        i++;
    }
    next_plus(i, parser);
    return true;
}

bool expect_newline(Parser* parser)
{
    if (current(*parser) == '\n')
    {
        next(parser);
        return true;
    }
    if (current(*parser) == '\r' && current_plus(1, *parser) == '\n')
    {
        next_plus(2, parser);
        return true;
    }
    return false;
}

bool expect_eof(Parser parser)
{
    return parser.source[parser.index] == '\0';
}

void parsing_failed(Parser parser)
{
    print("Parsing failed on line ");
    print_number(parser.line);
    print("\n");
    ExitProcess(1);
}

Input* parse_input(char* source)
{
    Input* result = allocate(sizeof(Input));

    Parser parser = make_parser(source);
    while (!expect_eof(parser))
    {
        if (expect("field", &parser))
        {
            if (!expect_newline(&parser))
            {
                print("Expected just 'field' on line ");
                print_number(parser.line);
                print(", but got something else\n");
                ExitProcess(1);
            }

            int field_size = 0;
            int x = 0;
            int y = 0;
            do
            {
                if (y != 0 && y >= field_size)
                {
                    print("Line ");
                    print_number(parser.line);
                    print(" is larger than the rest of the field\n");
                    ExitProcess(1);
                }

                if (current(parser) == '.') { }
                else if (current(parser) == 'X')
                {
                    add_field_x(x, y, result);
                }
                else
                {
                    print("Unrecognized character in field on line ");
                    print_number(parser.line);
                    print("\n");
                    ExitProcess(1);
                }
                x++;
                next(&parser);

                if (expect_newline(&parser) || expect_eof(parser))
                {
                    if (y == 0) { field_size = x; }
                    x = 0;
                    y++;
                }
            }
            while (!expect_newline(&parser) && !expect_eof(parser)); // two newlines in a row or end of the file means we're done with the block
            result->field_size = field_size;
        }
        else if (expect_newline(&parser)) { /* skip blank lines */ }
        else
        {
            char shape_name = current(parser);
            next(&parser);
            if (!expect_newline(&parser))
            {
                print("Expected newline after one-character shape name on line ");
                print_number(parser.line);
                print("\n");
                ExitProcess(1);
            }

            bool x_found = false;
            int x = 0;
            int y = 0;
            Shape shape = {};
            shape.name = shape_name;
            do
            {
                if (current(parser) == ' ') { }
                else if (current(parser) == '.')
                {
                    add_point_to_shape(x, y, PointInSpaceShapeBody, &shape);
                }
                else if (current(parser) == 'X')
                {
                    if (x_found)
                    {
                        print("Found a duplicate X mark on shape ");
                        print_char(shape_name);
                        print("\n");
                        ExitProcess(1);
                    }
                    add_point_to_shape(x, y, PointInSpaceX, &shape);
                    x_found = true;
                }
                else
                {
                    print("Found an unexpected character on line ");
                    print_number(parser.line);
                    print(" while parsing shape ");
                    print_char(shape.name);
                    ExitProcess(1);
                }
                x++;
                next(&parser);

                if (expect_newline(&parser) || expect_eof(parser))
                {
                    x = 0;
                    y++;
                }
            }
            while (!expect_newline(&parser) && !expect_eof(parser));

            if (!x_found)
            {
                print("Did not find X on shape ");
                print_char(shape_name);
                print("\n");
                ExitProcess(1);
            }

            add_shape_to_input(shape, result);
        }
    }

    return result;
}

char* read_input_file()
{
    HANDLE input_file = CreateFile(
            "input.txt",
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );
    if (input_file == INVALID_HANDLE_VALUE)
    {
        print("Failed to open input.txt\n");
        ExitProcess(1);
    }

    char* buffer = allocate(MAX_INPUT_SIZE + 1);
    DWORD bytes_read;
    ReadFile(input_file, buffer, MAX_INPUT_SIZE + 1, &bytes_read, NULL);
    CloseHandle(input_file);
    if (bytes_read == MAX_INPUT_SIZE + 1)
    {
        print("input.txt can not be larger than ");
        print_number(MAX_INPUT_SIZE);
        print(" bytes\n");
        ExitProcess(1);
    }
    buffer[bytes_read] = '\0';

    return buffer;
}

typedef struct
{
    char shape_name;
    int x, y;
} ShapePositionItem;

typedef struct
{
    int count;
    ShapePositionItem items[MAX_SHAPE_COUNT];
} ShapePositions;

void add_shape_position(char name, int x, int y, ShapePositions* positions)
{
    if (positions->count == MAX_SHAPE_COUNT)
    {
        print("Max shape count of ");
        print_number(MAX_SHAPE_COUNT);
        print(" exceeded\n");
        ExitProcess(1);
    }
    positions->items[positions->count].shape_name = name;
    positions->items[positions->count].x = x;
    positions->items[positions->count].y = y;
    positions->count++;
}

bool do_positions_include_shape(char name, ShapePositions positions)
{
    for (int i = 0; i < positions.count; i++)
    {
        if (positions.items[i].shape_name == name)
        {
            return true;
        }
    }
    return false;
}

bool shapes_intersect(Shape shape, ShapePositionItem position, Shape other_shape, ShapePositionItem other_position)
{
    for (int x = 0; x < shape.width; x++)
    {
        for (int y = 0; y < shape.height; y++)
        {
            if (get_shape_point(x, y, shape) != PointInSpaceEmpty && get_shape_point(x + position.x - other_position.x, y + position.y - other_position.y, other_shape) != PointInSpaceEmpty)
            {
                return true;
            }
        }
    }
    return false;
}

bool is_contradictory(ShapePositions positions, Input input)
{
    for (int i = 0; i < positions.count; i++)
    {
        ShapePositionItem position = positions.items[i];
        Shape shape = get_shape_by_name(position.shape_name, input);
        if (position.x < 0 || position.y < 0 || position.x + shape.width > input.field_size || position.y + shape.height > input.field_size) { return true; }
        for (int j = 0; j < positions.count; j++)
        {
            if (i == j) { continue; }
            ShapePositionItem other_position = positions.items[j];
            Shape other_shape = get_shape_by_name(other_position.shape_name, input);
            if (shapes_intersect(shape, position, other_shape, other_position)) { return true; }
        }
    }
    return false;
}

bool is_win(ShapePositions positions, Input input)
{
    return positions.count == input.shapes_count;
}

void print_solutions_loop(ShapePositions* positions, Input* input)
{
    if (is_contradictory(*positions, *input)) { return; }
    if (is_win(*positions, *input))
    {
        for (int i = 0; i < positions->count; i++)
        {
            ShapePositionItem position = positions->items[i];
            print_char(position.shape_name);
            print(": ");
            print_number(position.x); print(", "); print_number(position.y);
            print("\n");
        }
        print("\n");
        return;
    }

    for (int i = 0; i < input->shapes_count; i++)
    {
        Shape shape = input->shapes[i];
        if (do_positions_include_shape(shape.name, *positions)) { continue; }
        for (int j = 0; j < input->field_xs_count; j++)
        {
            Position field_x_position = input->field_xs[j];
            Position shape_x_position = get_shape_x_position(shape);
            add_shape_position(shape.name, field_x_position.x - shape_x_position.x, field_x_position.y - shape_x_position.y, positions);
            print_solutions_loop(positions, input);
            positions->count--;
        }
        break; // each recursive call checks just one shape
    }
}

void print_solutions(Input* input)
{
    print("Solutions:\n");
    ShapePositions positions;
    positions.count = 0;
    print_solutions_loop(&positions, input);
}
