#include "ecc.h"

noreturn void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

char *format(char *fmt, ...) {
	char buf[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	return strdup(buf);
}

Vector *new_vec(void)
{
	Vector *v = malloc(sizeof(Vector));
	v->data = malloc(sizeof(void *) * 16);
	v->capacity = 16;
	v->len = 0;

	return v;
}

void vec_push(Vector *v, void *elem)
{
	if (v->len == v->capacity) {
		v->capacity *= 2;
		v->data = realloc(v->data, sizeof(Vector) * v->capacity);
	}
	v->data[v->len++] = elem;
}

Map *new_map(void)
{
	Map *map = malloc(sizeof(Map));
	map->keys = new_vec();
	map->vals = new_vec();
	return map;
}

void map_put(Map *map, char *key, void *val)
{
	vec_push(map->keys, key);
	vec_push(map->vals, val);
}

void *map_get(Map *map, char *key)
{
	for (int i = map->keys->len - 1; i >= 0; i--) {
		if (!strcmp(map->keys->data[i], key))
			return map->vals->data[i];
	}
	return NULL;
}

bool map_exists(Map *map, char *key)
{
	for (int i = 0; i < map->keys->len; i++)
		if (!strcmp(map->keys->data[i], key))
			return true;
	return false;
}

int debug = 0;

static void print_horizon(const char *str)
{
	printf("----- %s -----\n", str);
}

void show_token(Vector *v)
{
	if (!debug)
		return;

	print_horizon("TOKENIZE");

	for (int i = 0; i < v->len; i++) {
		Token *t = v->data[i];
		if (t->ty == TK_NUM)
			printf("TK_NUM:    %d\n", t->val);
		else if (t->ty == TK_RETURN)
			printf("TK_RETURN: %s(%d)\n", t->name, t->ty);
		else if (t->ty == TK_IDENT)
			printf("TK_IDENT:  %s(%d)\n", t->name, t->ty);
		else if (t->ty == TK_IF)
			printf("TK_IF:     %s(%d)\n", t->name, t->ty);
		else if (t->ty == TK_ELSE)
			printf("TK_ELSE:   %s(%d)\n", t->name, t->ty);
		else
			printf("ELSE:      %c(%d)\n", (char)t->ty, t->ty);
	}
	printf("\n");
}

static void show_lval(Node *node)
{
	printf("%s ", node->name);
}

static void show_expr(Node *node)
{
	if (node->ty == ND_NUM) {
		printf("%d ", node->val);
		return;
	} else if (node->ty == ND_IDENT) {
		printf("%s ", node->name);
		return;
	} else if (node->ty == '=') {
		printf("= ");
		show_expr(node->rhs);
		show_lval(node->lhs);
		return;
	} else if (strchr("+-*/", node->ty))
		printf("%c ", node->ty);
	else
		printf("(root) ");

	show_expr(node->lhs);
	show_expr(node->rhs);
}

static void do_show_descendantTree(Node *node)
{
	if (node->ty == ND_RETURN) {
		show_expr(node->expr);
		printf("return ");
		printf("; ");
		return;
	}

	if (node->ty == ND_IF) {
		show_expr(node->cond);
		do_show_descendantTree(node->then);
		return;
	}

	if (node->ty == ND_ELSE) {
		do_show_descendantTree(node->then);
		return;
	}

	if (node->ty == ND_IF_BLOCK) {
		show_expr(node->cond);
		for (int i = 0; i < node->stmts->len; i++)
			do_show_descendantTree(node->stmts->data[i]);
		return;
	}

	if (node->ty == ND_EXPR_STMT) {
		show_expr(node->expr);
		printf("; ");
		return;
	}

	if (node->ty == ND_COMP_STMT) {
		for (int i = 0; i < node->stmts->len; i++)
			do_show_descendantTree(node->stmts->data[i]);
		return;
	}
}

void show_descendantTree(Node *node)
{
	if (!debug)
		return;

	print_horizon("PARSE");
	do_show_descendantTree(node);
	printf("\n\n");
}
