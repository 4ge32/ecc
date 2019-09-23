#include "ecc.h"

/* Display result of tokenize */
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
		else if (t->ty == TK_INT)
			printf("TK_INT:  %s(%d)\n", t->name, t->ty);
		else if (t->ty == TK_IF)
			printf("TK_IF:     %s(%d)\n", t->name, t->ty);
		else if (t->ty == TK_ELSE)
			printf("TK_ELSE:   %s(%d)\n", t->name, t->ty);
		else if (t->ty == TK_FUNC)
			printf("TK_FUNC:   %s\n", t->name);
		else
			printf("ELSE:      %c(%d)\n", (char)t->ty, t->ty);
	}
	printf("\n");
}
