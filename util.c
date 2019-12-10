#include "ecc.h"

int debug = 0;

noreturn void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

char *format(char *fmt, ...) {
	char buf[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	return strdup(buf);
}

void show_node(const char *function, const char *fmt, ...)
{
	if (!debug)
		return;

	va_list ap;
	fprintf(stdout, " %s ", function);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

Vector *new_vec(void)
{
	Vector *v = calloc(1, sizeof(Vector));
	v->data = malloc(sizeof(void *) * 16);
	v->capacity = 16;
	v->len = 0;

	return v;
}

void vec_push(Vector *v, void *elem)
{
	if (v->len == v->capacity) {
		v->capacity *= 2;
		v->data = realloc(v->data, sizeof(Vector) * v->capacity);
	}
	v->data[v->len++] = elem;
}

Map *new_map(void)
{
	Map *map = calloc(1, sizeof(Map));
	map->keys = new_vec();
	map->vals = new_vec();
	return map;
}

void map_put(Map *map, char *key, void *val)
{
	vec_push(map->keys, key);
	vec_push(map->vals, val);
}

void *map_get(Map *map, char *key)
{
	for (int i = map->keys->len - 1; i >= 0; i--) {
		if (!strcmp(map->keys->data[i], key))
			return map->vals->data[i];
	}
	return NULL;
}

bool map_exists(Map *map, char *key)
{
	for (int i = 0; i < map->keys->len; i++)
		if (!strcmp(map->keys->data[i], key))
			return true;
	return false;
}

void print_horizon(const char *str)
{
	printf("----- %s -----\n", str);
}
