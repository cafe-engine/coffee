#include "coffee.h"

#define car(x) ( peek(vm, (x)->car) )
#define cdr(x) ( peek(vm, (x)->cdr) )
#define setcar(x, b) ( (x)->car = addr(vm, (b)) )
#define setcdr(x, b) ( (x)->cdr = addr(vm, (b)) )

#define header(x) ((x)->tag)

#define isnull(x) ((x) == cf_null(vm))

#define type(x) (header((x)) & TYPE_MASK)
#define settype(x, t) (header((x)) = (header((x)) & MARK_MASK) | (t & TYPE_MASK))

#define mark(x) (header((x)) |= MARK_MASK)
#define unmark(x) (header((x)) = header((x)) & TYPE_MASK)
#define marked(x) (header((x)) & MARK_MASK)

#define next(x) ( peek(vm, (x)->next) )
#define setnext(x, b) ( (x)->next = addr(vm, (b)) )

#define number(x) ((x)->n)
#define string(x) (cdr((x))->str)
#define cfunc(x) (cdr((x))->fn)
#define ptr(x) (cdr((x))->ptr)
// #define ccfunc(x) ((cfunc(cdr((x))))->fn)

// #define vptr(x) ((x)->cdr.vptr)

#define push(vm, b) ((vm)->stack[(vm)->sp++] = addr((vm), (b)))
#define pop(vm) (peek((vm), (vm)->stack[--(vm)->sp]))
#define top(vm) ((vm)->sp - (vm)->fp)
// #define stack_index(vm, i) ((vm)->)

#define peek(vm, i) (&(vm)->memory[i])
#define poke(vm, i, b) ((vm)->memory[(i)] = *(b))
#define addr(vm, b) ((int)((b) - (vm)->memory))

#define cmalloc(vm) (&(vm)->memory[(vm)->hp--])

#define start_addr sizeof(Coffee)

// static Coffee *vm;

const char *typenames[CF_TMAX] = {
  "free", "null", "true",
  "number", "string",
  "pair", "table",
  "func", "cfunc",

  "ptr",
  "symbol"
};

// static void printtype(cf_bean_t *bn) {
//   printf("type: %s\n", typenames[type(bn)]);
// }

#define aritfn(op) \
  cf_bean_t *a = cf_get(vm, 0); \
  cf_bean_t *b = cf_get(vm, 1); \
  cf_bean_t *n = cf_number(vm, a->n op b->n); \
  return n;

cf_bean_t *cf_add(Coffee *vm) { aritfn(+) }
cf_bean_t *cf_sub(Coffee *vm) { aritfn(-) }
cf_bean_t *cf_mul(Coffee *vm) { aritfn(*) }
cf_bean_t *cf_div(Coffee *vm) { aritfn(/) }

static cf_bean_t* fn_car(Coffee *vm) {
  cf_bean_t *b = cf_get(vm, 0);
  if (cf_top(vm) > 1) cf_error(vm, "'car' expects 1 argument\n");

  return cf_car(vm, b);
}

static cf_bean_t* fn_cdr(Coffee *vm) {
  cf_bean_t *b = cf_get(vm, 0);

  return cf_cdr(vm, b);
}

static cf_bean_t* fn_pair(Coffee *vm) {
  cf_bean_t *a = cf_get(vm, 0);
  cf_bean_t *b = cf_get(vm, 1);

  return cf_pair(vm, a, b);
}

static void* toptr(Coffee *vm, cf_bean_t *b);
static void* toptr(Coffee *vm, cf_bean_t *b) {
  if (isnull(b)) return 0;
  int type = type(b);

  // printf("0x%x\n", type);
  // printf("%p %s\n", b, typenames[type]);
  // printf("q\n");
  if (type != CF_TPTR && type != CF_TCFUNC) return 0;
  // printf("%p\n", b);

  long int ptr = b->next;
  long int pptr = b->i;
  pptr <<= 0x10;
  pptr |= ptr;

  // printf("%ld\n", pptr);


  return (void*)pptr;
}

static cf_bean_t *fn_list(Coffee *vm) {
  return cf_list(vm, cf_top(vm));
}

static cf_bean_t *fn_table(Coffee *vm) {
  return cf_table(vm, cf_top(vm));
}

Coffee *cf_init(void *ptr, int size) {
  CF_ASSERT(size > 0, "count must be greater than 0");
  memset(ptr, 0, size);
  Coffee *vm;
  vm = (Coffee*)ptr;
  // memset(vm, 0, sizeof(*vm));
  ptr = (char*)ptr + sizeof(*vm);
  size -= sizeof(*vm);

  // settype(&vm->sNull, CF_TNULL);
  // settype(&vm->sTrue, CF_TTRUE);


  // vm->first = &vm->sNull;
  // vm->gc = &vm->sNull;

  vm->mem_size = size / sizeof(cf_bean_t);
  vm->memory = (cf_bean_t*)ptr;


  // memset(vm->memory, 0, vm->mem_size);


  vm->sp = 0;
  vm->fp = 0;
  vm->hp = vm->mem_size-1;
  vm->sNull = cf_malloc(vm);
  vm->sTrue = cf_malloc(vm);
  printf("null: %p true: %p\n", vm->sNull, vm->sTrue);
  // for (int i = 0; i < 10; i++) vm->sym_table[i] = vm->sNull;

  settype(vm->sNull, CF_TNULL);
  settype(vm->sTrue, CF_TTRUE);
  vm->symbol_list = vm->sNull;

  cf_pushnull(vm);
  cf_setglobal(vm, "null");

  cf_push(vm, cf_true(vm));
  cf_setglobal(vm, "true");

  cf_pushcfunc(vm, cf_add);
  cf_setglobal(vm, "+");

  cf_pushcfunc(vm, cf_sub);
  cf_setglobal(vm, "-");

  cf_pushcfunc(vm, cf_mul);
  cf_setglobal(vm, "*");

  cf_pushcfunc(vm, cf_div);
  cf_setglobal(vm, "/");

  cf_pushcfunc(vm, cf_set);
  cf_setglobal(vm, "=");

  cf_pushcfunc(vm, cf_quote);
  cf_setglobal(vm, "quote");

  cf_pushcfunc(vm, cf_status);
  cf_setglobal(vm, "mem");

  cf_pushcfunc(vm, cf_print);
  cf_setglobal(vm, "print");

  cf_pushcfunc(vm, fn_car);
  cf_setglobal(vm, "car");

  cf_pushcfunc(vm, fn_cdr);
  cf_setglobal(vm, "cdr");

  cf_pushcfunc(vm, fn_pair);
  cf_setglobal(vm, "cons");

  cf_pushcfunc(vm, fn_list);
  cf_setglobal(vm, "list");

  cf_pushcfunc(vm, fn_table);
  cf_setglobal(vm, "table");

  // printf("%p: %p %p\n", b, cdr(b), f);

  return vm;
}

void cf_deinit(Coffee *vm) {
  // free(vm->memory);
}

static void printstring(Coffee *vm, cf_bean_t *b);
static void printstring(Coffee *vm, cf_bean_t *b) {
  if (b == cf_null(vm)) return;


  int len = 4;
  // for (int i = 0; i < 4; i++) 
    // if (b->str[i] == '\0') {len = i; break;}
  // while (b != cf_null(vm)) {
  // printf("%p %p %c\n", b, cf_null(vm), b->car.str[1]);
  // printf("%.*s", 3, b->car.str);
  // printf("%p %.4s\n", b, b->str);
  printf("%.*s", len, b->str);
  cf_bean_t *n = next(b);
  // printf("next: %p\n", n);
  printstring(vm, n);
    // printf("1: %p\n", b);
  // }
  // printf("%.*s", 4, b->car.str);
  // printf("ok\n");
  // printf("qqqq\n");
}

// static int tabs = 0;
// static void print_bean(Coffee *vm, cf_bean_t *bean);
// static void print_bean(Coffee *vm, cf_bean_t *bean) {
//   printf("bean: 0x%.4x <%s>", addr(vm, bean), typenames[type(bean)]);
//   char tc[tabs+1];
//   tc[tabs] = '\0';

//   if (tabs > 0) memset(tc, '\t', tabs);

//   switch(type(bean)) {
//     case CF_TNULL: break;
//     case CF_TNUMBER: printf("(%g)", bean->n); break;
//     case CF_TSTRING: {printf("(\""); printstring(vm, bean); printf("\")"); break;}
//     case CF_TCFUNC: printf("(%p)", toptr(vm, bean)); break;
//     case CF_TSYMBOL: printf("("); printstring(vm, car(bean)); printf(")"); break;
//     case CF_TPAIR: {
//       printf("%s", tc);
//       tabs++;
//       printf("({\n\t");
//       print_bean(vm, car(bean));
//       printf("\t");
//       print_bean(vm, cdr(bean));
//       printf("})");
//       tabs--;
//     }
//   }

//   printf("\n");

// }

static void print_addr(Coffee *vm, const char *type, cf_bean_t *b) {
  printf("%s: 0x%.4x", type, addr(vm, b));
}

static void print_val(Coffee *vm, cf_bean_t *b);
static void print_val(Coffee *vm, cf_bean_t *b) {
  // printf("tá indo memo?\n");
  switch(type(b)) {
    case CF_TNULL: printf("null"); break;
    case CF_TTRUE: printf("true"); break;
    case CF_TNUMBER: printf("%g", b->n); break;
    case CF_TSTRING: printstring(vm, b); break;
    case CF_TSYMBOL: printstring(vm, car(b)); break;
    case CF_TCFUNC: print_addr(vm, "cfunc", b); break;
    case CF_TPAIR: print_addr(vm, "pair", b); break;
    case CF_TTABLE: print_addr(vm, "table", b); break;
  }
}


cf_bean_t* cf_print(Coffee *vm) {
  int top = top(vm);
  // printf("testandow %d\n", top);

  int i = 0;
  while (i < top) {
    // print_bean(vm, cf_get(vm, i++));
    cf_bean_t *b = cf_get(vm, i++);
    print_val(vm, b);
    printf(" ");
    // printf("0x%.4lx\n", addr(vm, b));
    // printf("top: %d, i: %d, type: %d\n", top, i, b->tag);
    // printf("%p\n", b);
    // switch(type(b)) {
    //   case CF_TNULL: printf("null"); break;
    //   case CF_TNUMBER: printf("%g", b->n); break;
    //   case CF_TSTRING: printstring(vm, b); break;
    //   case CF_TSYMBOL: printstring(vm, car(cdr(b))); break;
    //   case CF_TPAIR: printf("pair: 0x%.4x", addr(vm, b)); break;
    // }
    // printf("\n");
    // printf("\nteste\n");
    // top = top(vm);
  }
  printf("\n");

  return cf_null(vm);
}

void cf_mark(Coffee *vm, cf_bean_t *bean) {
  if (marked(bean)) return;
  mark(bean);


  // switch(type(bean)) {
  //   case CF_TPAIR:
  //     cf_mark(vm, bean->car);
  //     cf_mark(vm, bean->cdr);

  //   case CF_TSTRING
  // }
  if (type(bean) == CF_TSTRING) cf_mark(vm, next(bean));
  if (type(bean) == CF_TPAIR || type(bean) == CF_TSYMBOL) {
    cf_mark(vm, car(bean));
    cf_mark(vm, cdr(bean));
  }
}

void cf_sweep(Coffee *vm) {
  int hp = vm->hp;

  while (hp < vm->mem_size) {
    cf_bean_t *b = peek(vm, hp);
    // printf("sweep: %p %d\n", b, marked(b));
    if (!marked(b)) b->li = 0;
    else unmark(b);

    hp++;
  }
}

void cf_gc(Coffee *vm) {
  // for (int i = 0; i < STACK_MAX; i++) cf_mark(vm, )
  mark(vm->sTrue);
  mark(vm->sNull);
  cf_mark(vm, vm->symbol_list);
  for (int i = 0; i < vm->sp; i++) {
    cf_bean_t *b = peek(vm, vm->stack[i]);
    printf("mark: %p\n", b);
    cf_mark(vm, b);
  }
  cf_sweep(vm);
}

void cf_error(Coffee *vm, const cf_string_t str) {
  fprintf(stderr, "[coffee] error: %s", str);
  exit(1);
}

cf_bean_t* cf_status(Coffee *vm) {
  float mem_size = (vm->mem_size*sizeof(cf_bean_t))/1000.f;
  float usage = (vm->mem_size - vm->hp - 1) * sizeof(cf_bean_t);
  float stack = (vm->sp) * sizeof(cf_vptr_t);
  // usage += (vm->sp * sizeof(cf_Short));
  printf("[coffee] mem usage: %gkb / %gkb [ stack: %gkb heap: %gkb ]\n", (stack+usage)/1000.f, mem_size, stack/1000.f, usage/1000.f);
  return cf_null(vm);
}

int cf_top(Coffee *vm) {
  return vm->sp - vm->fp;
}

void cf_settop(Coffee *vm, int index) {
  vm->sp = vm->fp + index;
}

cf_bean_t* cf_call(Coffee *vm, int args) {
  int fp = vm->fp;
  // int top = vm->sp;
  vm->fp = (vm->sp-1) - args;
  if (vm->fp < 0) cf_error(vm, "wrong args number\n");

  cf_bean_t *b = cf_get(vm, 0);
  // printf("%p (%s): %x %x\n", b, typenames[type(b)], b->i, b->next);
  vm->fp++;
  cf_cfunc_t fn = (cf_cfunc_t) toptr(vm, b);
  if (isnull(b)) cf_error(vm, "function cannot be null\n");
  // printf("qqqqqqqq %p\n", ptr);
  // printf("cfunc: %p call: %p %p\n", b, cf_print, fn);
  cf_bean_t *res = fn(vm);
  // printf("qqq\n");

  vm->sp = vm->fp - 1;
  vm->fp = fp;
  return res;
}

cf_bean_t *cf_quote(Coffee *vm) {
  cf_bean_t *b = cf_get(vm, 0);
  printf("%p: %d\n", b, type(b));
  return b;
}

cf_bean_t* cf_set(Coffee *vm) {
  cf_bean_t *s = cf_get(vm, 0);
  cf_bean_t *v = cf_get(vm, 1);

  // printf("%p: %d, %p: %d\n", s, type(s), v, type(v));

  setcdr(s, v);

  return s;
}

cf_bean_t *cf_get(Coffee *vm, int index) {
  if (index < 0) index = (vm->sp - vm->fp) + index;

  // printf("%d\n", vm->fp + index);
  return peek(vm, vm->stack[vm->fp + index]);
}

void cf_push(Coffee *vm, cf_bean_t *bn) {
  // int sp = (vm->sp+1) / (sizeof(cf_bean_t) / 2);
  int hp = vm->hp * sizeof(cf_bean_t);
  int sp = vm->sp * sizeof(cf_vptr_t);
  if (sp > hp)  cf_error(vm, "stack overflow\n");
  // printf("push: %p (%s)\n", bn, typenames[type(bn)]);
  push(vm, bn);
}

cf_bean_t *cf_pop(Coffee *vm) {
  if (vm->sp <= 0) cf_error(vm, "stack underflow\n");
  return pop(vm);
}

cf_bean_t* cf_setglobal(Coffee *vm, const cf_string_t name) {
  cf_bean_t *v = cf_pop(vm);

  cf_bean_t *s = cf_symbol(vm, name);
  setcdr(s, v);

  return s;
}

cf_bean_t* cf_malloc(Coffee *vm) {
  int hp = vm->hp * sizeof(cf_bean_t);
  int sp = vm->sp * sizeof(cf_vptr_t);
  if (sp > hp) cf_error(vm, "out of memory\n");

  return cmalloc(vm);
}

cf_bean_t* cf_create(Coffee *vm) {
  int i = vm->hp + 1;
  // printf("---\n");
  // printf("qqq\n");
  while (i < vm->mem_size) {
    // printf("CREATE: %p, %p\n", b, b->ptr);
    // if (b->ln == 0) {
    //   b = peek(vm, i);
    //   // printf("gc: %p\n", b);
    //   break;
    // }
    cf_bean_t *b = peek(vm, i);
    // printf("create: %p: %d\n", b, type(b));
    if (type(b) == CF_TFREE) return b;
    i++;
  }

  cf_bean_t *n = cf_malloc(vm);
  settype(n, CF_TNULL);
  // b->ln = 1;
  // printf("%p vaddr: 0x%.4lx\n", b, addr(vm, b));
  return n;

  // if (!isnull(vm->gc)) {
  //   int i = vm->hp;
  //   while (i < vm->mem_size) if 
  // } else bn = cmalloc(vm);
  // else bn = malloc(sizeof(*bn));

  // bn->next = vm->first;
  // vm->first = bn;

  // return cmalloc(vm);
}

cf_bean_t *cf_null(Coffee *vm) {
  return vm->sNull;
}

int cf_isnull(Coffee *vm, cf_bean_t *b) {
  return isnull(b);
}

cf_bean_t *cf_true(Coffee *vm) {
  return vm->sTrue;
}

cf_bean_t *cf_number(Coffee *vm, cf_number_t n) {
  cf_bean_t *bn = cf_create(vm);
  // printf("%p %g %d\n",bn, bn->ln, bn->ln == 0);
  settype(bn, CF_TNUMBER);
  number(bn) = n;
  return bn;
}

static cf_bean_t* buildstring(Coffee *vm, const cf_string_t str);
static cf_bean_t* buildstring(Coffee *vm, const cf_string_t str) {
  if (!str) return cf_null(vm);
  if (*str == '\0') return cf_null(vm);
  // printf("%s\n", str);
  cf_string_t p = str;
  int len = strlen(str);

  cf_bean_t *s = cf_create(vm);
  s->li = 0;
  // printf("ptr: %p\n", s);
  settype(s, CF_TSTRING);

  int sz = len > 4 ? 4 : len;
  // printf("%p %.*s\n", s, sz, str);
  // if (len >= 4) memcpy(s->car.str, p, 4);
  // else memcpy(s->car.str, p, len);
  // p += (len % 4) + 1;
  // if (*str == '\0') sz = 1;
  memcpy(s->str, p, sz);
  p += sz;
  // printf("%s\n", str);
  // str

  // printf("%f\n", s->car.n);


  // int i = 0;
  // while (i < 4) s->car.str[i++] = *(p++);

  // setcdr(s, buildstring(vm, p));
  cf_bean_t *n = buildstring(vm, p);
  // printf("next: %p\n", n);
  setnext(s, n);
  // printf("%.4s, %p %p %p\n", s->car.str, s, cdr(s), cf_null(vm));
  // s->cdr.vptr =;
  // setcdr(s, cf_null(vm));
  // printf("len: %d\n", len);
  // if (len <= 4) return s;
  // cf_bean_t *ex = buildstring(vm, p);

  // setcdr(s, ex);
  return s; 
}

cf_bean_t *cf_string(Coffee *vm, const cf_string_t str) {
  // cf_bean_t *bn = cf_create(vm);
  // settype(bn, CF_TSTRING);
  // int len = strlen(str);
  // string(bn) = malloc(len + 1);
  // strcpy(string(bn), str);
  // string(bn)[len] = '\0';
  // cf_bean_t *bn = cf_create(vm);
  // setcdr(bn, b)
  // settype(bn, CF_TSTRING);
  // setnext(bn, buildstring(vm, str));
  // printf("string: %p %p\n", bn, cdr(bn));
  // printf("q: %s\n", cdr(bn)->car.str);

  return buildstring(vm, str);
}

cf_bean_t *cf_bool(Coffee *vm, cf_bool_t b) {
  return b ? vm->sTrue : vm->sNull;
}

// cf_bean_t* cf_cons(Coffee *vm, cf_bean_t *a, cf_bean_t *b) {
//   cf_bean_t *bn = cf_create(vm);
//   // settype()
//   bn->car.vptr = addr(vm, a);
//   bn->cdr.vptr = addr(vm, b);

//   return bn;
// }

// #defin cf_cons()

cf_bean_t *cf_pair(Coffee *vm, cf_bean_t *a, cf_bean_t *b) {
  cf_bean_t *bn = cf_create(vm);

  settype(bn, CF_TPAIR);
  // cf_bean_t *vptr = cf_create(vm);
  setcar(bn, a);
  setcdr(bn, b);

  // setcdr(bn, vptr);
  // setcar(vptr, a);
  // setcdr(vptr, b);
  // bn->cdr.vptr = addr(vm, b);
  // cdr(bn) = cf_cons(vm, a, b);

  return bn;
}

cf_bean_t* cf_table(Coffee *vm, int sz) {
  cf_bean_t *b = cf_create(vm);
  settype(b, CF_TTABLE);
  setcar(b, cf_null(vm));

  cf_bean_t *bn = cf_list(vm, sz);
  setcdr(b, bn);

  return b;
}

cf_bean_t *cf_list(Coffee *vm, int sz) {
  cf_bean_t *bn = vm->sNull;

  while (sz > 0) {
    cf_bean_t *p = cf_pop(vm);
    // printf("%p: %s\n", p, typenames[type(p)]);
    // print_addr(vm, typenames[type(p)], p);
    // printf("\n");
    bn = cf_cons(vm, p, bn);
    sz--;
  }

  // while (len--) {
  //   bn = cf_cons(vm, el[len], bn);
  // }

  return bn;
}

cf_bean_t *cf_car(Coffee *vm, cf_bean_t *bn) {
  if (isnull(bn)) return bn;
  return car(bn);
}

cf_bean_t *cf_cdr(Coffee *vm, cf_bean_t *bn) {
  if (isnull(bn)) return bn;
  // printf("%p -> %p\n", bn, car(bn));
  return cdr(bn);
}

cf_bean_t *cf_cfunc(Coffee *vm, cf_cfunc_t fn) {
  if (!fn) return cf_null(vm);
  cf_bean_t *bn = cf_create(vm);
  settype(bn, CF_TCFUNC);

  long int iptr = (long int)fn;
  // cf_vptr_t l = iptr & 0xffff;
  // int h = iptr >> 16;
  bn->next = iptr & 0xffff;
  bn->i = iptr >> 0x10;

  // printf("%d\n", type(bn));

  // printf("%p %lx\n", fn, iptr);
  // printf("%x %x\n", bn->next, bn->i);
  // printf("%p tp: %x\n", bn, bn->i);
  // cf_bean_t *ex = cf_cfunc(vm, (cf_cfunc_t)(iptr >> 0x20));
  // bn->next = addr(vm, ex);
  // setnext(bn, ex);

  // setcdr(bn, cf_create(vm));
  // cf_bean_t *c = cdr(bn);
  // c->fn = fn;

  // printf("fn: %p %p\n", c->fn, fn);
  // setcdr(bn, c);
  // printf("fn: %p %p\n", cfunc(cdr(bn)), fn);

  return bn;
}

cf_bean_t *cf_ptr(Coffee *vm, cf_ptr_t pptr) {
  if (!pptr) return cf_null(vm);
  cf_bean_t *bn = cf_create(vm);

  // int iptr = (int)pptr;
  // cf_Short ptr1 = (iptr & 0xffff0000) >> 0x10;
  // cf_Short ptr0 = iptr & 0xffff;

  settype(bn, CF_TPTR);

  // bn->cdr = 
  long int iptr = (long int)pptr;
  bn->next = iptr & 0xffff;
  bn->i = iptr >> 0x10;
  // bn->i = iptr & 0xffffffff;

  // cf_bean_t *ex = cf_ptr(vm, (cf_ptr_t)(iptr >> 0x20));
  // setnext(bn, ex);
  
  // cf_bean_t *p = cf_create(vm);
  // // p->car.n = ptr1;
  // // p->cdr.n = ptr0;
  // // p->ptr = pptr;
  // p->car.vptr1 =  

  // setcdr(bn, p);

  return bn;
}

static int streq(Coffee *vm, cf_bean_t *bstr, const cf_string_t str) {
  // Coffee *vm = &vm;
  int i = 0;
  // printf("len %d\n", 1);
  int len = strlen(str);
  // printf("%s\n", str);
  char *p = str;
  while (*p != '\0' && bstr != cf_null(vm)) {
    // printf("%c %c\n", );
    // printf("%c %c\n", bstr->str[i % 4], *p);
    // if (bstr->str[i % 4] != *p) return 0;
    for (i = 0; i < 4 && *p != '\0'; i++, p++) {
      if (bstr->str[i % 4] != *p) return 0;
    }
    bstr = next(bstr);
    // p++;
    // if (strlen(p) < 4) return 0;
    // p += 4;
  }

  return 1;
  // return !strcmp(bstr->str, str);
}

cf_bean_t* cf_symbol(Coffee *vm, const cf_string_t str) {
  CF_ASSERT(str != NULL, "symbol name cannot be null");

	cf_bean_t *symbol = vm->symbol_list;
  // printf("symbol list: %p\n", symbol);
	while (symbol != cf_null(vm)) {
    cf_bean_t *s = car(symbol);
    // printstring(vm, car(s));
    // printf("\n");
		if (streq(vm, car(s), str)) return s;
		symbol = cdr(symbol);
	}

	symbol = vm->symbol_list;

	cf_bean_t *bean = cf_create(vm);
  settype(bean, CF_TSYMBOL);
	// // int len = strlen(name);
	cf_bean_t *nm = cf_string(vm, (cf_string_t)str);
	// setcar(bean, nm);
	// cf_bean_t *p = cf_cons(vm, nm, cf_null(vm));

  setcar(bean, nm);
  setcdr(bean, vm->sNull);

	// cdr(bean) = cf_cons(vm, nm, &s_null);

	// bean->next = symbol;
	vm->symbol_list = cf_cons(vm, bean, symbol);

	return bean;
}

cf_bean_t * cf_optbean(Coffee *vm, int index, cf_bean_t *opt) {
  cf_bean_t *b = cf_get(vm, index);
  return b;
}

cf_number_t cf_optnumber(Coffee *vm, int index, cf_number_t opt) {
  cf_bean_t *b = cf_get(vm, index);
  if (type(b) != CF_TNUMBER) return opt;
  return b->n;
}

const cf_string_t cf_optstring(Coffee *vm, int index, cf_string_t opt) {
  cf_bean_t *b = cf_get(vm, index);
  if (type(b) != CF_TSTRING) return opt;
  // return 
  return NULL;
}

cf_bean_t* cf_optsymbol(Coffee *vm, int index, cf_bean_t *bn) {
  return bn;
}

cf_bool_t cf_optbool(Coffee *vm, int index, cf_bool_t opt) {
  cf_bean_t *b = cf_get(vm, index);
  return isnull(b) ? 0 : 1;
}

cf_cfunc_t cf_optcfunc(Coffee *vm, int index, cf_cfunc_t opt) {
  cf_bean_t *b = cf_get(vm, index);
  if (type(b) != CF_TCFUNC) return opt;
  cf_cfunc_t f = (cf_cfunc_t)toptr(vm, b);

  return f;
}

cf_ptr_t cf_optptr(Coffee *vm, int index, cf_ptr_t opt) {
  cf_bean_t *b = cf_get(vm, index);
  if (type(b) != CF_TPTR) return opt;
  cf_ptr_t ptr = (cf_ptr_t)toptr(vm, b);

  return ptr;
}

// REPL

#define next_char(c) ((c)[1])

static int is_whitespace(char p) {
  switch(p) {
    case ' ': return 1;
    case '\n': return 1;
    case '\t': return 1;
    case '\r': return 1;
  }

  return 0;
}

static int isnum(char p) {
  return p >= '0' && p <= '9';
}

static int isalpha(char p) {
  return (p >= 'a' && p <= 'z') || (p >= 'A' && p <= 'Z');
}

static cf_bean_t *read_number(Coffee *vm, const char **str) {
  char *p = (char*)*str;
  int mul = 1;

  if (*p == '-') {
    mul = -1;
    p++;
    if (!isnum(*p)) cf_error(vm, "expected number after '-'\n");
  }

  float n = strtof(p, NULL);
  while (isnum(*p) || *p == '.') p++;
  *str = p;

  return cf_number(vm, n);
}

static cf_bean_t *read_string(Coffee *vm, const char **str) {
  char *p = (char*)*str;

  int len = 0;
  while (*p != '"') p++;
  len = p - *str;

  char buf[len+1];
  sprintf(buf, "%.*s", len, *str);
  buf[len] = '\0';

  // printf("buf: %s\n", buf);
  *str = p+1;

  return cf_string(vm, buf);
}

static cf_bean_t *read_symbol(Coffee *vm, const char **str) {
  char *p = (char*)*str;

  int len = 0;
  while (!is_whitespace(*p) && *p != ')') p++;
  len = p - *str;
  char buf[len+1];
  sprintf(buf, "%.*s", len, *str);

  buf[len] = '\0';
  // printf("symbol: %s\n", buf);

  *str = p;

  /*switch(*buf) {
    case 'p': return cf_;
    case 'm': return cf_cfunc(vm, cf_status);
    case 'l': return cf_cfunc(vm, make_list);
  }*/

  return cf_symbol(vm, buf);
}

static cf_bean_t *read(Coffee *vm, const char **str);
static cf_bean_t *read(Coffee *vm, const char **str) {
  char *p = (char*)*str;
  // p++;
  while(is_whitespace(*p)) p++;

  *str = p;

  if (isalpha(*p)) return read_symbol(vm, str);
  // printf("q\n");

  // printf("token: %s\n", p);
  if (*p == '-' && isnum(next_char(p)))  return read_number(vm, str);
  else if (isnum(*p)) return read_number(vm, str);

  *str = p+1;
  switch(*p) {
  case '(': {
    int args = 0;
    int top = cf_top(vm);
    // printf("%d\n", top);
    while (*p != ')') {
      // printf("%d %d\n", top, cf_top(vm));
      if (top != cf_top(vm)) args++;
      if (*p == '\0') cf_error(vm, "missing ')'\n");
      cf_bean_t *b = read(vm, str);
      // printf("%c ", *p);
      p = (char*)*str;
      // printf("%p\n", b);
      cf_push(vm, b);
      while(is_whitespace(*p)) p++;
    }
    // printf("%d\n", args);
    return cf_list(vm, args+1);
  }
    // case ')': return cf_call(vm, 2);
  case '\'': return cf_symbol(vm, "quote");
  case '+': return cf_symbol(vm, "+");
  case '-': return cf_symbol(vm, "-");
  case '*': return cf_symbol(vm, "*");
  case '/': return cf_symbol(vm, "/");
  case 'p': return cf_symbol(vm, "print");
  case '"': return read_string(vm, str);
  case ';': while (**str != '\n') (*str)++;
  case '=': return cf_symbol(vm, "=");
  }

  // printf("testandow\n");

  return cf_null(vm);
}

cf_bean_t* cf_read(Coffee *vm, const char *str) {
  // cf_error(vm, "string cannot be null\n");
  cf_bean_t *b = cf_null(vm);

  int len = 0;
  while (*str != '\0') {
    // printf("%c\n", *str);
    // if (isnum(*p)) {
    //   char *n = p;
    //   while (isnum(*p)) p++;
    // }
    cf_bean_t *bn = read(vm, &str);
    cf_push(vm, bn);
    len++;
    // cf_status(vm);
    // printf("%p\n", bn);
    // str++;
  }


  // b = cf_pop(vm);


  return cf_list(vm, len);
}
