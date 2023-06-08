#define MAX_INPUT_SIZE 1000
#define MAX_FIELD_SIZE 10 // both width and height
#define MAX_SHAPE_COUNT 10
#define MAX_X_COUNT 10
#define MAX_SOLUTIONS 10

typedef struct { int x, y; } Position;

typedef enum
{
    ShapeRotation0 = 0,
    ShapeRotation90,
    ShapeRotation180,
    ShapeRotation270,
} ShapeRotation;

ShapeRotation ALL_ROTATIONS[] = { ShapeRotation0, ShapeRotation90, ShapeRotation180, ShapeRotation270 };

int rotation_to_degrees(ShapeRotation rotation)
{
    switch (rotation)
    {
        case ShapeRotation0: return 0;
        case ShapeRotation90: return 90;
        case ShapeRotation180: return 180;
        case ShapeRotation270: return 270;
        default:
            print("Received an unexpected rotation value of ");
            print_number((int)rotation);
            print("\n");
            ExitProcess(1);
    }
}

ShapeRotation degrees_to_rotation(int degrees)
{
    switch (degrees)
    {
        case 0: return ShapeRotation0;
        case 90: return ShapeRotation90;
        case 180: return ShapeRotation180;
        case 270: return ShapeRotation270;
        default:
            print("Received an unexpected degrees value of ");
            print_number(degrees);
            print("\n");
            ExitProcess(1);
    }
}

Position rotate_position(ShapeRotation rotation, Position position)
{
    Position result;
    if (rotation == ShapeRotation0)
    {
        result.x = position.x;
        result.y = position.y;
    }
    else if (rotation == ShapeRotation90)
    {
        result.x = position.y;
        result.y = -position.x;
    }
    else if (rotation == ShapeRotation180)
    {
        result.x = -position.x;
        result.y = -position.y;
    }
    else if (rotation == ShapeRotation270)
    {
        result.x = -position.y;
        result.y = position.x;
    }
    else
    {
        print("Received an unexpected rotation value of ");
        print_number((int)rotation);
        print("\n");
        ExitProcess(1);
    }
    return result;
}

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

Position get_shape_x_position(ShapeRotation rotation, Shape shape)
{
    for (int x = 0; x < shape.width; x++)
    {
        for (int y = 0; y < shape.height; y++)
        {
            if (get_shape_point(x, y, shape) == PointInSpaceX)
            {
                Position vector;
                vector.x = x;
                vector.y = y;
                return rotate_position(degrees_to_rotation(modulo(-rotation_to_degrees(rotation), 360)), vector);
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
    int field_width, field_height;
    int field_xs_count;
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

bool expect_eof(Parser parser) { return parser.source[parser.index] == '\0'; }

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

            int field_width = 0;
            int x = 0;
            int y = 0;
            do
            {
                if (y != 0 && y >= field_width)
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
                    if (y == 0) { field_width = x; }
                    x = 0;
                    y++;
                }
            }
            while (!expect_newline(&parser) && !expect_eof(parser)); // two newlines in a row or end of the file means we're done with the block
            result->field_width = field_width;
            result->field_height = y;
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
            Shape shape;
            set_memory(sizeof(shape), &shape, 0);
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

    if (result->shapes_count != result->field_xs_count)
    {
        print("Expected an equal amount of shapes an Xs on the field, instead got ");
        print_number(result->field_xs_count); print(" Xs and ");
        print_number(result->shapes_count); print(" shapes\n");
        ExitProcess(1);
    }

    return result;
}

char* read_input_file(char* filename)
{
    HANDLE input_file = CreateFile(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (input_file == INVALID_HANDLE_VALUE)
    {
        print("Failed to open \""); print(filename); print("\"\n");
        ExitProcess(1);
    }

    char* buffer = allocate(MAX_INPUT_SIZE + 1);
    DWORD bytes_read;
    ReadFile(input_file, buffer, MAX_INPUT_SIZE + 1, &bytes_read, NULL);
    CloseHandle(input_file);
    if (bytes_read == MAX_INPUT_SIZE + 1)
    {
        print("Input file can not be larger than ");
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
    ShapeRotation rotation;
    int x, y;
} ShapePositionItem;

typedef struct
{
    int count;
    ShapePositionItem items[MAX_SHAPE_COUNT];
} ShapePositions;

void add_shape_position(char name, int x, int y, ShapeRotation rotation, ShapePositions* positions)
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
    positions->items[positions->count].rotation = rotation;
    positions->count++;
}

bool do_positions_include_shape(char name, ShapePositions positions)
{
    for (int i = 0; i < positions.count; i++)
    { if (positions.items[i].shape_name == name) { return true; } }
    return false;
}

Position position_in_rotated_shape_coordinates_to_field_coordinates(Position position_in_rotated_shape_coordinates, ShapePositionItem shape_position_in_field_coordinates)
{
    Position position_in_shape_coordinates = rotate_position(
        degrees_to_rotation(modulo(-rotation_to_degrees(shape_position_in_field_coordinates.rotation), 360)),
        position_in_rotated_shape_coordinates
    );
    Position position_in_field_coordinates;
    position_in_field_coordinates.x = position_in_shape_coordinates.x + shape_position_in_field_coordinates.x;
    position_in_field_coordinates.y = position_in_shape_coordinates.y + shape_position_in_field_coordinates.y;
    return position_in_field_coordinates;
}

Position position_in_field_coordinates_to_position_in_rotated_shape_coordinates(Position position_in_field_coordinates, ShapePositionItem shape_position_in_field_coordinates)
{
    Position position_in_rotated_shape_coordinates;
    position_in_rotated_shape_coordinates.x = position_in_field_coordinates.x - shape_position_in_field_coordinates.x;
    position_in_rotated_shape_coordinates.y = position_in_field_coordinates.y - shape_position_in_field_coordinates.y;
    return rotate_position(shape_position_in_field_coordinates.rotation, position_in_rotated_shape_coordinates);
}

bool shapes_intersect(Shape shape_1, ShapePositionItem shape_1_position_in_field_coordinates, Shape shape_2, ShapePositionItem shape_2_position_in_field_coordinates)
{
    for (int x = 0; x < shape_1.width; x++)
    {
        for (int y = 0; y < shape_1.height; y++)
        {
            Position position_in_rotated_shape_1_coordinates;
            position_in_rotated_shape_1_coordinates.x = x;
            position_in_rotated_shape_1_coordinates.y = y;
            Position position_in_rotated_shape_2_coordinates = position_in_field_coordinates_to_position_in_rotated_shape_coordinates(
                position_in_rotated_shape_coordinates_to_field_coordinates(
                    position_in_rotated_shape_1_coordinates,
                    shape_1_position_in_field_coordinates
                ),
                shape_2_position_in_field_coordinates
            );
            if (get_shape_point(position_in_rotated_shape_1_coordinates.x, position_in_rotated_shape_1_coordinates.y, shape_1) != PointInSpaceEmpty
                && get_shape_point(position_in_rotated_shape_2_coordinates.x, position_in_rotated_shape_2_coordinates.y, shape_2) != PointInSpaceEmpty)
            { return true; }
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

        // shape does not go outside the field
        {
            // top left corner
            int actual_x = position.x;
            if (position.rotation == ShapeRotation90) { actual_x -= shape.height - 1; }
            else if (position.rotation == ShapeRotation180) { actual_x -= shape.width - 1; }
            int actual_y = position.y;
            if (position.rotation == ShapeRotation180) { actual_y -= shape.height - 1; }
            else if (position.rotation == ShapeRotation270) { actual_y -= shape.width - 1; }
            // bottom right corner
            int actual_width = position.rotation == ShapeRotation90 || position.rotation == ShapeRotation270
                ? shape.height : shape.width;
            int actual_height = position.rotation == ShapeRotation90 || position.rotation == ShapeRotation270
                ? shape.width : shape.height;
            if (actual_x < 0 || actual_y < 0
                || actual_x + actual_width > input.field_width || actual_y + actual_height > input.field_height)
            { return true; }
        }

        // shape contains only one X
        bool one_x_found = false;
        for (int x = 0; x < shape.width; x++)
        {
            for (int y = 0; y < shape.height; y++)
            {
                if (get_shape_point(x, y, shape) == PointInSpaceEmpty) { continue; }
                Position position_in_rotated_shape_coordinates;
                position_in_rotated_shape_coordinates.x = x;
                position_in_rotated_shape_coordinates.y = y;
                Position position_in_field_coordinates = position_in_rotated_shape_coordinates_to_field_coordinates(position_in_rotated_shape_coordinates, position);
                for (int k = 0; k < input.field_xs_count; k++)
                {
                    if (position_in_field_coordinates.x == input.field_xs[k].x && position_in_field_coordinates.y == input.field_xs[k].y)
                    {
                        if (one_x_found) { return true; }
                        one_x_found = true;
                    }
                }
            }
        }

        // shape does not intersect other shapes
        for (int j = i + 1; j < positions.count; j++)
        {
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

typedef struct
{
    int count;
    bool exceeded;
    ShapePositions data[MAX_SOLUTIONS];
} Solutions;

void add_solution(ShapePositions positions, Solutions* solutions)
{
    if (solutions->count == MAX_SOLUTIONS)
    {
        solutions->exceeded = true;
        return;
    }
    solutions->data[solutions->count] = positions;
    solutions->count++;
}

void find_solutions_loop(ShapePositions* positions, Input* input, Solutions* solutions)
{
    if (is_contradictory(*positions, *input) || solutions->exceeded) { return; }
    if (is_win(*positions, *input))
    {
        add_solution(*positions, solutions);
        return;
    }

    // find shape with the lowest amount of possible placements
    int target_shape_possible_placements = MAX_S32;
    int target_shape_index = MAX_S32; // so that it would crash immediately in case of a bug
    for (int i = 0; i < input->shapes_count; i++)
    {
        Shape shape = input->shapes[i];
        if (do_positions_include_shape(shape.name, *positions)) { continue; }
        int possible_placements = 0;
        for (int k = 0; k < countof(ALL_ROTATIONS); k++)
        {
            ShapeRotation rotation = ALL_ROTATIONS[k];
            Position shape_x_position = get_shape_x_position(rotation, shape);
            for (int j = 0; j < input->field_xs_count; j++)
            {
                Position field_x_position = input->field_xs[j];
                add_shape_position(
                    shape.name,
                    field_x_position.x - shape_x_position.x,
                    field_x_position.y - shape_x_position.y,
                    rotation,
                    positions
                );
                if (!is_contradictory(*positions, *input)) { possible_placements++; }
                positions->count--;
            }
        }
        if (possible_placements < target_shape_possible_placements)
        {
             target_shape_possible_placements = possible_placements;
             target_shape_index = i;
        }
    }
    // if there is nowhere left to place at least one shape, then this is an incorrect solution
    if (target_shape_possible_placements == 0) { return; }

    // loop through all possible placements of the shape with the lowest amount of possible placements
    {
        Shape shape = input->shapes[target_shape_index];
        for (int k = 0; k < countof(ALL_ROTATIONS); k++)
        {
            ShapeRotation rotation = ALL_ROTATIONS[k];
            Position shape_x_position = get_shape_x_position(rotation, shape);
            for (int j = 0; j < input->field_xs_count; j++)
            {
                Position field_x_position = input->field_xs[j];
                add_shape_position(
                    shape.name,
                    field_x_position.x - shape_x_position.x,
                    field_x_position.y - shape_x_position.y,
                    rotation,
                    positions
                );
                find_solutions_loop(positions, input, solutions);
                positions->count--;
            }
        }
    }
}

Solutions find_solutions(Input* input)
{
    Solutions solutions;
    solutions.count = 0;
    solutions.exceeded = false;
    ShapePositions positions;
    positions.count = 0;
    find_solutions_loop(&positions, input, &solutions);
    return solutions;
}

void visualize_solution(ShapePositions positions, Input* input)
{
    char* field = allocate(input->field_width * input->field_height);
    // initialize to an empty field
    for (int x = 0; x < input->field_width; x++)
    {
        for (int y = 0; y < input->field_height; y++)
        {
            field[y * input->field_width + x] = '.';
        }
    }

    // copy shapes
    for (int i = 0; i < positions.count; i++)
    {
        ShapePositionItem position = positions.items[i];
        Shape shape = get_shape_by_name(position.shape_name, *input);
        for (int x = 0; x < shape.width; x++)
        {
            for (int y = 0; y < shape.height; y++)
            {
                PointInSpace point = get_shape_point(x, y, shape);
                if (point == PointInSpaceEmpty) { continue; }
                Position local_position;
                local_position.x = x;
                local_position.y = y;
                Position field_position = rotate_position(degrees_to_rotation(modulo(-rotation_to_degrees(position.rotation), 360)), local_position);
                field_position.x += position.x;
                field_position.y += position.y;
                field[field_position.y * input->field_width + field_position.x] = point == PointInSpaceX ? 'X' : shape.name;
            }
        }
    }

    // output
    for (int y = 0; y < input->field_height; y++)
    {
        for (int x = 0; x < input->field_width; x++)
        {
            print_char(field[y * input->field_width + x]);
        }
        print("\n");
    }
}
