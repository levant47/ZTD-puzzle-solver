typedef unsigned int Pixel;

const Pixel COLOR_BLACK = 0xff000000;
const Pixel COLOR_BLUE = 0xff0000ff;
const Pixel COLOR_GREEN = 0xff00ff00;
const Pixel COLOR_RED = 0xffff0000;
const Pixel COLOR_CYAN = 0xff00ffff;
const Pixel COLOR_MAGENTA = 0xffff00ff;
const Pixel COLOR_YELLOW = 0xffffff00;
const Pixel COLOR_GRAY = 0xff808080;
const Pixel COLOR_ORANGE = 0xffff8000;
const Pixel COLOR_AZURE = 0xff0080ff;

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
