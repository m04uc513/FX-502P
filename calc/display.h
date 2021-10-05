/*
 * display.h
 */


struct display {
  int  len;
  char buf[32];
};

void  dis_reset(struct display *dp);
int   dis_add(struct display *dp, char c);
char *dis_str(struct display *dp);
int   dis_integer(struct display *dp);
float dis_float(struct display *dp);
void  dis_set_string(struct display *dp, char *s);
void  dis_set_integer(struct display *dp, int i);
void  dis_set_long(struct display *dp, long i);
void  dis_set_float(struct display *dp, float f);
