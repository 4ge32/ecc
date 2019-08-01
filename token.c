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

		if (strchr("+-*/;=()", *p)) {
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
	map_put(keywords, "if", (void *)TK_COND);
	map_put(keywords, "return", (void *)TK_RETURN);
	return scan(p);
}
