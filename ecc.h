#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <stdint.h>

extern int debug;

typedef struct {
	void **data;
	int capacity;
	int len;
} Vector;

Vector *new_vec(void);
void vec_push(Vector *v, void *elem);

typedef struct {
	Vector *keys;
	Vector *vals;
} Map;

// util_test.c

void util_test();

//token.c

enum {
	TK_NUM = 256,
	TK_IDENT,
	TK_IF,
	TK_RETURN,
	TK_EOF,
};

//Token type
typedef struct {
	int ty;
	int val;
	char *name;
	char *input;
} Token;

Vector *tokenize(char *p);

//parse.c

enum {
	ND_NUM = 256,
	ND_IDENT,
	ND_RETURN,
	ND_COMP_STMT,
	ND_EXPR_STMT,
	ND_IF,
};

typedef struct Node {
	int ty;
	struct Node *lhs;
	struct Node *rhs;
	int val;
	char *name;
	struct Node *expr;
	Vector *stmts;

	// if
	struct Node *cond;
	struct Node *then;
} Node;

Node *parse(Vector *tokens);

//ir.c

enum {
	IR_IMM,
	IR_MOV,
	IR_RETURN,
	IR_ALLOCA,
	IR_LOAD,
	IR_STORE,
	IR_KILL,
	IR_NOP,
	IR_UNLESS,
	IR_LABEL,
};

typedef struct {
	int op;
	int lhs;
	int rhs;
} IR;

typedef struct {
  char *name;
  int ty;
} IRInfo;

Vector *gen_ir(Node *node);

//regalloc.c

extern char *regs[];
void alloc_regs(Vector *irv);

//codegen.c
void gen_riscv(Vector *irv);

//util.c
noreturn void error(char *fmt, ...);
char *format(char *fmt, ...);
extern int debug;
Map *new_map(void);
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
bool map_exists(Map *map, char *key);
void show_token(Vector *v);
void show_descendantTree(Node *node);

