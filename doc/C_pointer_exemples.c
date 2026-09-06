// Summary for pointers to functions

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! Warning: this file is only a small summary about passing pointers to !
 * ! functions, not fully working code: many memory leaks occur.          !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Aliases for pointers:
 * 	(*foo).member <-> foo->member
 * 	(array+i) <-> array[i]
 * 	&array[0] <-> array
 * 	fx() <-> (*fx)()    // equivalent
 * 	int *bar[x] -> int **bar
 */

/* Increment tricky cases
 *	*p++   // same as *(p++): increment pointer, and dereference unincremented address
 * 	*++p   // same as *(++p): increment pointer, and dereference incremented address
 * 	++*p   // same as ++(*p): dereference pointer, and increment the value it points to
 *	(*p)++ // dereference pointer, and post-increment the value it points to
*/

#include <stdio.h>
#include <stdlib.h>

void foo(int b);
void bar(int *b);
void bar2(int *b);
void baz(int **b);

int main(void)
{
	int a = 10;
	printf("level 1 copy - no change\n[%p] = %i\n",(void*)&a, a);
	foo(a);
	printf("[%p] = %i\n\n",(void*)&a, a);

	int b = 10;
	printf("level 2 pointer - change\n[%p] = %i\n",(void*)&b, b);
	bar(&b);
	printf("[%p] = %i\n\n",(void*)&b, b);

	int *c;
	c = malloc(sizeof(*c));
	*c = 10;
	printf("level 2 pointer - change\n[%p] = %i\n",(void*)c, *c);
	bar(c);
	printf("[%p] = %i\n\n",(void*)c, *c);

	int *c2;
	c2 = malloc(sizeof(*c2));
	*c2 = 10;
	printf("level 2 pointer - no change\n[%p] = %i\n",(void*)c2, *c2);
	bar2(c2);
	printf("[%p] = %i\n\n",(void*)c2, *c2);

	int *d;
	d = malloc(sizeof(*d));
	*d = 10;
	printf("level 3 pointer to pointer - change\n[%p] = %i\n",(void*)d, *d);
	baz(&d);
	printf("[%p] = %i\n\n",(void*)d, *d);

	int **e;
	e = malloc(sizeof(*e));
	*e = malloc(sizeof(**e));
	**e = 10;
	printf("level 3 pointer to pointer - change\n[%p] = %i\n",(void*)*e, **e);
	baz(e);
	printf("[%p] = %i\n\n",(void*)*e, **e);

return 0;
}

void foo(int b)
{
	b = 20;
}

void bar(int *b)
{
	*b = 20;
}

void bar2(int *b)
{
	b = malloc(sizeof(*b));
	*b = 20;
}
void baz(int **b)
{
	*b = malloc(sizeof(**b));
	**b = 20;
}
