#include "ecc.h"

static Vector *code = NULL;
static int regno = 0;
// static int basereg;
// static Map *vars;
// static int bpoff;
// static int _bpoff = 0;
//
static int label = 2;
static int ret = 0;
static int stkp = 0;
// static int stkp;
// static int seq;
//
//
// static IR *add(int op, int lhs, int rhs, int sp)
//{
//	IR *ir = malloc(sizeof(IR));
//	ir->op = op;
//	ir->lhs = lhs;
//	ir->rhs = rhs;
//	ir->sp = sp;
//	vec_push(code, ir);
//	return ir;
//}
//
// static IR *call(int op, int lhs, char *name)
//{
//	IR *ir = malloc(sizeof(IR));
//	ir->op = op;
//	ir->lhs = lhs;
//	ir->name = name;
//	vec_push(code, ir);
//	return ir;
//}
//
// static int gen_lval(Node *node)
//{
//	if (node->ty != ND_IDENT)
//		error("not an lvalue");
//
//	if (!map_exists(vars, node->name)) {
//		map_put(vars, node->name, (void *)(intptr_t)bpoff);
//		bpoff += 4;
//	}
//
//	return ++regno;
//}
//
// static int gen_expr(Node *node, int *sp)
//{
//	if (node->ty == ND_NUM) {
//		int r = ++regno;
//		add(IR_IMM, r, node->val, *sp);
//		return r;
//	}
//
//	if (node->ty == ND_IDENT) {
//		int r = gen_lval(node);
//		_bpoff += 4;
//		add(IR_LOAD, r, r, _bpoff);
//		return r;
//	}
//	if (node->ty == '=') {
//		int rhs = gen_expr(node->rhs, sp);
//		int lhs = gen_lval(node->lhs);
//		add(IR_STORE, lhs, rhs, bpoff);
//		add(IR_KILL, rhs, -1, -1);
//		return lhs;
//	}
//
//	if (node->ty == ND_FUNC) {
//		int r = ++regno;
//		for (int i = 0; i < node->num_arg; i++) {
//			add(IR_PUSH, node->arg[i], -1, -1);
//		}
//		call(IR_FUNC, r, node->name);
//		return r;
//	}
//
//	if (node->ty == ND_EQ) {
//		int lhs = gen_expr(node->lhs, sp);
//		int rhs = gen_expr(node->rhs, sp);
//
//		add(IR_EQ, lhs, rhs, -1);
//		add(IR_KILL, rhs, -1, -1);
//		return lhs;
//	}
//
//	if (node->ty == ND_NE) {
//		int lhs = gen_expr(node->lhs, sp);
//		int rhs = gen_expr(node->rhs, sp);
//
//		add(IR_NE, lhs, rhs, -1);
//		add(IR_KILL, rhs, -1, -1);
//		add(IR_KILL, lhs, -1, -1);
//		return lhs;
//	}
//
//	if (node->ty == ND_LE) {
//		int lhs = gen_expr(node->lhs, sp);
//		int rhs = gen_expr(node->rhs, sp);
//
//		add(IR_LE, lhs, rhs, -1);
//		add(IR_JP, label, -1, -1);
//		add(IR_KILL, rhs, -1, -1);
//		add(IR_KILL, lhs, -1, -1);
//		return lhs;
//	}
//
//	if (node->ty == '<') {
//		int lhs = gen_expr(node->lhs, sp);
//		int rhs = gen_expr(node->rhs, sp);
//
//		add(IR_LE, lhs, rhs, -1);
//		add(IR_JP, label, -1, -1);
//		add(IR_KILL, rhs, -1, -1);
//		add(IR_KILL, lhs, -1, -1);
//		return lhs;
//	}
//
//	assert(strchr("+-*/", node->ty));
//
//	int lhs = gen_expr(node->lhs, sp);
//	int rhs = gen_expr(node->rhs, sp);
//
//	add(node->ty, lhs, rhs, -1);
//	add(IR_KILL, rhs, -1, -1);
//	return lhs;
//}
//
// static void gen_stmt(Node *node, int *sp)
//{
//	if (node->ty == ND_RETURN) {
//		int r = gen_expr(node->expr, sp);
//		add(IR_RETURN, r, -1, -1);
//		add(IR_KILL, r, -1, -1);
//		return;
//	}
//
//	if (node->ty == ND_EXPR_STMT) {
//		int r = gen_expr(node->expr, sp);
//		add(IR_KILL, r, -1, -1);
//		return;
//	}
//
//	if (node->ty == ND_IF) {
//		gen_expr(node->cond, sp);
//		if (node->then) {
//			gen_stmt(node->then, sp);
//			add(IR_LABEL, label, -1, -1);
//		}
//		return;
//	}
//
//	if (node->ty == ND_ELSE) {
//		add(IR_LABEL, label, -1, -1);
//		if (node->then) {
//			gen_stmt(node->then, sp);
//			add(IR_LABEL, label, -1, -1);
//		}
//		return;
//	}
//
//	if (node->ty == ND_COMP_STMT) {
//		for (int i = 0; i < node->stmts->len; i++) {
//			gen_stmt(node->stmts->data[i], &stkp);
//		}
//		return;
//	}
//
//	if (node->ty == ND_FUNC_DEF) {
//		bpoff = 32;
//		call(IR_FUNC_DEF, -1, node->name);
//		IR *alloca = add(IR_ALLOCA, basereg, bpoff, -1);
//		bpoff -= 16;
//		_bpoff = bpoff;
//		seq = label;
//		label++;
//		for (int i = 0; i < node->num_arg; i++) {
//			add(IR_POP, i, i, -8 * (i + 1));
//		}
//		for (int i = 0; i < node->stmts->len; i++) {
//			add(IR_DUMMY1, -1, -1, -1);
//			gen_stmt(node->stmts->data[i], &stkp);
//			add(IR_DUMMY2, -1, -1, -1);
//		}
//		add(IR_LABEL, seq, -1, -1);
//		add(IR_DEALLOCA, basereg, alloca->rhs, -2);
//		add(IR_KILL, basereg, -1, -1);
//		add(IR_FUNC_RET, - 1, -1, -1);
//		return;
//	}
//
//	if (node->ty == ND_BLOCK) {
//		int s = stkp;
//		for (int i = 0; i < node->stmts->len; i++)
//			gen_stmt(node->stmts->data[i], &s);
//		add(IR_RETURN, -1, -1, -1);
//		//add(IR_BLOCK_END, label, -1, -1);
//		//add(IR_LABEL, label, -1, -1);
//		return;
//	}
//
//
//	error("unknown node: %d", node->ty);
//}

static IR *new_ir(void) { return malloc(sizeof(IR)); }

static void add(int op, int lhs, int rhs, int opt) {
  IR *ir = new_ir();
  ir->op = op;
  ir->lhs = lhs;
  ir->rhs = rhs;
  ir->opt = opt;
  stkp += 4;
  vec_push(code, ir);
}

static void add_ret(int lhs) {
  add(IR_RET, lhs, -1, ret);
}

static void add_imm(int lhs, int rhs) {
  add(IR_IMM, lhs, rhs, -1);
}

static void add_label(void) {
  add(IR_LABEL, label++, 0, -1);
}

static void add_branch(int op, int lhs, int rhs) {
  add(op, lhs, rhs, label);
}

static void kill(int lhs) {
  add(IR_KILL, lhs, -1, -1);
}

static IR *enter_function(Function *fn) {
  IR *ir = new_ir();
  ret = label;
  ir->op = IR_FUNC_IN;
  ir->lhs = regno++;
  ir->opt = label++;
  ir->name = fn->name;
  stkp = 0;
  vec_push(code, ir);
  return ir;
}

static void leave_function(IR *in) {
  IR *out = new_ir();
  out->op = IR_FUNC_OUT;
  out->lhs = regno++;
  out->opt = in->opt;
  vec_push(code, out);
  kill(in->lhs);
  kill(out->lhs);
}

static int gen_expr(Node *node) {
  if (node->ty == ND_NUM) {
    int r = regno++;
    add_imm(r, node->val);
    return r;
  }

  if (node->ty == ND_EQ) {
    int lhs = gen_expr(node->lhs);
    int rhs = gen_expr(node->rhs);
    add_branch(IR_EQ, lhs, rhs);
    kill(lhs);
    kill(rhs);
    return lhs;
  }

  if (node->ty == ND_NE) {
    int lhs = gen_expr(node->lhs);
    int rhs = gen_expr(node->rhs);
    add_branch(IR_NE, lhs, rhs);
    kill(lhs);
    kill(rhs);
    return lhs;
  }
  return -1;
}

static void ipt_stmt(Node *node) {
  if (node->ty == ND_RETURN) {
    int r = gen_expr(node->expr);
    add_ret(r);
    kill(r);
    return;
  }

  if (node->ty == ND_IF) {
    Vector *body;
    gen_expr(node->cond);
    body = node->then->body;
    for (int i = 0; i < body->len; i++) {
      Node *node = body->data[i];
      ipt_stmt(node);
    }
    add_label();
    if (node->els) {
      body = node->els->body;
      for (int i = 0; i < body->len; i++) {
        Node *node = body->data[i];
        ipt_stmt(node);
      }
    }
  }
}

static void ipt_stmts(Vector *stmts) {
  for (int i = 0; i < stmts->len; i++) {
    ipt_stmt(stmts->data[i]);
  }
}

static void gen_code(Program *program) {
  Function *fn = program->fns;
  while (fn) {
    IR *ir = enter_function(fn);
    ipt_stmts(fn->stmts);
    fn = fn->next;
    leave_function(ir);
  }
}

void dump_ir(Vector *irv);
Vector *gen_ir(Program *program) {
  code = new_vec();
  gen_code(program);
  dump_ir(code);
  return code;
}

IRInfo irinfo[] = {
    [IR_IMM] = {"IMM", IR_IMM},
    [IR_RET] = {"RET", IR_RET},
    [IR_EQ] = {"EQ", IR_EQ},
    [IR_MOV] = {"MOV", IR_KILL},
    [IR_RETURN] = {"RET", IR_RETURN},
    [IR_ALLOCA] = {"ALLOC", IR_ALLOCA},
    [IR_DEALLOCA] = {"DEALLOC", IR_ALLOCA},
    [IR_LOAD] = {"LOAD", IR_LOAD},
    [IR_STORE] = {"STORE", IR_STORE},
    [IR_KILL] = {"KILL", IR_KILL},
    [IR_NOP] = {"NOP", IR_NOP},
    [IR_UNLESS] = {"UNLESS", IR_UNLESS},
    [IR_LE] = {"LessEq", IR_LE},
    [IR_LABEL] = {"LABEL", IR_LABEL},
    [IR_BLOCK_END] = {"BLK_END", IR_BLOCK_END},
    [IR_ELSE] = {"ELSE", IR_ELSE},
    [IR_FUNC] = {"CALL", IR_FUNC},
    [IR_PUSH] = {"PUSH", IR_PUSH},
    [IR_FUNC_DEF] = {"FUNC:", IR_FUNC_DEF},
    ['+'] = {"ADD", '+'},
    ['-'] = {"SUB", '-'},
    ['*'] = {"MUL", '*'},
};

static char *tostr(IR *ir) {
  IRInfo info = irinfo[ir->op];

  switch (info.ty) {
  case IR_IMM:
    return format("%7s%3d[reg]%3d[val] ", info.name, ir->lhs, ir->rhs);
  case IR_RET:
    return format("%7s%3d    ", info.name, ir->lhs);
  case IR_EQ:
    return format("%7s %2d[reg] %2d[reg]", info.name, ir->lhs, ir->rhs);
  case IR_RETURN:
    return format("%7s%3d    ", info.name, ir->lhs);
  case IR_ALLOCA:
    return format("%7s%3d[reg]", info.name, ir->lhs);
  case IR_LOAD:
    return format("%7s%3d[reg]%3d[src] ", info.name, ir->lhs, ir->sp);
  case IR_STORE:
    return format("%7s%3d[reg]%3d[dest] ", info.name, ir->rhs, ir->sp);
  case IR_NOP:
    return format("%7s       ", info.name);
  case IR_KILL:
    return format("%7s%3d[reg]  ", info.name, ir->lhs);
  case IR_UNLESS:
    return format("%7s%3d L.%d ", info.name, ir->lhs, ir->rhs);
  case IR_LE:
    return format("%7s %2d <=%2d ", info.name, ir->lhs, ir->rhs);
  case IR_LABEL:
    return format("%7s  .L%d  ", info.name, ir->lhs);
  case IR_BLOCK_END:
    return format("%7s  .L%d  ", info.name, ir->lhs);
  case IR_FUNC:
    return format("%7s  %s  ", info.name, ir->name);
  case IR_FUNC_DEF:
    return format("%s%8s", info.name, ir->name);
  case IR_FUNC_IN:
    return format("IN:\n");
  case IR_FUNC_OUT:
    return format("\n");
  case IR_PUSH:
    return format("%7s  %d  ", info.name, ir->lhs);
  case '+':
    return format("%7s%3d[reg]%3d[reg] ", info.name, ir->lhs, ir->rhs);
  case '*':
    return format("%7s%3d[reg]%3d[reg] ", info.name, ir->lhs, ir->rhs);
  default:
    return format("unimplemented: %d\n", info.ty);
  }
}

void dump_ir(Vector *irv) {
  if (!debug)
    return;

  for (int i = 0; i < irv->len; i++)
    printf("%s\n", tostr(irv->data[i]));
  printf("\n");
}