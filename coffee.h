#ifndef COFFEE_H
#define COFFEE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COFFEE_VERSION "0.1.0"
#define CF_VERSION COFFEE_VERSION

// #define OBJ_MAX 16000

#define STR(x) #x
#define CF_ASSERT(expr, error_msg) if(!(expr)) { \
  fprintf(stderr, "[coffee] assertion '%s' failed, %s\n", STR(expr), error_msg); \
  exit(1); \
}

#define CF_API

#define cf_foreach(vm, el, iter) for ((el) = cf_car((vm), (iter)); (iter) != cf_null((vm)); (iter) = cf_cdr((vm), (iter)), (el) = cf_car((vm), (iter)))

#define MARK_MASK 0x80
#define TYPE_MASK 0x7f

#define STACK_MAX 256

typedef struct Coffee Coffee;
typedef struct cf_bean_s cf_bean_t;
typedef cf_bean_t*(*cf_cfunc_t)(Coffee*);

typedef float cf_number_t;
typedef double cf_double_t;

typedef unsigned short cf_short_t;
typedef unsigned short cf_vptr_t;
typedef int cf_int_t;
typedef unsigned int cf_uint_t;
typedef long int cf_lint_t;
typedef void* cf_ptr_t;
typedef char* cf_string_t;
typedef char cf_bool_t;
typedef unsigned char cf_byte_t;

// co_short_t st;
// co_byte_t byte;
// co_string_t str;

typedef unsigned short cf_uint16;
typedef unsigned int cf_uint32;
typedef unsigned char cf_uint8;

typedef enum {
  OP_RET = 0, // ret
  OP_REQ,     // req
  OP_PRINT,   // prn
  OP_ADD,     // +
  OP_SUB,     // -
  OP_MUL,     // *
  OP_DIV      // /
} OPCODE;

typedef struct cf_OpCode {
  const char *name;
  cf_byte_t args;
  cf_cfunc_t fn;
} cf_OpCode;

CF_API cf_bean_t *cf_ret(Coffee *vm);
CF_API cf_bean_t *cf_req(Coffee *vm);
// CF_API cf_bean_t *cf_print(Coffee *vm);
CF_API cf_bean_t *cf_print(Coffee *vm);
CF_API cf_bean_t *cf_add(Coffee *vm);
CF_API cf_bean_t *cf_sub(Coffee *vm);
CF_API cf_bean_t *cf_mul(Coffee *vm);
CF_API cf_bean_t *cf_div(Coffee *vm);

CF_API cf_bean_t *cf_quote(Coffee *vm);

static cf_OpCode opcodes[] = {
  {"ret", 1, NULL},
  {"req", 1, NULL},
  {"prn", -1, cf_print},
  {"+", 2, cf_add},
  {"-", 2, cf_sub},
  {"*", 2, cf_mul},
  {"/", 2, cf_div}
};

// typedef union cf_value_t {
//   cf_byte_t header;
//   cf_byte_t b;
//   cf_number_t n;
//   cf_string_t str;
//   cf_ptr_t ptr;
//   cf_cfunc_t fn;
//   cf_bean_t *bn;
// } cf_value_t;

// typedef union cf_value_t {
//   cf_byte_t b;
//   struct {
//     cf_byte_t p, q, r, s;
//   };
//   cf_byte_t str[4];
//   cf_number_t n;
//   cf_uint_t vptr;


//   // cf_string_t
// } cf_value_t;

// co_value_t val;

typedef union cf_value_t {
  cf_byte_t b;
  cf_vptr_t vptr;
  cf_uint_t tag;
  cf_number_t n;
  cf_uint_t i;
  cf_byte_t str[4];
  struct {
    cf_vptr_t vptr0;
    cf_vptr_t vptr1;
  };
  // struct
} cf_value_t;

struct cf_bean_s {
  union {
    struct {
      cf_byte_t tag;
      cf_vptr_t next;
      union {
        cf_byte_t byte;
        cf_number_t n;
        cf_uint_t i;
        cf_byte_t str[4];
        struct {
          cf_vptr_t car;
          cf_vptr_t cdr;
        };
      };
    };

    cf_double_t ln;
    cf_lint_t li;
  };
};

// co_bean_t bean;

// struct cf_bean_t {

//   union {
//     struct {

//       union {
//         cf_byte_t tag;
//         cf_value_t car;
//         struct {
//           cf_vptr_t _;
//           cf_vptr_t vptr;
//         }
//       };

//       union {
//         cf_byte_t b;
//         cf_number_t n;
//         cf_value_t cdr;
//       };
//     };

//     // cf_string_t str;
//     cf_bean_t *bn;
//     cf_ptr_t ptr;
//     cf_cfunc_t fn;
//     cf_double_t ln;
//   };
//   // union {
//   //   cf_byte_t tag;
//   //   struct {
//   //     cf_value_t car;
//   //     cf_value_t cdr;
//   //   };
//   //   cf_number_t n;
//   //   cf_string_t str;
//   //   cf_ptr_t ptr;
//   //   cf_cfunc_t fn;
//   //   cf_double_t ln;
//   // };
//   // union {
//   //   struct {
//   //     union {
//   //       cf_value_t tag;
//   //       cf_value_t car;
//   //     };
//   //     cf_value_t cdr;
//   //   };
//   //   cf_cfunc_t fn;
//   //   cf_ptr_t ptr;
//   //   cf_double_t n;
//   //   cf_string_t str;
//   // };
// };

// struct cf_bean_t {
//   cf_byte_t header; // 1bit mark + 7bit type
//   cf_byte_t s[3];
//   // union {
//   //   cf_byte_t b;
//   //   cf_number_t n;
//   //   cf_string_t str;
//   //   cf_ptr_t ptr;
//   //   cf_cfunc_t fn;
//   //   struct {
//   //     cf_bean_t *car;
//   //     cf_bean_t *cdr;
//   //   };
//   // };
//   union {
//     struct {
//       cf_value_t car;
//       cf_value_t cdr;
//     };
//     cf_ptr_t ptr;
//     cf_cfunc_t fn;
//     cf_double_t n;
//     cf_string_t str;
//     // void **pptr;
//     // cf_byte_t data[8];
//     // cf_short_t d[4];
//     // cf_lint_t ptr;
//     // void* ptr;
//   };

//   // cf_bean_t *next;
// };

typedef enum {
  CF_TFREE = 0,
  CF_TNULL, CF_TTRUE,
  CF_TNUMBER, CF_TSTRING,
  CF_TPAIR, CF_TTABLE,
  CF_TFUNC, CF_TCFUNC,
  CF_TPTR,

  CF_TSYMBOL,
  CF_TMAX
} CF_TYPE;

struct cf_HashTable {
  cf_bean_t *nodes[10];
};

struct Coffee {
  cf_bean_t *sNull;
  cf_bean_t *sTrue;

  unsigned int sp; // stack pointer
  unsigned int hp; // heap pointer

  unsigned int fp;

  // cf_bean_t stack[STACK_MAX];
  union {
    cf_short_t *stack;
    cf_bean_t *memory;
  };
  int mem_size;

  // cf_bean_t *first;
  // cf_bean_t *gc;
  cf_bean_t *symbol_list;
  // cf_bean_t *sym_table[10];

  // symbols

  // cf_byte_t *code;
};

// cf_bean_t sNull;
// cf_bean_t sTrue;

#if defined(__cplusplus)
extern "C" {
#endif

#define tonumber(bean) (bean)->n
#define tostring(bean) (bean)->str
#define tobool(bean) 
#define tocfunc(vm, b) (cf_cdr(vm, b)->fn);

// Coffee* co_init(void *ptr, int size);
// int cof_deinit(Coffee *vm);
// cfe_mark(Coffee *vm, co_bean_t *bean);
// int co_sweep(Coffee *vm);
// 

CF_API Coffee* cf_init(void *ptr, int size);
CF_API void cf_deinit(Coffee *vm);

CF_API void cf_mark(Coffee *vm, cf_bean_t *bean);
CF_API void cf_sweep(Coffee *vm);
CF_API void cf_gc(Coffee *vm);

CF_API void cf_error(Coffee *vm, const cf_string_t error);
CF_API cf_bean_t* cf_status(Coffee *vm);

CF_API int cf_top(Coffee *vm);
CF_API void cf_settop(Coffee *vm, int index);

CF_API cf_bean_t* cf_call(Coffee *vm, int args);


CF_API cf_bean_t* cf_set(Coffee *vm);
CF_API cf_bean_t* cf_get(Coffee *vm, int index);
#define cf_getnumber(vm, index) tonumber(cf_get((vm), (index)))

CF_API void cf_push(Coffee *vm, cf_bean_t *bean);
#define cf_pushnumber(vm, n) cf_push((vm), cf_number((vm), (n)))
#define cf_pushstring(vm, str) cf_push((vm), cf_string((vm), (str)))
#define cf_pushsymbol(vm, str) cf_push((vm), cf_symbol((vm), (str)))
#define cf_pushbool(vm, b) cf_push((vm), cf_bool((vm), (b)))
#define cf_pushcfunc(vm, cfn) cf_push((vm), cf_cfunc((vm), (cfn)))
#define cf_pushptr(vm, ptr) cf_push((vm), cf_ptr((vm), (ptr)))
#define cf_pushnull(vm) cf_push((vm), cf_null((vm)))

CF_API cf_bean_t* cf_pop(Coffee *vm);
#define cf_popnumber(vm) tonumber(cf_pop((vm)))

// pop a value from the stack and set as a global variable
CF_API cf_bean_t* cf_setglobal(Coffee *vm, const cf_string_t name);

// Check for a free object or malloc new
// Threw an error if is out of memory (hp < sp)
CF_API cf_bean_t* cf_create(Coffee *vm);

// Alloc a new object at heap (hp++)
CF_API cf_bean_t* cf_malloc(Coffee *vm);

// return the global null object
CF_API cf_bean_t* cf_null(Coffee *vm);
// return the global true object
CF_API cf_bean_t* cf_true(Coffee *vm);
// return a new number
CF_API cf_bean_t* cf_number(Coffee *vm, cf_number_t n);
// return a new string
CF_API cf_bean_t* cf_string(Coffee *vm, cf_string_t str);
// return global true or null
CF_API cf_bean_t* cf_bool(Coffee *vm, cf_bool_t b);
// return a new pair or 'a' and 'b'
CF_API cf_bean_t* cf_pair(Coffee *vm, cf_bean_t *a, cf_bean_t *b);
#define cf_cons(vm, a, b) cf_pair(vm, a, b)
// return a new table
CF_API cf_bean_t* cf_table(Coffee *vm, int sz);

CF_API cf_bean_t* cf_list(Coffee *vm, int sz);
// return a new cfunc
CF_API cf_bean_t* cf_cfunc(Coffee *vm, cf_cfunc_t fn);
// return a new ptr
CF_API cf_bean_t* cf_ptr(Coffee *vm, cf_ptr_t ptr);
// return a symbol (check in symbol list first)
CF_API cf_bean_t* cf_symbol(Coffee *vm, const cf_string_t str);


// return car (object at b->car position)
CF_API cf_bean_t* cf_car(Coffee *vm, cf_bean_t *b);
// return car (object at b->cdr position)
CF_API cf_bean_t* cf_cdr(Coffee *vm, cf_bean_t *b);

#define cf_tobean(vm, index) cf_optbean((vm), (index), cf_null((vm)))
#define cf_tonumber(vm, index) cf_optnumber((vm), (index), -1)
#define cf_tostring(vm, index) cf_optstring((vm), (index), NULL)
#define cf_tosymbol(vm, index) cf_optsymbol((vm), (index), cf_null((vm)))
#define cf_tobool(vm, index) cf_optbool((vm), (index), -1)
#define cf_tocfunc(vm, index) cf_optcfunc((vm), (index), NULL)
#define cf_toptr(vm, index) cf_optptr((vm), (index), NULL)

CF_API cf_bean_t* cf_optbean(Coffee *vm, int index, cf_bean_t *opt);
CF_API cf_number_t cf_optnumber(Coffee *vm, int index, cf_number_t opt);
CF_API const cf_string_t cf_optstring(Coffee *vm, int index, cf_string_t opt);
CF_API cf_bean_t* cf_optsymbol(Coffee *vm, int index, cf_bean_t *bn);
CF_API cf_bool_t cf_optbool(Coffee *vm, int index, cf_bool_t opt);

CF_API cf_cfunc_t cf_optcfunc(Coffee *vm, int index, cf_cfunc_t opt);
CF_API cf_ptr_t cf_optptr(Coffee *vm, int index, cf_ptr_t opt);

// REPL

CF_API cf_bean_t* cf_read(Coffee *vm, const char *str);
CF_API cf_bean_t* cf_eval(Coffee *vm);

#if defined(__cplusplus)
}
#endif

#endif
