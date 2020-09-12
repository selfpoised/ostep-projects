// sort.system.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define get_inter_file(i) \
  ({ char *p = malloc(1024); bzero(p, 1024); sprintf(p, "%s%d", "ttt.", i); p; })

#define wait_or_die(status) \
  ({ pid_t pid = wait(status); assert(pid >= 0); pid; })

int main(int argc, char **argv)
{
  if (argc != 2)
  { // expecting one argument, file which we will sort
    fprintf(stderr, "Usage: sort [FILE]\n");
    return EXIT_FAILURE;
  }
  char line1[1024];
  sprintf(line1, "one two three");
  char *token, *dummy = line1;
  token = strsep(&dummy, " \t\n\r");
  printf("%s ->%s\n", token, dummy);
  token = strsep(&dummy, " \t\n\r");
  printf("%s ->%s\n", token, dummy);


  FILE *fp = fopen(argv[1], "r");
  unsigned int curpos = ftell(fp);
  char *line = NULL;
  size_t size = 0;
  getline(&line, &size, fp);
  curpos = ftell(fp);
  printf("%d: %s\n", curpos, line);
  getline(&line, &size, fp);
  // curpos = ftell(fp);
  printf("%d: %s\n", curpos, line);

  fseek(fp, 4, SEEK_SET);
  getline(&line, &size, fp);
  printf("%d: %s\n", curpos, line);

  int *p_partion_getter_pos = (int *)malloc(6 * sizeof(int *));
  for (int t = 0; t <= 5; t++)
  {
    p_partion_getter_pos[t] = t;
  }
  for (int t = 0; t <= 5; t++)
  {
    printf("%d\t", p_partion_getter_pos[t]);
  }
  printf("\n");

  char *p = get_inter_file(2);
  printf("%s\n", p);
  free(p);

  int length = 1024; // length of "sort " is 5.
  char *command = (char *)calloc(length, sizeof(char));

  sprintf(command, "sort %s > %s.s", argv[1], argv[1]);
  int ret = system(command);

  bzero(command, 1024);
  sprintf(command, "rm %s", argv[1]);
  ret = system(command);

  bzero(command, 1024);
  sprintf(command, "mv %s.s %s", argv[1], argv[1]);
  ret = system(command);

  free(command);

  return ret;
}

// gcc -o my_sort sort.c -Wall -Werror