#include <stdio.h>

extern void __lance_start(void) asm("__lance_start");
extern int __axe_read(void) asm("__axe_read");
extern void __axe_write(int n) asm("__axe_write");

int main(int argc, char *argv[])
{
	__lance_start();
}

int __axe_read(void)
{
	int tmp;
	printf("int value? >");
	scanf("%d", &tmp);
	return tmp;
}

void __axe_write(int n)
{
	printf("%d\n", n);
}
