#define MAX_SOLUTIONS 10

bool shapes_intersect(
    Shape shape_1,
    ShapeBoardPlacement shape_1_placement,
    Shape shape_2,
    ShapeBoardPlacement shape_2_placement
)
{
    for (int x = 0; x < shape_1.width; x++)
    {
        for (int y = 0; y < shape_1.height; y++)
        {
            Position pos_in_rot_s1_coords = make_position(x, y);
            Position pos_in_rot_s2_coords = position_in_field_coordinates_to_position_in_rotated_shape_coordinates(
                position_in_rotated_shape_coordinates_to_field_coordinates(pos_in_rot_s1_coords, shape_1_placement),
                shape_2_placement
            );
            if (get_shape_point(pos_in_rot_s1_coords.x, pos_in_rot_s1_coords.y, shape_1) != PointInSpaceEmpty
                && get_shape_point(pos_in_rot_s2_coords.x, pos_in_rot_s2_coords.y, shape_2) != PointInSpaceEmpty)
            { return true; }
        }
    }
    return false;
}

bool is_contradictory(BoardPlacements positions, GameBoard input)
{
    for (int i = 0; i < positions.count; i++)
    {
        ShapeBoardPlacement position = positions.items[i];
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
            ShapeBoardPlacement other_position = positions.items[j];
            Shape other_shape = get_shape_by_name(other_position.shape_name, input);
            if (shapes_intersect(shape, position, other_shape, other_position))
            { return true; }
        }
    }
    return false;
}

bool is_win(BoardPlacements positions, GameBoard input) { return positions.count == input.shapes_count; }

typedef struct
{
    int count;
    bool exceeded;
    BoardPlacements data[MAX_SOLUTIONS];
} Solutions;

void add_solution(BoardPlacements positions, Solutions* solutions)
{
    if (solutions->count == MAX_SOLUTIONS)
    {
        solutions->exceeded = true;
        return;
    }
    solutions->data[solutions->count] = positions;
    solutions->count++;
}

void find_solutions_loop(BoardPlacements* positions, GameBoard* input, Solutions* solutions)
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
        if (is_shape_already_placed(shape.name, *positions)) { continue; }
        int possible_placements = 0;
        for (int k = 0; k < countof(ALL_ROTATIONS); k++)
        {
            Rotation rotation = ALL_ROTATIONS[k];
            Position shape_x_position = get_shape_x_position(rotation, shape);
            for (int j = 0; j < input->field_xs_count; j++)
            {
                Position field_x_position = input->field_xs[j];
                add_board_placement(
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
                add_board_placement(
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

Solutions find_solutions(GameBoard* input)
{
    Solutions solutions;
    solutions.count = 0;
    solutions.exceeded = false;
    BoardPlacements positions;
    positions.count = 0;
    find_solutions_loop(&positions, input, &solutions);
    return solutions;
}

char* visualize_solution_to_text(BoardPlacements positions, GameBoard* input)
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
        ShapeBoardPlacement position = positions.items[i];
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
