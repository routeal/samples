#include "test.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *gstr = "this is a c str";

const char *cgstr = "this is a const c str";

void printswift(const char *s)
{
  printf("printswift: %s\n", s);
}

extern void printstruct(const CStruct *t)
{
  printf("from c: %d %s\n", t->i, t->str);
}

int CUseCallback(callback01 c, int n)
{
  APIStruct a;
  a.m_Int = 128;
  a.m_Long = 12;
  a.m_Array[0] = 100;
  c(&a);
  printf("CUseCallback¥n");
  return 0;
}

void CallStrCallback(callback02 c)
{
  c("CallStrCallback", strlen("CallStrCallback"));
}


void test_func_callback ( void *data )
{
  Data *d = (Data *) data;
  printf("%d¥n", d->i);
}


void CallCFunc(const cfunc_struct *c)
{
  if (c) {
    Data d;
    d.i = 1111;
    c->func1("tako", 4);
    c->func2("ika", 3);
    c->func3(test_func_callback, &d);
  }
}
