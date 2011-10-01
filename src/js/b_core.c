/*
 * Core builtins for the JavaScript VM.
 * Copyright (c) 1998 New Generation Software (NGS) Oy
 *
 * Author: Markku Rossi <mtr@ngs.fi>
 */

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

/*
 * $Source: /usr/local/cvsroot/ngs/js/src/b_core.c,v $
 * $Id: b_core.c,v 1.4 1998/09/25 08:41:51 mtr Exp $
 */

/*
 * Global methods:
 *
 *  parseInt (string[, radix])
 *  parseFloat (string)
 *  escape (string)
 *  unescape (string)
 *  isNaN (any)
 *  isFinite (any)
 *  debug (any)
 *  error (string)
 *  float (any)
 *  int (any)
 *  isFloat (any)
 *  isInt (any)
 *  print (any[,...])
 */

#include "jsint.h"

prog_char b_core_string_0[] PROGMEM = "parseInt(): illegal amount of arguments";
prog_char b_core_string_1[] PROGMEM =
        "parseFloat(): illegal amount of arguments";
prog_char b_core_string_2[] PROGMEM = "escape(): illegal amount of arguments";
prog_char b_core_string_3[] PROGMEM = "unescape(): illegal amount of arguments";
prog_char b_core_string_4[] PROGMEM = "isNaN(): illegal amount of arguments";
prog_char b_core_string_5[] PROGMEM = "isFinite(): illegal amount of arguments";


/*
 * Types and definitions.
 */

#define EMIT_TO_RESULT(c)						\
  do {									\
    result_return->u.vstring->data =					\
    js_vm_realloc (vm, result_return->u.vstring->data,			\
		   result_return->u.vstring->len + 1);			\
   result_return->u.vstring->data[result_return->u.vstring->len] = (c); \
   result_return->u.vstring->len += 1;					\
 } while (0)


/*
 * Static functions.
 */

static void
parseInt_global_method (JSVirtualMachine *vm, JSBuiltinInfo *builtin_info,
			void *instance_context, JSNode *result_return,
			JSNode *args)
{
  JSInt32 base = 0;
  char *cp, *end;

  result_return->type = JS_INTEGER;

  if (args->u.vinteger != 1 && args->u.vinteger != 2)
    {
      sprintf_P (vm->error, b_core_string_0);
      js_vm_error (vm);
    }
  if (args[1].type == JS_STRING)
    cp = js_string_to_c_string (vm, &args[1]);
  else
    {
      JSNode input;

      /* Convert the input to string. */
      js_vm_to_string (vm, &args[1], &input);
      cp = js_string_to_c_string (vm, &input);
    }
  if (args->u.vinteger == 2)
    {
      if (args[2].type == JS_INTEGER)
	base = args[2].u.vinteger;
      else
	base = js_vm_to_int32 (vm, &args[2]);
    }

  result_return->u.vinteger = strtol (cp, &end, base);
  js_free (cp);

  if (cp == end)
    result_return->type = JS_NAN;
}


static void
parseFloat_global_method (JSVirtualMachine *vm, JSBuiltinInfo *builtin_info,
			  void *instance_context, JSNode *result_return,
			  JSNode *args)
{
  char *cp, *end;

  result_return->type = JS_FLOAT;

  if (args->u.vinteger != 1)
    {
      sprintf_P (vm->error, b_core_string_1);
      js_vm_error (vm);
    }
  if (args[1].type == JS_STRING)
    cp = js_string_to_c_string (vm, &args[1]);
  else
    {
      JSNode input;

      /* Convert the input to string. */
      js_vm_to_string (vm, &args[1], &input);
      cp = js_string_to_c_string (vm, &input);
    }

  result_return->u.vfloat = strtod (cp, &end);
  js_free (cp);

  if (cp == end)
    /* Couldn't parse, return NaN. */
    result_return->type = JS_NAN;
}


static void
escape_global_method (JSVirtualMachine *vm, JSBuiltinInfo *builtin_info,
		      void *instance_context, JSNode *result_return,
		      JSNode *args)
{
  unsigned char *dp;
  unsigned int n, i;
  JSNode *source;
  JSNode source_n;
  static const prog_char charset[] PROGMEM =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@*_+-./";

  if (args->u.vinteger != 1)
    {
      sprintf (vm->error, b_core_string_2);
      js_vm_error (vm);
    }
  if (args[1].type == JS_STRING)
    source = &args[1];
  else
    {
      /* Convert the argument to string. */
      js_vm_to_string (vm, &args[1], &source_n);
      source = &source_n;
    }

  /*
   * Allocate the result string, Let's guess that we need at least
   * <source->u.vstring->len> bytes of data.
   */
  n = source->u.vstring->len;
  dp = source->u.vstring->data;
  js_vm_make_string (vm, result_return, NULL, n);
  result_return->u.vstring->len = 0;

  /*
   * Scan for characters requiring escapes.
   */
  for (i = 0; i < n; i += 1)
    {
      unsigned int c = dp[i];

      if (strchr_P (charset, c))
	EMIT_TO_RESULT (c);
      else if (c > 0xFF)
	{
	  unsigned char buf[6];

	  sprintf (buf, "%04x", c);
	  EMIT_TO_RESULT ('%');
	  EMIT_TO_RESULT ('u');
	  EMIT_TO_RESULT (buf[0]);
	  EMIT_TO_RESULT (buf[1]);
	  EMIT_TO_RESULT (buf[2]);
	  EMIT_TO_RESULT (buf[3]);
      }
    else
      {
	unsigned char buf[4];
	sprintf (buf, "%02x", c);

	EMIT_TO_RESULT ('%');
	EMIT_TO_RESULT (buf[0]);
	EMIT_TO_RESULT (buf[1]);
      }
    }
}

/* A helper function for unescape(). */
static int
scanhexdigits (unsigned char *dp, int nd, unsigned int *cp)
{
  static const prog_char digits[] PROGMEM = "0123456789abcdefABCDEF";
  int i;
  unsigned int d;

  *cp = 0;
  for (i = 0; i < nd; i += 1)
    {
      d = strchr_P (digits, dp[i]) - digits;
      if (d < 16)
	;
      else if (d < 22)
	d -= 6;
      else
	return 0;

      *cp <<= 4;
      *cp += d;
    }

  return 1;
}


static void
unescape_global_method (JSVirtualMachine *vm, JSBuiltinInfo *builtin_info,
			void *instance_context, JSNode *result_return,
			JSNode *args)
{
  unsigned char *dp;
  unsigned int n, i;
  JSNode *source;
  JSNode source_n;

  if (args->u.vinteger != 1)
    {
      sprintf_P (vm->error, b_core_string_3);
      js_vm_error (vm);
    }
  if (args[1].type == JS_STRING)
    source = &args[1];
  else
    {
      js_vm_to_string (vm, &args[1], &source_n);
      source = &source_n;
    }

  /*
   * Allocate the result string, Let's guess that we need at least
   * <source->u.vstring->len> bytes of data.
   */
  n = source->u.vstring->len;
  dp = source->u.vstring->data;
  js_vm_make_string (vm, result_return, NULL, n);
  result_return->u.vstring->len = 0;

  /*
   * Scan for escapes requiring characters.
   */
  for (i = 0; i < n;)
    {
      unsigned int c = dp[i];

      if (c != '%')
	i += 1;
      else if (i <= n - 6 && dp[i + 1] == 'u'
	       && scanhexdigits (dp + i + 2, 4, &c))
	i += 6;
      else if (i <= n - 3 && scanhexdigits (dp + i + 1, 2, &c))
	i += 3;
      else
	{
	  c = dp[i];
	  i += 1;
	}
      EMIT_TO_RESULT (c);
    }
}


static void
isNaN_global_method (JSVirtualMachine *vm, JSBuiltinInfo *builtin_info,
		     void *instance_context, JSNode *result_return,
		     JSNode *args)
{
  JSNode cvt;
  int result;

  if (args->u.vinteger != 1)
    {
      sprintf (vm->error, b_core_string_4);
      js_vm_error (vm);
    }

  switch (args[1].type)
    {
    case JS_NAN:
      result = 1;
      break;

    case JS_INTEGER:
    case JS_FLOAT:
      result = 0;
      break;

    default:
      js_vm_to_number (vm, &args[1], &cvt);
      result = cvt.type == JS_NAN;
      break;
    }

  result_return->type = JS_BOOLEAN;
  result_return->u.vboolean = result;
}


static void
isFinite_global_method (JSVirtualMachine *vm, JSBuiltinInfo *builtin_info,
			void *instance_context, JSNode *result_return,
			JSNode *args)
{
  JSNode *source;
  JSNode cvt;
  int result;

  if (args->u.vinteger != 1)
    {
      sprintf_P (vm->error, b_core_string_5);
      js_vm_error (vm);
    }

  if (args[1].type == JS_NAN || args[1].type == JS_INTEGER
      || args[1].type == JS_FLOAT)
    source = &args[1];
  else
    {
      js_vm_to_number (vm, &args[1], &cvt);
      source = &cvt;
    }

  switch (source->type)
    {
    case JS_NAN:
      result = 0;
      break;

    case JS_INTEGER:
      result = 1;
      break;

    case JS_FLOAT:
      if (JS_IS_POSITIVE_INFINITY (&args[1])
	  || JS_IS_NEGATIVE_INFINITY (&args[1]))
	result = 0;
      else
	result = 1;
      break;

    default:
      /* NOTREACHED */
      result = 0;
      break;
    }

  result_return->type = JS_BOOLEAN;
  result_return->u.vboolean = result;
}


/*
 * Global functions.
 */

static struct
{
  char *name;
  JSBuiltinGlobalMethod method;
} global_methods[] =
{
  {"parseInt",		parseInt_global_method},
  {"parseFloat",	parseFloat_global_method},
  {"escape",		escape_global_method},
  {"unescape",		unescape_global_method},
  {"isNaN",		isNaN_global_method},
  {"isFinite",		isFinite_global_method},

  {NULL, NULL},
};


void
js_builtin_core (JSVirtualMachine *vm)
{
  int i;
  JSNode *n;

  /* Properties. */

  n = &vm->globals[js_vm_intern (vm, "NaN")];
  n->type = JS_NAN;

  n = &vm->globals[js_vm_intern (vm, "Infinity")];
  JS_MAKE_POSITIVE_INFINITY (n);

  /* Global methods. */
  for (i = 0; global_methods[i].name; i++)
    {
      JSBuiltinInfo *info;

      info = js_vm_builtin_info_create (vm);
      info->global_method_proc = global_methods[i].method;

      n = &vm->globals[js_vm_intern (vm, global_methods[i].name)];
      js_vm_builtin_create (vm, n, info, NULL);
    }
}
