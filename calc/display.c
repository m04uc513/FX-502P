/*
 * display.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "display.h"

void
dis_reset(struct display *dp)
{
  int i;
  bzero(dp->buf, 32);
  for (i = 0; i < 10; i++) dp->buf[i] = ' ';
  dp->len = 0;
}

int
dis_add(struct display *dp, char c)
{
  if (dp->len >= 10) {
    return(-1);
  }
  
  dp->buf[dp->len+10] = c;
  return(++dp->len);
}

char *
dis_str(struct display *dp)
{
  return(&dp->buf[dp->len]);
}

int
dis_integer(struct display *dp)
{
  return(atoi(&dp->buf[dp->len]));
}

float
dis_float(struct display *dp)
{
  return(atof(&dp->buf[dp->len]));
}

void
dis_set_string(struct display *dp, char *s)
{
  strcpy(&dp->buf[10], s);
  dp->len = strlen(dp->buf) - 10;
}

void
dis_set_integer(struct display *dp, int i)
{
  snprintf(&dp->buf[10], 10, "%d", i);
  dp->len = strlen(dp->buf) - 10;
}

void
dis_set_long(struct display *dp, long i)
{
  snprintf(&dp->buf[10], 10, "%ld", i);
  dp->len = strlen(dp->buf) - 10;
}

void
dis_set_float(struct display *dp, float f)
{
  snprintf(&dp->buf[10], 10, "%f", f);
  dp->len = strlen(dp->buf) - 10;
}
#if 0
int
main(int argc, char *argv[])
{
  struct display x;
  
  dis_reset(&x);
  dis_add(&x, '1');
  dis_add(&x, '2');
  dis_add(&x, '8');
  printf("str: %s\n", dis_str(&x));
  printf("int: %d\n", dis_integer(&x));

  dis_reset(&x);
  dis_add(&x, '3');
  dis_add(&x, '.');
  dis_add(&x, '1');
  dis_add(&x, '4');
  printf("str: %s\n", dis_str(&x));
  printf("float: %f\n", dis_float(&x));

  dis_set_integer(&x, 12);
  printf("%3d: %s\n", x.len, dis_str(&x));

  dis_set_float(&x, 2.7);
  printf("%3d: %s\n", x.len, dis_str(&x));
  
  exit(0);
}
#endif
