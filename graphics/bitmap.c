typedef unsigned int Pixel;

#define COLOR_BLACK 0xff000000
#define COLOR_BLUE 0xff0000ff
#define COLOR_GREEN 0xff00ff00
#define COLOR_RED 0xffff0000
#define COLOR_CYAN 0xff00ffff
#define COLOR_MAGENTA 0xffff00ff
#define COLOR_YELLOW 0xffffff00
#define COLOR_GRAY 0xff808080
#define COLOR_ORANGE 0xffff8000
#define COLOR_AZURE 0xff0080ff

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

void set_pixel_in_bitmap(int x, int y, Pixel value, Bitmap bitmap)
{
    if (x < 0 || y < 0 || x >= bitmap.width || y >= bitmap.height) { return; }
    bitmap.data[y * bitmap.width_capacity + x] = value;
}

Pixel get_pixel_in_bitmap(int x, int y, Bitmap bitmap) { return bitmap.data[y * bitmap.width_capacity + x]; }
