#include "ecc.h"

static Vector *code;
static int regno;
static int basereg;
static Map *vars;
static int bpoff;

static int label;
static int stkp;

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
	[IR_BLOCK_END] = {"BLK_END", IR_BLOCK_END},
	[IR_ELSE] = {"ELSE", IR_ELSE},
	[IR_FUNC] = {"CALL", IR_FUNC},
	['+'] = {"ADD", '+'},
	['-'] = {"SUB", '-'},
};

static char *tostr(IR *ir)
{
	IRInfo info = irinfo[ir->op];

	switch (info.ty) {
	case IR_IMM:
		return format("%7s%3d[reg]%3d[val] ", info.name, ir->lhs, ir->rhs);
	case IR_RETURN:
		return format("%7s%3d    ", info.name, ir->lhs);
	case IR_ALLOCA:
		return format("%7s%3d%3d ", info.name, ir->lhs, ir->rhs);
	case IR_LOAD:
		return format("%7s%3d[reg]%3d[src] ", info.name, ir->lhs, ir->sp);
	case IR_STORE:
		return format("%7s%3d[reg]%3d[dest] ", info.name, ir->rhs, ir->sp);
	case IR_NOP:
		return format("%7s       ", info.name);
	case IR_KILL:
		return format("%7s%3d    ", info.name, ir->lhs);
	case IR_UNLESS:
		return format("%7s%3d L.%d ", info.name, ir->lhs, ir->rhs);
	case IR_LABEL:
		return format("%7s  L.%d  ", info.name, ir->lhs);
	case IR_BLOCK_END:
		return format("%7s  L.%d  ", info.name, ir->lhs);
	case IR_FUNC:
		return format("%7s  %s  ", info.name, ir->name);
	case '+':
		return format("%7s%3d[reg]%3d[reg] ", info.name, ir->lhs, ir->rhs);
	default:
		return format("unimplemented: %d\n", info.ty);
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

static IR *add(int op, int lhs, int rhs, int sp)
{
	IR *ir = malloc(sizeof(IR));
	ir->op = op;
	ir->lhs = lhs;
	ir->rhs = rhs;
	ir->sp = sp;
	vec_push(code, ir);
	return ir;
}

static IR *call(int op, int lhs, char *name)
{
	IR *ir = malloc(sizeof(IR));
	ir->op = op;
	ir->lhs = lhs;
	ir->name = name;
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

static int gen_expr(Node *node, int *sp)
{
	if (node->ty == ND_NUM) {
		int r = ++regno;
		add(IR_IMM, r, node->val, *sp);
		return r;
	}

	if (node->ty == ND_IDENT) {
		int r = gen_lval(node);
		*sp -= 8;
		add(IR_LOAD, r, r, *sp);
		return r;
	}
	if (node->ty == '=') {
		int rhs = gen_expr(node->rhs, sp);
		int lhs = gen_lval(node->lhs);
		add(IR_STORE, lhs, rhs, *sp);
		*sp += 8;
		add(IR_KILL, rhs, -1, -1);
		return lhs;
	}

	if (node->ty == ND_FUNC) {
		int r = ++regno;
		call(IR_FUNC, r, node->name);
		return r;
	}

	assert(strchr("+-*/", node->ty));

	int lhs = gen_expr(node->lhs, sp);
	int rhs = gen_expr(node->rhs, sp);

	add(node->ty, lhs, rhs, -1);
	add(IR_KILL, rhs, -1, -1);
	return lhs;
}

static void gen_stmt(Node *node, int *sp)
{
	if (node->ty == ND_RETURN) {
		int r = gen_expr(node->expr, sp);
		add(IR_RETURN, r, -1, -1);
		add(IR_KILL, r, -1, -1);
		return;
	}

	if (node->ty == ND_IF) {
		int r = gen_expr(node->cond, sp);
		int x = ++label;
		add(IR_UNLESS, r, x, -1);
		add(IR_KILL, r, -1, -1);
		gen_stmt(node->then, sp);
		add(IR_LABEL, x, -1, -1);
		return;
	}

	if (node->ty == ND_ELSE) {
		gen_stmt(node->then, sp);
		return;
	}

	if (node->ty == ND_IF_BLOCK) {
		int r = gen_expr(node->cond, sp);
		int x = ++label;
		int y = ++label;
		int s = stkp;
		add(IR_UNLESS, r, x, -1);
		add(IR_KILL, r, -1, -1);
		for (int i = 0; i < node->stmts->len; i++)
			gen_stmt(node->stmts->data[i], &s);
		add(IR_BLOCK_END, y, -1, -1);
		add(IR_LABEL, x, -1, -1);
		return;
	}

	if (node->ty == ND_ELSE_BLOCK) {
		int x = label;
		int s = stkp;
		for (int i = 0; i < node->stmts_then->len; i++)
			gen_stmt(node->stmts_then->data[i], &s);
		add(IR_LABEL, x, -1, -1);
		stkp = s;
		return;
	}

	if (node->ty == ND_EXPR_STMT) {
		int r = gen_expr(node->expr, sp);
		add(IR_KILL, r, -1, -1);
		return;
	}

	if (node->ty == ND_COMP_STMT) {
		for (int i = 0; i < node->stmts->len; i++)
			gen_stmt(node->stmts->data[i], &stkp);
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
	stkp = 0;
	label = 0;

	IR *alloca = add(IR_ALLOCA, basereg, -1, -1);
	gen_stmt(node, &stkp);
	alloca->rhs = bpoff;
	add(IR_KILL, basereg, -1, -1);
	dump_ir(code);
	return code;
}
