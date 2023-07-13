typedef struct
{
    char* source;
    int index;
    int line;
} Parser;

Parser make_parser(char* source)
{
    Parser result;
    result.source = source;
    result.index = 0;
    result.line = 1;
    return result;
}

void next_plus(int count, Parser* parser)
{
    for (int i = 0; i < count; i++)
    {
        if (parser->source[parser->index + i] == '\n')
        {
            parser->line++;
        }
        else if (parser->source[parser->index + i] == '\r' && parser->source[parser->index + i + 1] == '\n')
        {
            parser->line++;
            i++;
            if (i == count)
            {
                print("Stepped over newline\n");
                ExitProcess(1);
            }
        }
    }
    parser->index += count;
}

void next(Parser* parser) { next_plus(1, parser); }

char current_plus(int i, Parser parser) { return parser.source[parser.index + i]; }

char current(Parser parser) { return current_plus(0, parser); }

bool expect(char* prefix, Parser* parser)
{
    int i = 0;
    while (prefix[i] != '\0')
    {
        if (prefix[i] != current_plus(i, *parser)) { return false; }
        i++;
    }
    next_plus(i, parser);
    return true;
}

bool expect_newline(Parser* parser)
{
    if (current(*parser) == '\n')
    {
        next(parser);
        return true;
    }
    if (current(*parser) == '\r' && current_plus(1, *parser) == '\n')
    {
        next_plus(2, parser);
        return true;
    }
    return false;
}

bool expect_eof(Parser parser) { return parser.source[parser.index] == '\0'; }

Input* parse_input(char* source)
{
    Input* result = allocate(sizeof(Input));

    Parser parser = make_parser(source);
    while (!expect_eof(parser))
    {
        if (expect("field", &parser))
        {
            if (!expect_newline(&parser))
            {
                print("Expected just 'field' on line ");
                print_number(parser.line);
                print(", but got something else\n");
                ExitProcess(1);
            }

            int field_width = 0;
            int x = 0;
            int y = 0;
            do
            {
                if (y != 0 && y >= field_width)
                {
                    print("Line ");
                    print_number(parser.line);
                    print(" is larger than the rest of the field\n");
                    ExitProcess(1);
                }

                if (current(parser) == '.') { }
                else if (current(parser) == 'X')
                {
                    add_field_x(x, y, result);
                }
                else
                {
                    print("Unrecognized character in field on line ");
                    print_number(parser.line);
                    print("\n");
                    ExitProcess(1);
                }
                x++;
                next(&parser);

                if (expect_newline(&parser) || expect_eof(parser))
                {
                    if (y == 0) { field_width = x; }
                    x = 0;
                    y++;
                }
            }
            // two newlines in a row or end of the file means we're done with the block
            while (!expect_newline(&parser) && !expect_eof(parser));
            result->field_width = field_width;
            result->field_height = y;
        }
        else if (expect_newline(&parser)) { } // skip blank lines
        // comment (comments can only start at the beginning of the line and take up the whole line)
        else if (expect("#", &parser))
        {
            while (!expect_newline(&parser) || expect_eof(parser)) { next(&parser); }
        }
        else // shape
        {
            char shape_name = current(parser);
            next(&parser);
            if (!expect_newline(&parser))
            {
                print("Expected newline after one-character shape name on line ");
                print_number(parser.line);
                print("\n");
                ExitProcess(1);
            }
            if (shape_name == 'X' || shape_name == ' ' || shape_name == '\t' || shape_name == '.')
            {
                print("Shape name '");
                print_char(shape_name);
                print("' is invalid (line ");
                print_number(parser.line);
                print(")\n");
                ExitProcess(1);
            }
            for (int i = 0; i < result->shapes_count; i++)
            {
                if (result->shapes[i].name == shape_name)
                {
                    print("Duplicate shape name '");
                    print_char(shape_name);
                    print("' on line ");
                    print_number(parser.line);
                    print("\n");
                    ExitProcess(1);
                }
            }

            bool x_found = false;
            int x = 0;
            int y = 0;
            Shape shape;
            set_memory(sizeof(shape), &shape, 0);
            shape.name = shape_name;
            do
            {
                if (current(parser) == ' ') { }
                else if (current(parser) == '.')
                {
                    add_point_to_shape(x, y, PointInSpaceShapeBody, &shape);
                }
                else if (current(parser) == 'X')
                {
                    if (x_found)
                    {
                        print("Found a duplicate X mark on shape ");
                        print_char(shape_name);
                        print("\n");
                        ExitProcess(1);
                    }
                    add_point_to_shape(x, y, PointInSpaceX, &shape);
                    x_found = true;
                }
                else
                {
                    print("Found an unexpected character on line ");
                    print_number(parser.line);
                    print(" while parsing shape ");
                    print_char(shape.name);
                    ExitProcess(1);
                }
                x++;
                next(&parser);

                if (expect_newline(&parser) || expect_eof(parser))
                {
                    x = 0;
                    y++;
                }
            }
            while (!expect_newline(&parser) && !expect_eof(parser));

            if (!x_found)
            {
                print("Did not find X on shape ");
                print_char(shape_name);
                print("\n");
                ExitProcess(1);
            }

            shape.is_completely_symmetrical = is_shape_completely_symmetrical(shape);
            shape.is_vertically_symmetrical = is_shape_vertically_symmetrical(shape);
            add_shape_to_input(shape, result);
        }
    }

    if (result->shapes_count != result->field_xs_count)
    {
        print("Expected an equal amount of shapes an Xs on the field, instead got ");
        print_number(result->field_xs_count); print(" Xs and ");
        print_number(result->shapes_count); print(" shapes\n");
        ExitProcess(1);
    }

    return result;
}

char* read_input_file(char* filename)
{
    HANDLE input_file = CreateFile(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (input_file == INVALID_HANDLE_VALUE)
    {
        print("Failed to open \""); print(filename); print("\"\n");
        ExitProcess(1);
    }

    char* buffer = allocate(MAX_INPUT_SIZE + 1);
    DWORD bytes_read;
    ReadFile(input_file, buffer, MAX_INPUT_SIZE + 1, &bytes_read, NULL);
    CloseHandle(input_file);
    if (bytes_read == MAX_INPUT_SIZE + 1)
    {
        print("Input file can not be larger than ");
        print_number(MAX_INPUT_SIZE);
        print(" bytes\n");
        ExitProcess(1);
    }
    buffer[bytes_read] = '\0';

    return buffer;
}
