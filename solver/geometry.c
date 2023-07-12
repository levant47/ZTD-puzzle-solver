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
