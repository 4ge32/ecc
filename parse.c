#include "ecc.h"

Vector *tokens;
int pos;

static void expect(int ty) {
  Token *t = tokens->data[pos];
  if (t->ty != ty)
    error("%c (%d) expected, but got %c (%d)\n", ty, ty, t->ty, t->ty);
  pos++;
}

static bool consume(int ty) {
  Token *t = tokens->data[pos];
  if (t->ty != ty)
    return false;
  pos++;
  return true;
}

static int read_fn_params(void) {
  Token *t = tokens->data[pos];
  int num_args = 0;

  while (t->ty != ')') {
    t = tokens->data[pos++];
    if (t->ty == ',') {
      continue;
    }
    if (t->ty == TK_INT) {
      num_args++;
      continue;
    }
  }

  return num_args;
}

static Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

static Node *new_node(int op, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *expr();

// term = "(" expr ")" |
static Node *term() {
  Token *t = tokens->data[pos++];

  if (t->ty == '(') {
    Node *node = expr();
    expect(')');
    return node;
  }

  Node *node = malloc(sizeof(Node));

  if (t->ty == TK_NUM) {
    node->ty = ND_NUM;
    node->val = t->val;
    return node;
  }

  if (t->ty == TK_IDENT) {
    node->ty = ND_IDENT;
    node->name = t->name;
    return node;
  }

  error("number expected, but got %s %d", t->input, t->ty);
}

static Node *unary() {
  if (consume('+'))
    return term();
  else if (consume('-'))
    return new_node('-', new_node_num(0), unary());
  return term();
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
  Node *lhs = unary();
  for (;;) {
    Token *t = tokens->data[pos];
    int op = t->ty;
    if (op != '*' && op != '/')
      return lhs;
    pos++;
    lhs = new_node(op, lhs, unary());
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *lhs = mul();
  for (;;) {
    Token *t = tokens->data[pos];
    int op = t->ty;
    if (op != '+' && op != '-')
      return lhs;
    pos++;
    lhs = new_node(op, lhs, mul());
  }
}

// relational = add ("<" add | "<=" add)*
static Node *relational() {
  Node *lhs = add();
  for (;;) {
    Token *t = tokens->data[pos];
    int op = t->ty;
    if (op != '<' && op != TK_LE)
      return lhs;
    pos++;
    lhs = new_node(op, lhs, add());
  }
}

// equality = relational ("==" relational | "!=" relation)*
static Node *equality() {
  Node *lhs = relational();
  for (;;) {
    Token *t = tokens->data[pos];
    int op = t->ty;
    if (op != TK_EQ && op != TK_NE)
      return lhs;
    pos++;
    lhs = new_node(op, lhs, relational());
  }
}

// expr = equality ("=" expr)?
static Node *expr() {
  Node *lhs = equality();
  if (consume('='))
    return new_node('=', lhs, expr());
  return lhs;
}

// stmt = expr ";"
static Node *stmt() {

  if (consume(TK_RETURN)) {
    Node *node = new_node(ND_RETURN, NULL, NULL);
    node->ty = ND_RETURN;
    node->expr = expr();
    expect(';');
    return node;
  }

  if (consume(TK_IF)) {
    Node *node = new_node(ND_IF, NULL, NULL);
    node->ty = ND_IF;
    expect('(');
    node->cond = expr();
    expect(')');
    node->then = stmt();
    if (consume(TK_ELSE)) {
      node->els = stmt();
    }
    return node;
  }

  if (consume('{')) {
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    node->body = new_vec();

    while (!consume('}')) {
      vec_push(node->body, stmt());
    }

    return node;
  }

  Node *node = malloc(sizeof(Node));

  //if (t->ty == TK_INT) {
  //  pos++;
  //  t = tokens->data[pos];
  //  if (t->ty != TK_FUNC) {
  //    pos--;
  //    node->ty = ND_EXPR_STMT;
  //    node->expr = expr();
  //    expect(';');
  //    return node;
  //  }
  //}

  //if (t->ty == TK_FUNC) {
  //  node->ty = ND_FUNC_DEF;
  //  node->name = t->name;
  //  node->stmts = new_vec();
  //  pos++;
  //  expect('(');
  //  int i = 0;
  //  while (t->ty != ')') {
  //    t = tokens->data[pos++];
  //    if (t->ty == ',')
  //      continue;
  //    if (t->ty == TK_INT)
  //      continue;
  //    i++;
  //  }
  //  node->num_arg = i - 1;
  //  expect('{');
  //  while (t->ty != '}') {
  //    vec_push(node->stmts, stmt());
  //    t = tokens->data[pos];
  //  }
  //  expect('}');
  //  return node;
  //}

  //if (t->ty == TK_ELSE) {
  //  node->ty = ND_ELSE;
  //  pos++;
  //  t = tokens->data[pos];
  //  return node;
  //}

  //if (t->ty == '{') {
  //  node->ty = ND_BLOCK;
  //  expect('{');
  //  node->stmts = new_vec();
  //  while (t->ty != '}') {
  //    vec_push(node->stmts, stmt());
  //    t = tokens->data[pos];
  //  }
  //  expect('}');
  //  return node;
  //}

  node->ty = ND_EXPR_STMT;
  node->expr = expr();
  expect(';');
  return node;
}

// fucntion = basetype declarator "(" params? ")" ( { stmt* } | ";")
// params   = param ("," param)* | "void"
// param    = basetype declarator type-suffix
static Function *function(void) {
  Token *t = tokens->data[pos];

  if (t->ty == TK_EOF) {
    return NULL;
  }

  Function *fn = calloc(1, sizeof(Function));

  t = tokens->data[++pos];
  fn->name = t->name;
  fn->num_arg = read_fn_params();
  t = tokens->data[pos];

  if (!consume('{')) {
    return NULL;
  }

  fn->stmts = new_vec();
  while (!consume('}')) {
    vec_push(fn->stmts, stmt());
    t = tokens->data[pos];
  }

  return fn;
}

// program = function*
static Program *program(void) {
  Function head;
  Function *cur = &head;
  Program *prog = malloc(sizeof(Program));

  while (cur) {
    Function *fn = function();
    if (!fn) {
      break;
    }
    cur->next = fn;
    cur = cur->next;
  }

  prog->fns = head.next;
  return prog;
}

void show_descendantTree(Program *prog);

Program *parse(Vector *v) {
  tokens = v;
  pos = 0;

  Program *ret = program();
  show_descendantTree(ret);
  return ret;
}

// static void show_lval(Node *lhs) { printf("%s ", lhs->name); }
//
// static void show_expr(Node *node) {
//  if (node->ty == ND_NUM) {
//    printf("%d ", node->val);
//    return;
//  }
//
//  if (node->ty == ND_IDENT) {
//    printf("%s ", node->name);
//    return;
//  }
//
//  if (node->ty == '=') {
//    printf("%c ", '=');
//    show_lval(node->lhs);
//    show_expr(node->rhs);
//    return;
//  }
//
//  if (node->ty == TK_EQ) {
//    printf("%s ", "==");
//    show_expr(node->lhs);
//    show_expr(node->rhs);
//    return;
//  }
//
//  if (node->ty == TK_NE) {
//    printf("%s ", "!=");
//    show_expr(node->lhs);
//    show_expr(node->rhs);
//    return;
//  }
//
//  if (node->ty == '<') {
//    printf("%c ", '<');
//    show_expr(node->lhs);
//    show_expr(node->rhs);
//    return;
//  }
//
//  if (node->ty == TK_LE) {
//    printf("%s ", "<=");
//    show_expr(node->lhs);
//    show_expr(node->rhs);
//    return;
//  }
//
//  printf("%c ", node->ty);
//  show_expr(node->lhs);
//  show_expr(node->rhs);
//}
//
// static void show_stmt(Node *node) {
//  if (node->ty == ND_RETURN) {
//    printf("return ");
//    show_expr(node->expr);
//    return;
//  }
//
//  if (node->ty == ND_EXPR_STMT) {
//    show_expr(node->expr);
//    return;
//  }
//
//  if (node->ty == ND_IF) {
//    printf("if ");
//    show_expr(node->cond);
//    if (node->then) {
//      show_stmt(node->then);
//    }
//    return;
//  }
//
//  if (node->ty == ND_BLOCK) {
//    printf("{ ");
//    for (int i = 0; i < node->stmts->len; i++) {
//      if (node->name)
//        printf("%s ", node->name);
//      show_stmt(node->stmts->data[i]);
//      printf("\n");
//    }
//    printf("} ");
//  }
//
//  if (node->ty == ND_FUNC_DEF) {
//    printf("%s: \n", node->name);
//    for (int i = 0; i < node->stmts->len; i++) {
//      show_stmt(node->stmts->data[i]);
//      printf("\n");
//    }
//    return;
//  }
//}
static void show_expr(Node *node) {
  if (node->ty == ND_EQ) {
    printf("%s ", "==");
    show_expr(node->lhs);
    show_expr(node->rhs);
    return;
  }
  if (node->ty == ND_NUM) {
    printf("%d ", node->val);
    return;
  }
}

static void print_node(Node *node) {
  if (node->ty == ND_BLOCK) {
    printf("{ ");
    Vector *stmts = node->body;
    for (int i = 0; i < stmts->len; i++) {
      Node *node = stmts->data[i];
      printf("[%d]\n", node->ty);
      print_node(node);
    }
    printf(" }");
    return;
  }

  if (node->ty == ND_RETURN) {
    printf("return ");
    show_expr(node->expr);
    printf("\n");
    return;
  }

  if (node->ty == ND_IF) {
    printf("if ");
    printf("[ COND: ");
    show_expr(node->cond);
    printf("]\n");

    Vector *body = node->then->body;
    for (int i = 0; i < body->len; i++) {
      Node *node = body->data[i];
      print_node(node);
    }

    if (node->els) {
      Vector *body = node->els->body;
      for (int i = 0; i < body->len; i++) {
        Node *node = body->data[i];
        print_node(node);
      }
    }
    return;
  }
}

void print_func(Function *cur) {
  Vector *stmts = cur->stmts;
  printf("%s ", cur->name);
  for (int i = 0; i < stmts->len; i++) {
    Node *node = stmts->data[i];
    print_node(node);
  }
}

void show_descendantTree(Program *prog) {
  if (!debug)
    return;

  printf("---PARSE---\n");

  Function *cur = prog->fns;
  while (cur) {
    print_func(cur);
    cur = cur->next;
  }

  printf("\n");
  // for (int i = 0; i < tree->stmts->len; i++) {
  //  show_stmt(tree->stmts->data[i]);
  //  printf("\n");
  //}
}
