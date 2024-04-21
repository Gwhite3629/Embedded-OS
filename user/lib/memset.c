void *memset(void *dest, char val, size_t n)
{
    uint32_t i;
	char *b;

	b=(char *)dest;

	for(i=0;i<n;i++) b[i]=val;

	return dest;
}