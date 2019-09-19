#include "ecc.h"

Vector *tokens;
int pos;

static Node *assign();

static void expect(int ty)
{
	Token *t = tokens->data[pos];
	if (t->ty != ty)
		error("%c (%d) expected, but got %c (%d)\n", ty, ty, t->ty, t->ty);
	pos++;
}

static bool consume(int ty)
{
	Token *t = tokens->data[pos];
	if (t->ty != ty)
		return false;
	pos++;
	return true;
}

static Node *new_node_num(int val)
{
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

static Node *new_node(int op, Node *lhs, Node *rhs)
{
	Node *node = malloc(sizeof(Node));
	node->ty = op;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *term()
{
	Token *t = tokens->data[pos++];

	if (t->ty == '(') {
		Node *node = assign();
		expect(')');
		return node;
	}

	Node *node = malloc(sizeof(Node));

	if (t->ty == TK_NUM) {
		node->ty = ND_NUM;
		node->val = t->val;
		return node;
	}

	if (t->ty == TK_IDENT) {
		node->ty = ND_IDENT;
		node->name = t->name;
		return node;
	}

	if (t->ty == TK_FUNC) {
		node->ty = ND_FUNC;
		node->name = t->name;
		node->stmts = new_vec();
		expect('(');
		int i = 0;
		while (t->ty != ')') {
			t = tokens->data[pos++];
			if (t->ty == ',')
				continue;
			node->arg[i++] = (int)strtol(t->input, NULL, 10);
		}
		node->num_arg = i - 1;
		return node;
	}

	error("number expected, but got %s", t->input);
}

static Node *unary()
{
	if (consume('+'))
		return term();
	if (consume('-'))
		return new_node('-', new_node_num(0), unary());
	return term();
}

static Node *mul()
{
	Node *lhs = unary();
	for (;;) {
		Token *t = tokens->data[pos];
		int op = t->ty;
		if (op != '*' && op != '/')
			return lhs;
		pos++;
		lhs = new_node(op, lhs, unary());
	}
}

static Node *equality()
{
	Node *lhs = mul();
	for (;;) {
		Token *t = tokens->data[pos];
		int op = t->ty;
		if (op != ND_EQ && op != ND_NE)
			return lhs;
		pos++;
		lhs = new_node(op, lhs, mul());
	}
}

static Node *expr()
{
	Node *lhs = equality();
	for (;;) {
		Token *t = tokens->data[pos];
		int op = t->ty;
		if (op != '+' && op != '-')
			return lhs;
		pos++;
		lhs = new_node(op, lhs, equality());
	}
}

static Node *assign()
{
	Node *lhs = expr();
	if (consume('='))
		return new_node('=', lhs, expr());
	return lhs;
}

static Node *stmt()
{
	Node *node = malloc(sizeof(Node));
	Token *t = tokens->data[pos];

	if (t->ty == TK_INT) {
		pos++;
		t = tokens->data[pos];
		if (t->ty != TK_FUNC) {
			pos--;
			node->ty = ND_EXPR_STMT;
			node->expr = assign();
			expect(';');
			return node;
		}
	}

	if (t->ty == TK_FUNC) {
		node->ty = ND_FUNC_DEF;
		node->name = t->name;
		node->stmts = new_vec();
		pos++;
		expect('(');
		int i = 0;
		while (t->ty != ')') {
			t = tokens->data[pos++];
			if (t->ty == ',')
				continue;
			if (t->ty == TK_INT)
				continue;
			i++;
		}
		node->num_arg = i - 1;
		expect('{');
		while (t->ty != '}') {
			vec_push(node->stmts, stmt());
			t = tokens->data[pos];
		}
		expect('}');
		return node;
	}

	if (t->ty == TK_IF) {
		pos++;
		node->ty = ND_IF;
		expect('(');
		node->cond = assign();
		expect(')');

		// {}
		t = tokens->data[pos];
		if (t->ty == '{') {
			node->ty = ND_IF_BLOCK;
			expect('{');
			node->stmts = new_vec();
			while (t->ty != '}') {
				vec_push(node->stmts, stmt());
				t = tokens->data[pos];
			}
			expect('}');
		} else {
			node->then = stmt();
		}
		return node;
	}

	if (t->ty == TK_ELSE) {
		pos++;
		t = tokens->data[pos];
		if (t->ty == '{') {
			expect('{');
			node->ty = ND_ELSE_BLOCK;
			node->stmts_then = new_vec();
			while (t->ty != '}') {
				vec_push(node->stmts_then, stmt());
				t = tokens->data[pos];
			}
			expect('}');
		} else {
			node->ty = ND_ELSE;
			node->then = stmt();
		}
		return node;
	}

	if (t->ty == TK_RETURN) {
		pos++;
		node->ty = ND_RETURN;
		node->expr = assign();
		expect(';');
		return node;
	}

	node->ty = ND_EXPR_STMT;
	node->expr = assign();
	expect(';');
	return node;
}

static Node *compound_stmt()
{
	Node *node = malloc(sizeof(Node));
	node->ty = ND_COMP_STMT;
	node->stmts = new_vec();

	for (;;) {
		Token *t = tokens->data[pos];
		if (t->ty == TK_EOF)
			return node;
		vec_push(node->stmts, stmt());
	}
}

Node *parse(Vector *v)
{
	tokens = v;
	pos = 0;

	Node *ret = compound_stmt();
	show_descendantTree(ret);
	return ret;
}
