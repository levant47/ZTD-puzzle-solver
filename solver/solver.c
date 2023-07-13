#define MAX_INPUT_SIZE 1000
#define MAX_FIELD_SIZE 10 // both width and height
#define MAX_SHAPE_COUNT 10
#define MAX_X_COUNT 10
#define MAX_SOLUTIONS 10

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
    bool is_completely_symmetrical;
    bool is_vertically_symmetrical;
    PointInSpace body[MAX_FIELD_SIZE * MAX_FIELD_SIZE];
} Shape;

PointInSpace get_shape_point(int x, int y, Shape shape)
{
    if (x < 0 || y < 0 || x >= shape.width || y >= shape.height) { return PointInSpaceEmpty; }
    return shape.body[y * MAX_FIELD_SIZE + x];
}

// meaning top and bottom halves can be flipped without any effect
bool is_shape_vertically_symmetrical(Shape shape)
{
    for (int x = 0; x < shape.width; x++)
    {
        for (int y = 0; y < shape.height; y++)
        {
            if (get_shape_point(x, y, shape) != get_shape_point(shape.width - x - 1, shape.height - y - 1, shape))
            {
                return false;
            }
            if (x == shape.width / 2 && y == shape.height / 2) { return true; } // no need to check the cells twice
        }
    }
}

bool is_shape_completely_symmetrical(Shape shape)
{
    for (int x = 0; x < shape.width / 2; x++)
    {
        for (int y = 0; y < shape.height; y++)
        {
            // we check whether the shape remains the same if we rotate it by 90 degrees
            // (I hope this check actually guarantees symmetry)
            if (get_shape_point(x, y, shape) != get_shape_point(y, shape.width - x - 1, shape))
            {
                return false;
            }
        }
    }
    return true;
}

Position get_shape_x_position(Rotation rotation, Shape shape)
{
    for (int x = 0; x < shape.width; x++)
    {
        for (int y = 0; y < shape.height; y++)
        {
            if (get_shape_point(x, y, shape) == PointInSpaceX)
            {
                return rotate_position(rotate_back(rotation), make_position(x, y));
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
    input->field_xs[input->field_xs_count] = make_position(x, y);
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
    char shape_name;
    Rotation rotation;
    Position vector;
} ShapePositionItem;

typedef struct
{
    int count;
    ShapePositionItem items[MAX_SHAPE_COUNT];
} ShapePositions;

void add_shape_position(char name, int x, int y, Rotation rotation, ShapePositions* positions)
{
    if (positions->count == MAX_SHAPE_COUNT)
    {
        print("Max shape count of ");
        print_number(MAX_SHAPE_COUNT);
        print(" exceeded\n");
        ExitProcess(1);
    }
    positions->items[positions->count].shape_name = name;
    positions->items[positions->count].vector = make_position(x, y);
    positions->items[positions->count].rotation = rotation;
    positions->count++;
}

bool do_positions_include_shape(char name, ShapePositions positions)
{
    for (int i = 0; i < positions.count; i++)
    { if (positions.items[i].shape_name == name) { return true; } }
    return false;
}

Position position_in_rotated_shape_coordinates_to_field_coordinates(
    Position position_in_rotated_shape_coordinates,
    ShapePositionItem shape_position_in_field_coordinates
)
{
    Position position_in_shape_coordinates = rotate_position(
        rotate_back(shape_position_in_field_coordinates.rotation),
        position_in_rotated_shape_coordinates
    );
    return add_positions(position_in_shape_coordinates, shape_position_in_field_coordinates.vector);
}

Position position_in_field_coordinates_to_position_in_rotated_shape_coordinates(
    Position position_in_field_coordinates,
    ShapePositionItem shape_position_in_field_coordinates
)
{
    Position position_in_shape_coordinates = subtract_positions(
        position_in_field_coordinates,
        shape_position_in_field_coordinates.vector
    );
    return rotate_position(shape_position_in_field_coordinates.rotation, position_in_shape_coordinates);
}

bool shapes_intersect(
    Shape shape_1,
    ShapePositionItem shape_1_position_in_field_coordinates,
    Shape shape_2,
    ShapePositionItem shape_2_position_in_field_coordinates
)
{
    for (int x = 0; x < shape_1.width; x++)
    {
        for (int y = 0; y < shape_1.height; y++)
        {
            Position position_in_rotated_shape_1_coordinates = make_position(x, y);
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
            int actual_x = position.vector.x;
            if (position.rotation == ShapeRotation90) { actual_x -= shape.height - 1; }
            else if (position.rotation == ShapeRotation180) { actual_x -= shape.width - 1; }
            int actual_y = position.vector.y;
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
                Position position_in_rotated_shape_coordinates = make_position(x, y);
                Position position_in_field_coordinates = position_in_rotated_shape_coordinates_to_field_coordinates(
                    position_in_rotated_shape_coordinates,
                    position
                );
                for (int k = 0; k < input.field_xs_count; k++)
                {
                    if (positions_equal(position_in_field_coordinates, input.field_xs[k]))
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
            if (shapes_intersect(shape, position, other_shape, other_position))
            { return true; }
        }
    }
    return false;
}

bool is_win(ShapePositions positions, Input input) { return positions.count == input.shapes_count; }

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
            Rotation rotation = ALL_ROTATIONS[k];
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
            Rotation rotation = ALL_ROTATIONS[k];
            if ((rotation == ShapeRotation180 || rotation == ShapeRotation270) && shape.is_vertically_symmetrical)
            { continue; }
            if (rotation == ShapeRotation90 && shape.is_completely_symmetrical)
            { continue; }
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

char* visualize_solution_to_text(ShapePositions positions, Input* input)
{
    // (width + 1) because of newlines and + 1 to the whole expression for the terminating byte
    char* field = allocate((input->field_width + 1) * input->field_height + 1);
    // initialize to an empty field
    for (int x = 0; x < input->field_width + 1; x++)
    {
        for (int y = 0; y < input->field_height; y++)
        {
            field[y * (input->field_width + 1) + x] = x != input->field_width ? '.' : '\n';
        }
    }
    field[(input->field_width + 1) * input->field_height] = '\0';

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
                Position local_position = make_position(x, y);
                Position field_position = position_in_rotated_shape_coordinates_to_field_coordinates(
                    local_position,
                    position
                );
                field[field_position.y * (input->field_width + 1) + field_position.x] = point == PointInSpaceX
                    ? 'X'
                    : shape.name;
            }
        }
    }

    return field;
}
