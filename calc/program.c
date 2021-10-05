/*
 * program.c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "program.h"


static struct program programs[1];
static int max = sizeof(programs)/sizeof(struct program);


void Example0_setup(struct program *);

void
prg_setup(void)
{
  struct program *prg = &programs[0];
  Example0_setup(prg);
}


void
prg_list(void)
{
  struct program *prg;
  int i;

  for (i = 0; i < max; i++) {
    prg = &programs[i];
    if (prg->message == NULL) continue;
    printf("%d: %s\n", i, prg->message[0]);
  }
}

struct program *
prg_lookup(int no)
{
  if (no >= max) return(NULL);
  return(&programs[no]);
}
