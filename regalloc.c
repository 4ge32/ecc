#include "ecc.h"

char *regs[] = {"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"};

static bool used[sizeof(regs) / sizeof(*regs)];
static int *reg_map;

static int alloc(int ir_reg)
{
	if (reg_map[ir_reg] != -1) {
		int r = reg_map[ir_reg];
		assert(used[r]);
		//printf("!! %d\n", r);
		return r;
	}

	for (int i = 0; i < sizeof(regs) / sizeof(*regs); i++) {
		if (used[i])
			continue;
		used[i] = true;
		reg_map[ir_reg] = i;
		//printf("ALOC-%d\n", i);
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

	for (int i = 0; i < irv->len; i++) {
		IR *ir = irv->data[i];

		switch (ir->op) {
		case IR_IMM:
		case IR_ALLOCA:
		case IR_RETURN:
			ir->lhs = alloc(ir->lhs);
			break;
		case IR_STORE:
		case IR_MOV:
		case IR_LOAD:
		case '+':
		case '-':
		case '*':
		case '/':
			ir->lhs = alloc(ir->lhs);
			ir->rhs = alloc(ir->rhs);
			break;
		case IR_UNLESS:
			ir->lhs = alloc(ir->lhs);
			break;
		case IR_ELSE:
			break;
		case IR_KILL:
			kill(reg_map[ir->lhs]);
			ir->op = IR_NOP;
			break;
		case IR_LABEL:
		case IR_BLOCK_END:
		case IR_FUNC:
			break;
		default:
			assert(0 && "unknown operator");
		}
	}
}

