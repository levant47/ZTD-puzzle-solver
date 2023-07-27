// this file defines the main algorithm of the program that finds solutions of a given game board

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
            if (get_shape_piece(pos_in_rot_s1_coords.x, pos_in_rot_s1_coords.y, shape_1) != ShapePieceNone
                && get_shape_piece(pos_in_rot_s2_coords.x, pos_in_rot_s2_coords.y, shape_2) != ShapePieceNone)
            { return true; }
        }
    }
    return false;
}

bool are_board_placements_invalid(BoardPlacements placements, GameBoard board)
{
    for (int i = 0; i < placements.count; i++)
    {
        ShapeBoardPlacement placement = placements.items[i];
        Shape shape = get_shape_by_name(placement.shape_name, board);

        // shape does not go outside the field
        // TODO: maybe this could be simpler
        {
            // // top left corner
            int rotated_x = placement.vector.x;
            if (placement.rotation == ShapeRotation90) { rotated_x -= shape.height - 1; }
            else if (placement.rotation == ShapeRotation180) { rotated_x -= shape.width - 1; }
            int rotated_y = placement.vector.y;
            if (placement.rotation == ShapeRotation180) { rotated_y -= shape.height - 1; }
            else if (placement.rotation == ShapeRotation270) { rotated_y -= shape.width - 1; }
            // bottom right corner
            int rotated_width = placement.rotation == ShapeRotation90 || placement.rotation == ShapeRotation270
                ? shape.height : shape.width;
            int rotated_height = placement.rotation == ShapeRotation90 || placement.rotation == ShapeRotation270
                ? shape.width : shape.height;
            if (rotated_x < 0 || rotated_y < 0
                || rotated_x + rotated_width > board.field_width || rotated_y + rotated_height > board.field_height)
            { return true; }
        }

        // shape does not cover more than one X
        for (int j = 0; j < board.field_xs_count; j++)
        {
            Position x_pos_in_rot_shape_coords = position_in_field_coordinates_to_position_in_rotated_shape_coordinates(
                board.field_xs[j],
                placement
            );
            if (get_shape_piece(x_pos_in_rot_shape_coords.x, x_pos_in_rot_shape_coords.y, shape) == ShapePieceCorpus)
            { return true; }
        }

        // shape does not intersect with other shapes
        for (int j = i + 1; j < placements.count; j++)
        {
            ShapeBoardPlacement other_shape_placement = placements.items[j];
            Shape other_shape = get_shape_by_name(other_shape_placement.shape_name, board);
            if (shapes_intersect(shape, placement, other_shape, other_shape_placement))
            { return true; }
        }
    }
    return false;
}

// there is no point in rotating symmetrical shapes
bool is_shape_rotation_meaningful(Rotation rotation, Shape shape)
{
    if ((rotation == ShapeRotation180 || rotation == ShapeRotation270) && shape.is_vertically_symmetrical)
    { return false; }
    if (rotation == ShapeRotation90 && shape.is_completely_symmetrical)
    { return false; }
    return true;
}

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

void find_solutions_loop(BoardPlacements* placements, GameBoard* board, Solutions* solutions)
{
    // we stop if shape placement is invalid, or we have reached the maximum amount of solutions
    if (are_board_placements_invalid(*placements, *board) || solutions->exceeded) { return; }
    // or if there are no more shapes to place
    if (placements->count == board->shapes_count)
    {
        add_solution(*placements, solutions);
        return;
    }

    // find shape with the lowest amount of possible placements so as to limit the amount of possible placements we
    // have to consider as early as possible
    int target_shape_possible_placements = MAX_S32;
    int target_shape_index = MAX_S32; // so that it would crash immediately in case of a bug
    for (int i = 0; i < board->shapes_count; i++)
    {
        Shape shape = board->shapes[i];
        if (is_shape_already_placed(shape.name, *placements)) { continue; }
        int possible_placements = 0;
        for (int k = 0; k < countof(ALL_ROTATIONS); k++)
        {
            Rotation rotation = ALL_ROTATIONS[k];
            if (!is_shape_rotation_meaningful(rotation, shape)) { continue; }
            Position shape_x_position = get_shape_x_position(rotation, shape);
            for (int j = 0; j < board->field_xs_count; j++)
            {
                Position field_x_position = board->field_xs[j];
                add_board_placement(
                    shape.name,
                    field_x_position.x - shape_x_position.x,
                    field_x_position.y - shape_x_position.y,
                    rotation,
                    placements
                );
                if (!are_board_placements_invalid(*placements, *board)) { possible_placements++; }
                placements->count--;
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
        Shape shape = board->shapes[target_shape_index];
        for (int i = 0; i < countof(ALL_ROTATIONS); i++)
        {
            Rotation rotation = ALL_ROTATIONS[i];
            if (!is_shape_rotation_meaningful(rotation, shape)) { continue; }
            Position shape_x_position = get_shape_x_position(rotation, shape);
            for (int j = 0; j < board->field_xs_count; j++)
            {
                Position field_x_position = board->field_xs[j];
                add_board_placement(
                    shape.name,
                    field_x_position.x - shape_x_position.x,
                    field_x_position.y - shape_x_position.y,
                    rotation,
                    placements
                );
                find_solutions_loop(placements, board, solutions);
                placements->count--;
            }
        }
    }
}

Solutions find_solutions(GameBoard* input)
{
    Solutions solutions;
    solutions.count = 0;
    solutions.exceeded = false;
    BoardPlacements placements;
    placements.count = 0;
    find_solutions_loop(&placements, input, &solutions);
    return solutions;
}

char* visualize_solution_to_text(BoardPlacements placements, GameBoard* board)
{
    // (width + 1) because of newlines and + 1 to the whole expression for the terminating byte
    char* field = allocate((board->field_width + 1) * board->field_height + 1);
    // initialize to an empty field
    for (int x = 0; x < board->field_width + 1; x++)
    {
        for (int y = 0; y < board->field_height; y++)
        {
            field[y * (board->field_width + 1) + x] = x != board->field_width ? '.' : '\n';
        }
    }
    field[(board->field_width + 1) * board->field_height] = '\0';

    // copy shapes
    for (int i = 0; i < placements.count; i++)
    {
        ShapeBoardPlacement position = placements.items[i];
        Shape shape = get_shape_by_name(position.shape_name, *board);
        for (int x = 0; x < shape.width; x++)
        {
            for (int y = 0; y < shape.height; y++)
            {
                ShapePiece point = get_shape_piece(x, y, shape);
                if (point == ShapePieceNone) { continue; }
                Position local_position = make_position(x, y);
                Position field_position = position_in_rotated_shape_coordinates_to_field_coordinates(
                    local_position,
                    position
                );
                field[field_position.y * (board->field_width + 1) + field_position.x] = point == ShapePieceX
                    ? 'X'
                    : shape.name;
            }
        }
    }

    return field;
}
