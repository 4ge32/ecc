#include "ecc.h"

Map *keywords;

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


static Token *add_token(Vector *v, int ty, char *input)
{
	Token *t = malloc(sizeof(Token));
	t->ty = ty;
	t->input = input;
	vec_push(v, t);

	return t;
}

static Vector *scan(char *p)
{
	Vector *v = new_vec();

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strchr("+-*/;=(){},", *p)) {
			add_token(v, *p, p);
			p++;
			continue;
		}

		// Identifier
		if (isalpha(*p) || *p == '_') {
			int len = 1;
			while (isalpha(p[len]) || isdigit(p[len]) || p[len] == '_')
				len++;


			// returns a pointer to a new string which is a duplicate of the string s.
			char *name = strndup(p, len);
			int ty = (intptr_t)map_get(keywords, name);
			if (!ty)
				ty = TK_IDENT;

			if (*(p + len) == '(')
				ty = TK_FUNC;

			Token *t = add_token(v, ty, p);
			t->name = name;
			p += len;
			continue;
		}

		if (isdigit(*p)) {
			Token *t = add_token(v, TK_NUM, p);
			t->val = strtol(p, &p, 10);
			continue;
		}

		error("cannot tokenize: %s", p);
	}

	add_token(v, TK_EOF, p);
	show_token(v);
	return v;
}

Vector *tokenize(char *p)
{
	keywords = new_map();
	map_put(keywords, "if", (void *)TK_IF);
	map_put(keywords, "else", (void *)TK_ELSE);
	map_put(keywords, "return", (void *)TK_RETURN);
	map_put(keywords, "int", (void *)TK_INT);
	return scan(p);
}
