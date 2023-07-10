#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define CELL_SIZE_IN_PIXELS 50
#define GRID_COLOR 0xff000000 // black

typedef unsigned int Pixel;

typedef struct
{
    int width, height;
    int width_capacity, height_capacity;
    Pixel* data; // ARGB
} Bitmap;

Bitmap allocate_bitmap(int width, int height)
{
    Bitmap result;
    result.width = width;
    result.height = height;
    result.width_capacity = width * 2;
    result.height_capacity = height * 2;
    result.data = (Pixel*)allocate(result.width_capacity * result.height_capacity * sizeof(*result.data));
    return result;
}

void deallocate_bitmap(Bitmap bitmap)
{
    deallocate(bitmap.data);
}

Pixel ALL_SHAPE_COLORS[] =
{
    0xff0000ff,
    0xff00ff00,
    0xffff0000,
    0xff00ffff,
    0xffff00ff,
    0xffffff00,
    0xff808080,
    0xffff8000,
    0xff0080ff,
};

typedef struct
{
    HDC temporary_device_context;
    HFONT letter_font;
} RenderingState;

RenderingState make_rendering_state()
{
    RenderingState result;
    result.temporary_device_context = NULL;
    result.letter_font = NULL;
    return result;
}

HDC get_temporary_device_context_from_rendering_state(RenderingState* rendering_state)
{
    if (rendering_state->temporary_device_context == NULL)
    {
        rendering_state->temporary_device_context = CreateCompatibleDC(NULL);
    }
    return rendering_state->temporary_device_context;
}

void release_rendering_state(RenderingState* rendering_state)
{
    if (rendering_state->letter_font != NULL)
    {
        DeleteObject(rendering_state->letter_font);
        rendering_state->letter_font = NULL;
    }
    if (rendering_state->temporary_device_context != NULL)
    {
        DeleteDC(rendering_state->temporary_device_context);
        rendering_state->temporary_device_context = NULL;
    }
}

struct
{
    Bitmap screen;
    Solutions solutions;
    Input* input;
    Bitmap letter_bitmaps[128];
    int scroll;
    int max_scroll;
} STATE;

void set_pixel_in_bitmap(int x, int y, Pixel value, Bitmap bitmap)
{
    if (x < 0 || y < 0 || x >= bitmap.width || y >= bitmap.height) { return; }
    bitmap.data[y * bitmap.width_capacity + x] = value;
}

Pixel get_pixel_in_bitmap(int x, int y, Bitmap bitmap) { return bitmap.data[y * bitmap.width_capacity + x]; }

void resize_bitmap(int width, int height, Bitmap* bitmap)
{
    if (bitmap->width_capacity >= width && bitmap->height_capacity >= height)
    {
        bitmap->width = width;
        bitmap->height = height;
        return;
    }
    Bitmap original_bitmap = *bitmap;
    *bitmap = allocate_bitmap(max(width, original_bitmap.width), max(height, original_bitmap.height));
    deallocate_bitmap(original_bitmap);
}

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

void draw_shape_letter(int x0, int y0, int width, int height, char letter, Pixel color, RenderingState* rendering_state, Bitmap bitmap)
{
    Bitmap letter_bitmap = STATE.letter_bitmaps[letter];
    if (letter_bitmap.width == 0) // if it hasn't been cached yet
    {
        HDC temporary_device_context = get_temporary_device_context_from_rendering_state(rendering_state);
        if (rendering_state->letter_font == NULL)
        {
            rendering_state->letter_font = CreateFontA(
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
        }
        SelectObject(temporary_device_context, rendering_state->letter_font);
        TextDimensions letter_dimensions = get_text_dimensions(&letter, 1, temporary_device_context);
        letter_bitmap = allocate_bitmap(letter_dimensions.width, letter_dimensions.height);
        HBITMAP letter_windows_bitmap = CreateCompatibleBitmap(temporary_device_context, letter_dimensions.width, letter_dimensions.height);
        SelectObject(temporary_device_context, letter_windows_bitmap);
        TextOutA(temporary_device_context, 0, 0, &letter, 1);
        for (int y = 0; y < letter_dimensions.height; y++)
        {
            for (int x = 0; x < letter_dimensions.width; x++)
            {
                if (GetPixel(temporary_device_context, x, y) == 0)
                {
                    set_pixel_in_bitmap(x, y, 0xff000000 /* black */, letter_bitmap);
                }
            }
        }
        DeleteObject(letter_windows_bitmap);
        STATE.letter_bitmaps[letter] = letter_bitmap;
    }
    int x_padding = (width - letter_bitmap.width) / 2;
    int y_padding = (height - letter_bitmap.height) / 2;
    for (int y = 0; y < letter_bitmap.height; y++)
    {
        for (int x = 0; x < letter_bitmap.width; x++)
        {
            if (get_pixel_in_bitmap(x, y, letter_bitmap) == 0xff000000 /* black */)
            {
                set_pixel_in_bitmap(x + x0 + x_padding, y + y0 + y_padding, color, bitmap);
            }
        }
    }
}

// returns height of the rendered element
int render_solution(int x0, int x1, int y_padding, ShapePositions positions, Input* input, RenderingState* rendering_state, Bitmap bitmap)
{
    int field_width_in_pixels = CELL_SIZE_IN_PIXELS * input->field_width;
    int field_height_in_pixels = CELL_SIZE_IN_PIXELS * input->field_height;
    int cell_size = CELL_SIZE_IN_PIXELS;
    int x_padding = max(0, (x1 - x0 - field_width_in_pixels) / 2);

    for (int row = 0; row <= input->field_height; row++)
    {
        int adjustment_for_last_row = row == input->field_height ? -1 : 0;
        draw_horizontal_line(x_padding, x_padding + field_width_in_pixels, y_padding + row * cell_size + adjustment_for_last_row, GRID_COLOR, bitmap);
    }
    for (int column = 0; column <= input->field_width; column++)
    {
        int adjustment_for_last_column = column == input->field_width ? -1 : 0;
        draw_vertical_line(x_padding + column * cell_size + adjustment_for_last_column, y_padding, y_padding + field_height_in_pixels, GRID_COLOR, bitmap);
    }

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
                draw_shape_letter(
                    x_padding + field_position.x * CELL_SIZE_IN_PIXELS,
                    y_padding + field_position.y * CELL_SIZE_IN_PIXELS,
                    CELL_SIZE_IN_PIXELS,
                    CELL_SIZE_IN_PIXELS,
                    point == PointInSpaceX ? 'X' : shape.name,
                    ALL_SHAPE_COLORS[i % countof(ALL_SHAPE_COLORS)],
                    rendering_state,
                    bitmap
                );
            }
        }
    }

    return field_height_in_pixels;
}

void render_window(RenderingState* rendering_state, Bitmap bitmap)
{
    // clear the whole screen to white
    for (int y = 0; y < bitmap.height; y++)
    {
        for (int x = 0; x < bitmap.width; x++)
        {
            set_pixel_in_bitmap(x, y, 0xffffffff, bitmap);
        }
    }

    // TODO: render scrollbar

    // render the solution
    int total_y = 10; // initialized with padding from top window edge
    int y = total_y - STATE.scroll;
    // TODO: don't actually render solutions that are beyond the visible area
    for (int i = 0; i < STATE.solutions.count; i++)
    {
        int solution_height = render_solution(0, bitmap.width, y, STATE.solutions.data[i], STATE.input, rendering_state, bitmap);
        y += solution_height;
        total_y += solution_height;
        if (i != STATE.solutions.count - 1) { y += 50; total_y += 50; } // padding between solutions
    }
    total_y += 10;
    total_y -= bitmap.height;

    STATE.max_scroll = total_y;
}

LRESULT window_proc(HWND window_handle, unsigned int message, WPARAM wparam, LPARAM lparam)
{
    // TODO: fix the cursor
    switch (message)
    {
        case WM_PAINT:
        {
            RECT client_rect;
            GetClientRect(window_handle, &client_rect);
            if (client_rect.bottom > STATE.screen.height)
            { // prevent the renderer from showing a blank area at the bottom of the screen after scrolling all the way down and maximizing the window
                STATE.scroll = clamp(0, STATE.scroll, STATE.max_scroll - (client_rect.bottom - STATE.screen.height));
            }
            resize_bitmap(client_rect.right, client_rect.bottom, &STATE.screen);

            PAINTSTRUCT paint_operation;
            HDC device_context = BeginPaint(window_handle, &paint_operation);

            RenderingState rendering_state = make_rendering_state();

            render_window(&rendering_state, STATE.screen);

            BITMAPINFO bitmap_info;
            bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
            bitmap_info.bmiHeader.biWidth = STATE.screen.width_capacity;
            bitmap_info.bmiHeader.biHeight = -STATE.screen.height_capacity;
            bitmap_info.bmiHeader.biPlanes = 1;
            bitmap_info.bmiHeader.biBitCount = 32;
            bitmap_info.bmiHeader.biCompression = BI_RGB;
            StretchDIBits(
                device_context,
                0, 0,
                STATE.screen.width, STATE.screen.height,
                0, 0,
                STATE.screen.width, STATE.screen.height,
                STATE.screen.data,
                &bitmap_info,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            release_rendering_state(&rendering_state);

            EndPaint(window_handle, &paint_operation);

            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            int wheel_distance = -(short)HIWORD(wparam);
            STATE.scroll = clamp(0, STATE.scroll + wheel_distance, STATE.max_scroll);
            InvalidateRect(window_handle, NULL, FALSE);
            return 0;
        }
    }
    return DefWindowProcA(window_handle, message, wparam, lparam);
}

void show_solution_bitmap_in_window(Solutions solutions, Input* input)
{
    STATE.screen = allocate_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);

    set_memory(sizeof(STATE.letter_bitmaps), &STATE.letter_bitmaps, 0);

    STATE.solutions = solutions;
    STATE.input = input;
    STATE.scroll = 0;

    WNDCLASSA window_class;
    set_memory(sizeof(window_class), &window_class, 0);
    window_class.lpfnWndProc = window_proc;
    window_class.lpszClassName = "MyWindowClass";
    window_class.style = CS_HREDRAW | CS_VREDRAW;

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
        STATE.screen.width, STATE.screen.height,
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
