/*
    gcc nonamestill.c -o nonamestill  -Wl,-s -m32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct URLList {
  struct URLList *next;
  char url[];
};

struct URLList *g_head;

char *cgiDecodeString (char *text) {
  char *cp, *xp;

  for (cp=text,xp=text; *cp; cp++) {
    if (*cp == '%') {
      if (strchr("0123456789ABCDEFabcdef", *(cp+1))
          && strchr("0123456789ABCDEFabcdef", *(cp+2))) {
        if (islower(*(cp+1)))
          *(cp+1) = toupper(*(cp+1));
        if (islower(*(cp+2)))
          *(cp+2) = toupper(*(cp+2));
        *(xp) = (*(cp+1) >= 'A' ? *(cp+1) - 'A' + 10 : *(cp+1) - '0' ) * 16
          + (*(cp+2) >= 'A' ? *(cp+2) - 'A' + 10 : *(cp+2) - '0');
        xp++;cp+=2;
      }
    } else {
      *(xp++) = *cp;
    }
  }

  memset(xp, 0, cp-xp);
  return text;
}

void print_banner(void) {
  puts("   __ __  ____   _          ___      ___    __   ___   ___      ___  ____  ");
  puts("   |  |  ||    \\ | |        |   \\    /  _]  /  ] /   \\ |   \\    /  _]|    \\ ");
  puts("   |  |  ||  D  )| |        |    \\  /  [_  /  / |     ||    \\  /  [_ |  D  )");
  puts("   |  |  ||    / | |___     |  D  ||    _]/  /  |  O  ||  D  ||    _]|    / ");
  puts("   |  :  ||    \\ |     |    |     ||   [_/   \\_ |     ||     ||   [_ |    \\ ");
  puts("   |     ||  .  \\|     |    |     ||     \\     ||     ||     ||     ||  .  \\");
  puts("    \\__,_||__|\\_||_____|    |_____||_____|\\____| \\___/ |_____||_____||__|\\_|");
  puts("                                                                             ");
}

void print_menu(void) {
  puts("===== Menu =====");
  puts("1: create a url");
  puts("2: decode a url");
  puts("3: list urls");
  puts("4: delete a url");
  puts("5: exit");
  puts("================\n");
}

int get_integer(void) {
  int ret = 0;
  char buf[16];
  fgets(buf, sizeof(buf), stdin);
  sscanf(buf, "%d", &ret);
  return ret;
}

void create_url(void) {
  unsigned int size;
  struct URLList *node;

  printf("size: ");
  size = get_integer();
  if (size+sizeof(struct URLList) < size) {
    puts("Error: invalid size");
    exit(0);
  }

  node = malloc(sizeof(struct URLList) + size);
  if (!node) {
    puts("Error: malloc failed");
    exit(0);
  }

  printf("URL: ");
  fgets(node->url, size, stdin);
  node->next = g_head;
  g_head = node;
}

void decode_url(void) {
  int idx;

  printf("index: ");
  idx = get_integer();

  struct URLList *node = g_head;
  while (idx--) {
    if (node == NULL) break;
    node = node->next;
  }

  if (node == NULL) {
    puts("Error: no such url");
    exit(0);
  }

  cgiDecodeString(node->url);
}

void list_urls(void) {
  int idx = 0;
  struct URLList *node = g_head;

  puts("LIST START");
  while (node) {
    printf("%d: %s\n", idx, node->url);
    node = node->next;
    idx++;
  }
  puts("LIST END");
}

void delete_url(void) {
  int idx;

  printf("index: ");
  idx = get_integer();

  struct URLList *prev = NULL;
  struct URLList *node = g_head;
  while (idx--) {
    if (node == NULL) break;
    prev = node;
    node = node->next;
  }

  if (node == NULL) {
    puts("Error: no such url");
    exit(0);
  }
  
  if (prev == NULL) {
    g_head = node->next;
  } else {
    prev->next = node->next;
  }

  free(node);
}

int main() {
  setbuf(stdout, NULL);

  print_banner();

  while (1) {
    int cmd;

    print_menu();
    printf("> ");
    cmd = get_integer();

    if (cmd == 1) {
      create_url();
    } else if (cmd == 2) {
      decode_url();
    } else if (cmd == 3) {
      list_urls();
    } else if (cmd == 4) {
      delete_url();
    } else if (cmd == 5) {
      exit(0);
    } else {
      puts("Error: undefined command");
    }
  }
}
