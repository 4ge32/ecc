#include "ecc.h"

const char *RET = "0";

static char *gen_label() {
  static int n;
  char buf[10];
  sprintf(buf, ".L%d", n++);
  return strdup(buf);
}

void gen_riscv(Vector *irv) {
  char *ret = gen_label();

  for (int i = 0; i < irv->len; i++) {
    IR *ir = irv->data[i];

    switch (ir->op) {
    case IR_IMM:
      printf("  li   %s, %d\n", regs[ir->lhs], ir->rhs);
      break;
    case IR_MOV:
      printf("? mv   %s, %s\n", regs[ir->lhs], regs[ir->rhs]);
      break;
    case IR_RETURN:
      printf("  mv   a0, %s\n", regs[ir->lhs]);
      printf("  j    %s\n", ret);
      break;
    case IR_RET:
      printf("  j    .L%d\n", ir->opt);
      // printf("  mv   a0, %s\n", regs[ir->lhs]);
      // printf("  jr   ra\n");
      break;
    case IR_ALLOCA:
      printf("  addi sp, sp, -%d\n", ir->rhs);
      printf("  sd   %s, %d(sp)\n", regs[ir->lhs], ir->rhs - 8);
      printf("  addi %s, sp, %d\n", regs[ir->lhs], ir->rhs);
      break;
    case IR_DEALLOCA:
      printf("  addi sp, sp, %d\n", ir->rhs);
      printf("  ld   %s, %d(sp)\n", regs[ir->lhs], ir->rhs - 8);
      printf("  mv   %s, sp, %d\n", regs[ir->lhs], ir->rhs);
      break;
    case IR_LOAD:
      printf("  lw   %s, -%d(sp)\n", regs[ir->lhs], ir->sp);
      break;
    case IR_STORE:
      printf("  mv   %s, %s\n", regs[ir->lhs], regs[ir->rhs]);
      printf("  sw   %s, -%d(sp)\n", regs[ir->lhs], ir->sp);
      break;
    case IR_UNLESS:
      printf("  beq  %s, zero, .L%d\n", regs[ir->lhs], ir->opt);
      break;
    case IR_TRUE:
      printf("  bne  %s, zero, .L%d\n", regs[ir->lhs], ir->rhs);
      break;
    case IR_FALSE:
      printf("  beq  %s, zero, .L%d\n", regs[ir->lhs], ir->rhs);
      break;
    case IR_ELSE:
      printf("  j   .L%d\n", ir->lhs);
      break;
    case IR_LABEL:
      printf(".L%d:\n", ir->lhs);
      break;
    case IR_BLOCK_END:
      printf("  j   .L%d\n", ir->lhs);
      break;
    case IR_FUNC:
      printf("  j    %s\n", ir->name);
      printf("  mv   %s, a0\n", regs[ir->lhs]);
      break;
    case IR_FUNC_DEF:
      if (!strcmp(ir->name, "main")) {
        printf("\n.global main");
      }
      printf("\n%s:\n", ir->name);
      break;
    case IR_FUNC_RET:
      printf("  jr   ra\n");
      break;
    case IR_PUSH:
      printf("  li   %s, %d\n", func_regs[ir->rhs], ir->lhs);
      break;
    case IR_POP:
      printf("  mv   %s, %s\n", regs[ir->lhs], func_regs[ir->rhs]);
      printf("  sw   %s, %d(sp)\n", regs[ir->lhs], ir->sp);
      break;
    case '+':
      printf("  add  %s, %s, %s\n", regs[ir->lhs], regs[ir->lhs],
             regs[ir->rhs]);
      break;
    case '-':
      printf("  sub  %s, %s, %s\n", regs[ir->lhs], regs[ir->lhs],
             regs[ir->rhs]);
      break;
    case '*':
      printf("  mul  %s, %s, %s\n", regs[ir->lhs], regs[ir->lhs],
             regs[ir->rhs]);
      break;
    case '/':
      printf("  div %s, %s, %s\n", regs[ir->lhs], regs[ir->lhs], regs[ir->rhs]);
      break;
    case IR_EQ:
      printf("  bne  %s, %s, .L%d\n", regs[ir->lhs], regs[ir->rhs], ir->opt);
      break;
    case IR_NE:
      printf("  beq  %s, %s, .L%d\n", regs[ir->lhs], regs[ir->rhs], ir->opt);
      break;
    case IR_LE:
      printf("  bge  %s, %s", regs[ir->lhs], regs[ir->rhs]);
      break;
    case '<':
      printf("  bgt  %s, %s", regs[ir->lhs], regs[ir->rhs]);
      break;
    case IR_JP:
      printf(" L%d\n", ir->lhs);
      break;
    case IR_DUMMY1:
      printf("---START---\n");
      break;
    case IR_DUMMY2:
      printf("--- END ---\n");
      break;
    case IR_NOP:
      break;
    /* function prologue */
    case IR_FUNC_IN:
      printf(".global %s\n", ir->name);
      printf("%s:\n", ir->name);
      printf("  addi sp, sp, -32\n");
      //printf("  sd   %s, 24(sp)\n", regs[ir->lhs]);
      break;
    /* function epilogue */
    case IR_FUNC_OUT:
      printf(".L%d:\n", ir->opt);
      printf("  mv    a0, s1\n");
      printf("  addi  sp, sp, 32\n");
      printf("  jr    ra\n");
      break;
    default:
      assert(0 && "unknown operator");
      break;
    }
  }
}
