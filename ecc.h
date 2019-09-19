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
	TK_INT,
	TK_IF,
	TK_BLOCK_END,
	TK_ELSE,
	TK_RETURN,
	TK_FUNC,
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
	ND_ELSE,
	ND_IF_BLOCK,
	ND_ELSE_BLOCK,
	ND_FUNC,
	ND_FUNC_DEF,
	ND_EQ,
	ND_NE,
};

typedef struct Node {
	int ty;
	struct Node *lhs;
	struct Node *rhs;
	int val;
	char *name;
	struct Node *expr;
	Vector *stmts;
	Vector *stmts_then;

	// if
	struct Node *cond;
	struct Node *then;

	int arg[6];
	int num_arg;
} Node;

Node *parse(Vector *tokens);

//ir.c

enum {
	IR_IMM,
	IR_MOV,
	IR_RETURN,
	IR_ALLOCA,
	IR_DEALLOCA,
	IR_LOAD,
	IR_STORE,
	IR_KILL,
	IR_NOP,
	IR_UNLESS,
	IR_LABEL,
	IR_BLOCK_END,
	IR_ELSE,
	IR_FUNC,
	IR_FUNC_DEF,
	IR_PUSH,
	IR_POP,
	IR_EQ,
	IR_NE,
};

typedef struct {
	int op;
	int lhs;
	int rhs;
	int sp;
	char *name;
} IR;

typedef struct {
  char *name;
  int ty;
} IRInfo;

Vector *gen_ir(Node *node);

//regalloc.c

extern char *regs[];
extern char *func_regs[];
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
void print_horizon(const char *str);

#define TEST \
	printf("%s: %d\n", __func__, __LINE__)

