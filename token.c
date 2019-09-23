#include "ecc.h"

Map *keywords;

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

		/* Firstly, you should tokenize longer tokens. */
		if (!strncmp(p, "==", 2)) {
			add_token(v, TK_EQ, p);
			p += 2;
			continue;
		}

		if (!strncmp(p, "!=", 2)) {
			add_token(v, TK_NE, p);
			p += 2;
			continue;
		}

		if (!strncmp(p, "<=", 2)) {
			add_token(v, TK_LE, p);
			p += 2;
			continue;
		}

		if (strchr("+-*/;=(){},<", *p)) {
			add_token(v, *p, p);
			p++;
			continue;
		}

		/* Identifier */
		if (isalpha(*p) || *p == '_') {
			int len = 1;
			while (isalpha(p[len]) || isdigit(p[len]) || p[len] == '_')
				len++;

			/* Returns a pointer to a new string "name"
			 * which is a duplicate of the string p.
			 */
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
