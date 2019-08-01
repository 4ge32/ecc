#include "ecc.h"

static char *gen_label()
{
	static int n;
	char buf[10];
	sprintf(buf, ".L%d", n++);
	return strdup(buf);
}

void gen_riscv(Vector *irv) {
	char *ret = gen_label();
	int sp = 0;

	for (int i = 0; i < irv->len; i++) {
		IR *ir = irv->data[i];

		switch (ir->op) {
		case IR_IMM:
			printf("  addi %s, zero, %d\n", regs[ir->lhs], ir->rhs);
			break;
		case IR_MOV:
			printf("? mv   %s, %s\n", regs[ir->lhs], regs[ir->rhs]);
			break;
		case IR_RETURN:
			printf("  mv   a0, %s\n", regs[ir->lhs]);
			printf("  j    %s\n", ret);
			break;
		case IR_ALLOCA:
			if (ir->rhs)
				printf("  addi sp, sp, -%d\n", ir->rhs);
			printf("  mv   %s, sp\n", regs[ir->lhs]);
			break;
		case IR_LOAD:
			sp -= 4;
			printf("  lw   %s, %d(sp)\n", regs[ir->lhs], sp);
			break;
		case IR_STORE:
			printf("  sw   %s, %d(sp)\n", regs[ir->rhs], sp);
			sp += 4;
			break;
		case IR_UNLESS:
			printf("  beq  %s, zero, .L%d\n", regs[ir->lhs], ir->rhs);
			break;
		case IR_LABEL:
			printf(".L%d:\n", ir->lhs);
			break;
		case '+':
			printf("  add  %s, %s, %s\n",
			       regs[ir->lhs], regs[ir->lhs], regs[ir->rhs]);
			break;
		case '-':
			printf("  sub  %s, %s, %s\n",
			       regs[ir->lhs], regs[ir->lhs], regs[ir->rhs]);
			break;
		case '*':
			printf("  mul  %s, %s, %s\n",
			       regs[ir->lhs], regs[ir->lhs], regs[ir->rhs]);
			break;
		case '/':
			printf("  div %s, %s, %s\n",
			       regs[ir->lhs], regs[ir->lhs], regs[ir->rhs]);
			break;
		case IR_NOP:
			break;
		default:
			assert(0 && "unknown operator");
		}
	}
	printf("%s:\n", ret);
	printf("  ret\n");
}
