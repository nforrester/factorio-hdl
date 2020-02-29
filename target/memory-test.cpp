int main()
{
    unsigned int const sz = 1024;
    char buffer[sz];
    for (unsigned int i = 0; i < sz; ++i)
    {
        buffer[i] = 'a' + (i % 26);
    }

    char a = buffer[260];
    char b = buffer[270];
    char c = buffer[280];
    char d = buffer[290];
    char e = buffer[300];
    char f = buffer[310];

    int ae = a == buffer[0];
    int be = b == buffer[1];
    int ce = c == buffer[2];
    int de = d == buffer[3];
    int ee = e == buffer[4];
    int fe = f == buffer[5];

    return (ae && be && ce && de && ee && fe) ? 0 : 1;
}

extern "C"
void _start()
{
    main();
    while (1)
    {
    }
}
