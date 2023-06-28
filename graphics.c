#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define CELL_SIZE_IN_PIXELS 50
#define GRID_COLOR 0xff000000 // black

typedef unsigned int Pixel;

typedef struct
{
    int width, height;
    Pixel* data; // ARGB
} Bitmap;

void set_pixel_in_bitmap(int x, int y, Pixel value, Bitmap bitmap) { bitmap.data[y * bitmap.width + x] = value; }

Pixel get_pixel_in_bitmap(int x, int y, Bitmap bitmap) { return bitmap.data[y * bitmap.width + x]; }

void draw_horizontal_line(int x0, int x1, int y, Pixel color, Bitmap bitmap)
{
    for (int x = x0; x < x1; x++)
    {
        set_pixel_in_bitmap(x, y, color, bitmap);
    }
}

void draw_vertical_line(int x, int y0, int y1, Pixel color, Bitmap bitmap)
{
    for (int y = y0; y < y1; y++)
    {
        set_pixel_in_bitmap(x, y, color, bitmap);
    }
}

typedef struct { int width, height; } TextDimensions;

TextDimensions get_text_dimensions(char* text, int text_length, HDC device_context)
{
    SIZE text_dimensions;
    GetTextExtentPoint32A(device_context, text, text_length, &text_dimensions);
    TextDimensions result;
    result.width = text_dimensions.cx;
    result.height = text_dimensions.cy;
    return result;
}

void draw_shape_letter(int x0, int y0, int width, int height, char letter, Pixel color, HDC device_context, Bitmap bitmap)
{
    TextDimensions letter_dimensions = get_text_dimensions(&letter, 1, device_context);
    HBITMAP letter_bitmap = CreateCompatibleBitmap(device_context, letter_dimensions.width, letter_dimensions.height);
    SelectObject(device_context, letter_bitmap);
    TextOutA(device_context, 0, 0, &letter, 1);
    int x_padding = (width - letter_dimensions.width) / 2;
    int y_padding = (height - letter_dimensions.height) / 2;
    for (int y = 0; y < letter_dimensions.height; y++)
    {
        for (int x = 0; x < letter_dimensions.width; x++)
        {
            if (GetPixel(device_context, x, y) == 0)
            {
                set_pixel_in_bitmap(x + x0 + x_padding, y + y0 + y_padding, color, bitmap);
            }
        }
    }
    DeleteObject(letter_bitmap);
}

Bitmap visualize_solutions_to_bitmap(Solutions solutions, Input* input)
{
    if (solutions.count != 1)
    {
        print("visualize_solutions_to_bitmap only supports visualizing a single solution right now, but got ");
        print_number(solutions.count);
        print("\n");
        ExitProcess(1);
    }

    Bitmap solution_bitmap;
    solution_bitmap.width = CELL_SIZE_IN_PIXELS * input->field_width;
    solution_bitmap.height = CELL_SIZE_IN_PIXELS * input->field_height;
    int data_size = solution_bitmap.width * solution_bitmap.height * sizeof(*solution_bitmap.data);
    solution_bitmap.data = (Pixel*)allocate(data_size);
    set_memory(data_size, solution_bitmap.data, 0);

    int cell_size = solution_bitmap.width / input->field_width; // pixels
    for (int row = 0; row <= input->field_height; row++)
    {
        int adjustment_for_last_row = row == input->field_height ? -1 : 0;
        draw_horizontal_line(0, solution_bitmap.width, row * cell_size + adjustment_for_last_row, GRID_COLOR, solution_bitmap);
    }
    for (int column = 0; column <= input->field_width; column++)
    {
        int adjustment_for_last_column = column == input->field_width ? -1 : 0;
        draw_vertical_line(column * cell_size + adjustment_for_last_column, 0, solution_bitmap.height, GRID_COLOR, solution_bitmap);
    }

    HDC device_context = CreateCompatibleDC(NULL);
    HFONT font = CreateFontA(
        CELL_SIZE_IN_PIXELS * 0.8,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        NULL
    );
    SelectObject(device_context, font);
    ShapePositions positions = solutions.data[0];
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
                // TODO: color per shape
                draw_shape_letter(field_position.x * CELL_SIZE_IN_PIXELS, field_position.y * CELL_SIZE_IN_PIXELS, CELL_SIZE_IN_PIXELS, CELL_SIZE_IN_PIXELS, point == PointInSpaceX ? 'X' : shape.name, 0xffff0000, device_context, solution_bitmap);
            }
        }
    }
    DeleteObject(font);
    DeleteDC(device_context);

    return solution_bitmap;
}

// TODO: put this into a WindowState struct
Bitmap SCREEN;
Bitmap SOLUTIONS_BITMAP;

void render_screen()
{
    // clear the whole screen to white
    for (int y = 0; y < SCREEN.height; y++)
    {
        for (int x = 0; x < SCREEN.width; x++)
        {
            set_pixel_in_bitmap(x, y, 0xffffffff, SCREEN);
        }
    }

    if (SCREEN.width < SOLUTIONS_BITMAP.width)
    {
        MessageBox(/* window handle: */ NULL, "Assertion failed", "Error", MB_OK);
        ExitProcess(1);
    }

    // TODO: support scrolling

    // copy the solution bitmap to the center of the screen
    int top_padding = 10;
    int side_padding = (SCREEN.width - SOLUTIONS_BITMAP.width) / 2;
    for (int y = 0; y < min(SOLUTIONS_BITMAP.height, SCREEN.height - top_padding); y++)
    {
        for (int x = 0; x < SOLUTIONS_BITMAP.width; x++)
        {
            int solutions_bitmap_pixel = get_pixel_in_bitmap(x, y, SOLUTIONS_BITMAP);
            int solutions_bitmap_pixel_alpha = solutions_bitmap_pixel & 0xff000000;
            if (solutions_bitmap_pixel_alpha == 0) { continue; }
            set_pixel_in_bitmap(x + side_padding, y + top_padding, solutions_bitmap_pixel, SCREEN);
        }
    }
}

LRESULT window_proc(HWND window_handle, unsigned int message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT paint_operation;
            HDC device_context = BeginPaint(window_handle, &paint_operation);

            render_screen();

            BITMAPINFO bitmap_info;
            bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
            bitmap_info.bmiHeader.biWidth = SCREEN.width;
            bitmap_info.bmiHeader.biHeight = -SCREEN.height;
            bitmap_info.bmiHeader.biPlanes = 1;
            bitmap_info.bmiHeader.biBitCount = 32;
            bitmap_info.bmiHeader.biCompression = BI_RGB;
            StretchDIBits(
                device_context,
                0, 0,
                SCREEN.width, SCREEN.height,
                0, 0,
                SCREEN.width, SCREEN.height,
                SCREEN.data,
                &bitmap_info,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            EndPaint(window_handle, &paint_operation);

            return 0;
        }
    }
    return DefWindowProcA(window_handle, message, wparam, lparam);
}

void show_solution_bitmap_in_window(Bitmap solutions_bitmap)
{
    SCREEN.width = SCREEN_WIDTH;
    SCREEN.height = SCREEN_HEIGHT;
    SCREEN.data = (int*)allocate(SCREEN.width * SCREEN.height * sizeof(*SCREEN.data));

    SOLUTIONS_BITMAP = solutions_bitmap;

    WNDCLASSA window_class;
    set_memory(sizeof(window_class), &window_class, 0);
    window_class.lpfnWndProc = window_proc;
    window_class.lpszClassName = "MyWindowClass";

    ATOM window_class_atom = RegisterClassA(&window_class);
    if (window_class_atom == 0)
    {
        MessageBox(/* window handle: */ NULL, "RegisterClassA failed", "Error", MB_OK);
        return;
    }

    HWND window_handle = CreateWindowExA(
        /* dwExStyle: */ 0,
        window_class.lpszClassName,
        "My Window",
        // either WS_VISIBLE or a ShowWindow call afterwards is required for the window to appear
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        /* x, y */ CW_USEDEFAULT, CW_USEDEFAULT,
        SCREEN.width, SCREEN.height,
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (window_handle == NULL)
    {
        MessageBox(/* window handle: */ NULL, "CreateWindowExA failed", "Error", MB_OK);
        return;
    }

    // GetMessage technically can return -1 indicating an error, but that error
    // doesn't mean anything, so we won't handle it
    for (MSG message; GetMessage(&message, window_handle, 0, 0) > 0; )
    {
        TranslateMessage(&message); // the return value is inconsistent so we can ignore it
        DispatchMessageA(&message);
    }
}
