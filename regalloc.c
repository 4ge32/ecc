#include "ecc.h"

char *regs[] = {"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"};
char *func_regs[] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

static bool used[sizeof(regs) / sizeof(*regs)];
static int *reg_map;

static bool arg_used[sizeof(func_regs) / sizeof(*func_regs)];
static int *arg_map;

static int alloc(int ir_reg)
{
	if (reg_map[ir_reg] != -1) {
		int r = reg_map[ir_reg];
		assert(used[r]);
		return r;
	}

	for (int i = 0; i < sizeof(regs) / sizeof(*regs); i++) {
		if (used[i])
			continue;
		used[i] = true;
		reg_map[ir_reg] = i;
		return i;
	}

	error("register exhausted");
}

static int alloc_args(int ir_reg)
{
	if (arg_map[ir_reg] != -1) {
		int r = arg_map[ir_reg];
		assert(arg_used[r]);
		return r;
	}

	for (int i = 0; i < sizeof(func_regs) / sizeof(*func_regs); i++) {
		if (arg_used[i])
			continue;
		arg_used[i] = true;
		arg_map[ir_reg] = i;
		return i;
	}

	error("register exhausted");
}

static void kill(int r) {
	assert(used[r]);
	used[r] = false;
	//printf("KILL-%d\n", r);
}

void alloc_regs(Vector *irv)
{
	reg_map = malloc(sizeof(int) * irv->len);
	for (int i = 0; i < irv->len; i++)
		reg_map[i] = -1;

	arg_map = malloc(sizeof(int) * irv->len);
	for (int i = 0; i < irv->len; i++)
		arg_map[i] = -1;

	for (int i = 0; i < irv->len; i++) {
		IR *ir = irv->data[i];

		switch (ir->op) {
		case IR_IMM:
		case IR_RET:
			ir->lhs = alloc(ir->lhs);
			break;
		case IR_KILL:
			kill(reg_map[ir->lhs]);
			ir->op = IR_NOP;
			break;
		case IR_EQ:
		case IR_NE:
		case IR_LE:
			ir->lhs = alloc(ir->lhs);
			ir->rhs = alloc(ir->rhs);
			break;
		case IR_STORE:
		case IR_MOV:
		case IR_LOAD:
		case '+':
		case '-':
		case '*':
		case '/':
		case '<':
		case IR_PUSH:
			ir->rhs = alloc_args(ir->lhs);
			break;
		case IR_POP:
		//case IR_FUNC:
		//	ir->lhs = alloc(ir->lhs);
		//	break;
		//case IR_UNLESS:
		case IR_TRUE:
		case IR_FALSE:
		case IR_FUNC_IN:
		case IR_FUNC_OUT:
			ir->lhs = alloc(ir->lhs);
			break;
		case IR_ELSE:
			break;
		//case IR_FUNC_RET:
		//case IR_FUNC_DEF:
			break;
		case IR_LABEL:
		//case IR_BLOCK_END:
		//case IR_DUMMY1:
		//case IR_DUMMY2:
		//case IR_JP:
			break;
		default:
			assert(0 && "unknown operator");
		}
	}

	for (int i = 0; i < sizeof(regs) / sizeof(*regs); i++) {
		if (used[i]) {
			assert(true);
		}
	}
}
