/*
 * program.h
 */

struct program {
  unsigned char *code;
  int		 length;
  int		*routine;
  int	       **label;
  char	       **message;
};

void prg_setup(void);
void prg_list(void);
struct program *prg_lookup(int no);

