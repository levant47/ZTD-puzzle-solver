typedef int bool;
#define true 1
#define false 0

int get_c_string_length(char* string)
{
    int result = 0;
    while (string[result] != '\0') { result++; }
    return result;
}

HANDLE STDOUT;

void init_stdout()
{
    STDOUT = GetStdHandle(STD_OUTPUT_HANDLE);
}

void print(char* message)
{
    DWORD _bytes_written;
    WriteFile(STDOUT, message, get_c_string_length(message), &_bytes_written, NULL);
}

void print_char(char c)
{
    char buffer[2];
    buffer[0] = c;
    buffer[1] = '\0';
    print(buffer);
}

void print_number(int number)
{
    char buffer[21];
    int i = 0;
    do
    {
        buffer[i] = (number % 10) + '0';
        number /= 10;
        i++;
    }
    while (number != 0);
    buffer[i] = '\0';

    // reverse the buffer
    for (int k = 0; k < i / 2; k++)
    {
        char temp = buffer[k];
        buffer[k] = buffer[i - k - 1];
        buffer[i - k - 1] = temp;
    }

    print(buffer);
}

void* allocate(int size)
{
    return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
}
