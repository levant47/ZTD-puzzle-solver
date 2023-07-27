#define MAX_INPUT_SIZE 1000
#define MAX_FIELD_SIZE 10 // both width and height
#define MAX_SHAPE_COUNT 10
#define MAX_X_COUNT 10

typedef enum
{
    PointInSpaceEmpty = 0,
    PointInSpaceShapeBody,
    PointInSpaceX,
} PointInSpace;

typedef struct
{
    int width, height;
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
            { return false; }
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
} GameBoard;

void add_field_x_to_board(int x, int y, GameBoard* board)
{
    if (board->field_xs_count == MAX_X_COUNT) { print("max amount of field Xs exceeded"); ExitProcess(1); }
    board->field_xs[board->field_xs_count] = make_position(x, y);
    board->field_xs_count++;
}

void add_shape_to_board(Shape shape, GameBoard* input)
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

Shape get_shape_by_name(char name, GameBoard input)
{
    for (int i = 0; i < input.shapes_count; i++)
    {
        Shape shape = input.shapes[i];
        if (shape.name == name) { return shape; }
    }
    print("Shape with name "); print_char(name); print(" not found\n");
    ExitProcess(1);
}

typedef struct
{
    char shape_name;
    Rotation rotation;
    Position vector;
} ShapeBoardPlacement;

typedef struct
{
    int count;
    ShapeBoardPlacement items[MAX_SHAPE_COUNT];
} BoardPlacements;

void add_board_placement(char name, int x, int y, Rotation rotation, BoardPlacements* placements)
{
    if (placements->count == MAX_SHAPE_COUNT)
    {
        print("Max shape count of "); print_number(MAX_SHAPE_COUNT); print(" exceeded\n");
        ExitProcess(1);
    }
    placements->items[placements->count].shape_name = name;
    placements->items[placements->count].vector = make_position(x, y);
    placements->items[placements->count].rotation = rotation;
    placements->count++;
}

bool is_shape_already_placed(char shape_name, BoardPlacements placements)
{
    for (int i = 0; i < placements.count; i++)
    {
        if (placements.items[i].shape_name == shape_name) { return true; }
    }
    return false;
}

Position position_in_rotated_shape_coordinates_to_field_coordinates(
    Position position_in_rotated_shape_coordinates,
    ShapeBoardPlacement placement
)
{
    Position position_in_shape_coordinates = rotate_position(
        rotate_back(placement.rotation),
        position_in_rotated_shape_coordinates
    );
    return add_positions(position_in_shape_coordinates, placement.vector);
}

Position position_in_field_coordinates_to_position_in_rotated_shape_coordinates(
    Position position_in_field_coordinates,
    ShapeBoardPlacement placement
)
{
    Position position_in_shape_coordinates = subtract_positions(
        position_in_field_coordinates,
        placement.vector
    );
    return rotate_position(placement.rotation, position_in_shape_coordinates);
}
