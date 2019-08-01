#include "ecc.h"

static Vector *code;
static int regno;
static int basereg;
static Map *vars;
static int bpoff;

static int label;

IRInfo irinfo[] = {
	[IR_IMM] = {"IMM", IR_IMM},
	[IR_MOV] = {"MOV", IR_KILL},
	[IR_RETURN] = {"RET", IR_RETURN},
	[IR_ALLOCA] = {"ALLOC", IR_ALLOCA},
	[IR_LOAD] = {"LOAD", IR_LOAD},
	[IR_STORE] = {"STORE", IR_STORE},
	[IR_KILL] = {"KILL", IR_KILL},
	[IR_NOP] = {"NOP", IR_NOP},
	[IR_UNLESS] = {"UNLESS", IR_UNLESS},
	[IR_LABEL] = {"LABEL", IR_LABEL},
};

static char *tostr(IR *ir)
{
	IRInfo info = irinfo[ir->op];
	static int sp = 0;

	switch (info.ty) {
	case IR_IMM:
		return format("%7s%3d[reg]%3d[val] ", info.name, ir->lhs, ir->rhs);
	case IR_RETURN:
		return format("%7s%3d    ", info.name, ir->lhs);
	case IR_ALLOCA:
		return format("%7s%3d%3d ", info.name, ir->lhs, ir->rhs);
	case IR_LOAD:
		return format("%7s%3d[reg]%3d[src] ", info.name, ir->lhs, --sp);
	case IR_STORE:
		return format("%7s%3d[reg]%3d[dest] ", info.name, ir->rhs, sp++);
	case IR_NOP:
		return format("%7s       ", info.name);
	case IR_LABEL:
		return format("%7s L.%d  ", info.name, ir->lhs);
	case IR_KILL:
		return format("%7s%3d    ", info.name, ir->lhs);
	case IR_UNLESS:
		return format("%7s%3d L.%d ", info.name, ir->lhs, ir->rhs);
	default:
		return format("unimplemented");
	}
}

void dump_ir(Vector *irv)
{
	if (!debug)
		return;

	for (int i = 0; i < irv->len; i++)
		printf("%s\n", tostr(irv->data[i]));
	printf("\n");
}

static IR *add(int op, int lhs, int rhs)
{
	IR *ir = malloc(sizeof(IR));
	ir->op = op;
	ir->lhs = lhs;
	ir->rhs = rhs;
	vec_push(code, ir);
	return ir;
}

static int gen_lval(Node *node)
{
	if (node->ty != ND_IDENT)
		error("not an lvalue");

	if (!map_exists(vars, node->name)) {
		map_put(vars, node->name, (void *)(intptr_t)bpoff);
		bpoff += 8;
	}

	return ++regno;
}

static int gen_expr(Node *node)
{
	if (node->ty == ND_NUM) {
		int r = ++regno;
		add(IR_IMM, r, node->val);
		return r;
	}

	if (node->ty == ND_IDENT) {
		int r = gen_lval(node);
		add(IR_LOAD, r, r);
		return r;
	}
	if (node->ty == '=') {
		int rhs = gen_expr(node->rhs);
		int lhs = gen_lval(node->lhs);
		add(IR_STORE, lhs, rhs);
		add(IR_KILL, rhs, -1);
		return lhs;
	}

	assert(strchr("+-*/", node->ty));

	int lhs = gen_expr(node->lhs);
	int rhs = gen_expr(node->rhs);

	add(node->ty, lhs, rhs);
	add(IR_KILL, rhs, -1);
	return lhs;
}

static void gen_stmt(Node *node)
{
	if (node->ty == ND_RETURN) {
		int r = gen_expr(node->expr);
		add(IR_RETURN, r, -1);
		add(IR_KILL, r, -1);
		return;
	}

	if (node->ty == ND_COND) {
		int r = gen_expr(node->cond);
		int x = ++label;
		add(IR_UNLESS, r, x);
		add(IR_KILL, r, -1);
		gen_stmt(node->then);
		add(IR_LABEL, x, -1);
		return;
	}

	if (node->ty == ND_EXPR_STMT) {
		int r = gen_expr(node->expr);
		add(IR_KILL, r, -1);
		return;
	}

	if (node->ty == ND_COMP_STMT) {
		for (int i = 0; i < node->stmts->len; i++)
			gen_stmt(node->stmts->data[i]);
		return;
	}

	error("unknown node: %d", node->ty);
}

Vector *gen_ir(Node *node)
{
	assert(node->ty == ND_COMP_STMT);

	code = new_vec();
	regno = 1;
	basereg = 0;
	vars = new_map();
	bpoff = 0;
	label = 0;

	IR *alloca = add(IR_ALLOCA, basereg, -1);
	gen_stmt(node);
	alloca->rhs = bpoff;
	add(IR_KILL, basereg, -1);
	dump_ir(code);
	return code;
}
