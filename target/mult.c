typedef long long int64_t;
_Static_assert(sizeof(int64_t) == 8);

long double multld()
{
    long double a = 4;
    long double b = 3;
    return a * b;
}

double multd()
{
    double a = 4;
    double b = 3;
    return a * b;
}

float multf()
{
    float a = 4;
    float b = 3;
    return a * b;
}

int mult()
{
    int a = 4;
    int b = 3;
    return a * b;
}

int64_t mult64()
{
    int64_t a = 4;
    int64_t b = 3;
    return a * b;
}

int main()
{
    int * out = (int*)(0x654321);
    int64_t * out64 = (int64_t*)(0x654321);
    float * outf = (float*)(0x653321);
    double * outd = (double*)(0x655321);
    long double * outld = (long double*)(0x656321);
    *out = mult();
    *out64 = mult64();
    *outf = multf();
    *outd = multd();
    *outld = multld();
    return 0;
}

void _start()
{
    main();
    while (1)
    {
    }
}
