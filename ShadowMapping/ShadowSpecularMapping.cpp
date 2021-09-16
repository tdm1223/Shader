template <typename T>
T Max(T a, T b)
{
	return a > b ? a : b;
}

template <>
char* Max<char*>(char *a, char* b)
{
	return strlen(a) > strlen(b) ? a : b;
}