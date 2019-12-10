#include "ecc.h"

int main(int argc, char **argv)
{
	int num_args = 2;

	if (argc == 3 && !strcmp(argv[2], "-debug")) {
		debug = 1;
		num_args++;
	}

	if (argc != num_args) {
		fprintf(stderr, "Usage: ecc <code>\n");
		return 1;
	}

	if (!strcmp(argv[1], "-test")) {
		util_test();
		return 0;
	}

	if (!strcmp(argv[1], "-debug")) {
		debug = 1;
		return 0;
	}

	// Tokenize and parse.
	Vector *tokens = tokenize(argv[1]);
	Program *program = parse(tokens);

	Vector *irv = gen_ir(program);
	alloc_regs(irv);
	gen_riscv(irv);
	return 0;
}
