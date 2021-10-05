/*
 * calc -- simple calcurator with program
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fx502p.h"

int
main(int argc, char *argv[])
{
  struct program *prg;
  int no, ret;
  
  prg_setup();
  if (argc == 1) {
    prg_list();
    exit(0);
  }

  fx502p_setup();

  no = atoi(argv[1]);
  prg = prg_lookup(no);
  if (prg == NULL) {
    fprintf(stderr, "error: program no %d is not defined\n", no);
    exit(1);
  }
  
  fx502p_program_setup(prg);
  ret = fx502p_exec();

  exit(0);
}

