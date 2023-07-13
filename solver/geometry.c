typedef struct { int x, y; } Position;

Position make_position(int x, int y)
{
    Position result;
    result.x = x;
    result.y = y;
    return result;
}

bool positions_equal(Position left, Position right)
{ return left.x == right.x && left.y == right.y; }

Position add_positions(Position left, Position right)
{ return make_position(left.x + right.x, left.y + right.y); }

Position subtract_positions(Position left, Position right)
{ return make_position(left.x - right.x, left.y - right.y); }

typedef enum
{
    ShapeRotation0 = 0,
    ShapeRotation90,
    ShapeRotation180,
    ShapeRotation270,
} Rotation;

Rotation ALL_ROTATIONS[] = { ShapeRotation0, ShapeRotation90, ShapeRotation180, ShapeRotation270 };

Rotation rotate_back(Rotation rotation)
{ return modulo(-rotation, 4); }

Position rotate_position(Rotation rotation, Position position)
{
    if (rotation == ShapeRotation0) { return make_position(position.x, position.y); }
    else if (rotation == ShapeRotation90) { return make_position(position.y, -position.x); }
    else if (rotation == ShapeRotation180) { return make_position(-position.x, -position.y); }
    else if (rotation == ShapeRotation270) { return make_position(-position.y, position.x); }
    else
    {
        print("Received an unexpected rotation value of ");
        print_number((int)rotation);
        print("\n");
        ExitProcess(1);
    }
}
