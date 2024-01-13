void touch2(unsigned val)
{
    vlevel = 2; /* Part of validation protocol */
    if (val == cookie)
    {
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    }
    else
    {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}