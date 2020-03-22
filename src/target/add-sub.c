_Static_assert(sizeof(int) == 4);

int main()
{
    int x = 0;
    x += 1;
    x += 2;
    x += 3;
    x += 4;
    x += 5;
    x += 6;
    x += 7;
    x += 8;
    x += 9;
    *((int*)0x1234567) = x;
    return 0;
}

void _start()
{
    main();
    while (1)
    {
    }
}
