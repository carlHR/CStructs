#ifndef _CDATASTRUCTS_STRING_ENGINE_
#define _CDATASTRUCTS_STRING_ENGINE_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <regex.h>

#include "memory.h"
#include "array.h"
#include "varray.h"

#define STRING_ENGINE_INVALID_INDEX ((size_t) -1)
#define STRING_ENGINE_INVALID_OFFSET ((size_t) -1)
#define STRING_ENGINE_INVALID_UNICODE ((uint32_t) -1)

#define _UTF8_MASK_10000000 ((unsigned char) 128)
#define _UTF8_MASK_11000000 ((unsigned char) 192)
#define _UTF8_MASK_11100000 ((unsigned char) 224)
#define _UTF8_MASK_11110000 ((unsigned char) 240)
#define _UTF8_MASK_11111000 ((unsigned char) 248)
#define _UTF8_MASK_00111111 ((unsigned char) 63)
#define _UTF8_MASK_00011111 ((unsigned char) 31)
#define _UTF8_MASK_00001111 ((unsigned char) 15)
#define _UTF8_MASK_00000111 ((unsigned char) 7)

typedef struct _string_engine {
	VArray buffer;
	Array scopes;  // Array<size_t>
	size_t id;
} StringEngine;

typedef struct _string {
	StringEngine *engine;
	size_t index;
} String;

typedef struct _string_iter {
	String string;
	size_t offset;
} StringIter;

typedef struct _string_span {
	size_t ia;
	size_t ib;
} StringSpan;

// Array<StringSpan>
typedef Array StringMatchGroup;
typedef String (*StringRegexReplaceCallback)(String src, StringMatchGroup group, void *userdata);

// String Engine
void string_engine_init(StringEngine *se);
void string_engine_free(StringEngine *se);
void string_engine_bind(StringEngine *se);
void string_engine_push();
void string_engine_pop();
char *string_engine_clone(String s, size_t *len);
StringEngine *string_engine_current();
bool string_engine_equals(StringEngine *seA, StringEngine *seB);
size_t string_engine_depth();

// String operations that target the current bound engine.
String string_newf(const char *fmt, ...);
String string_newv(const char *fmt, va_list args);
String string_newb(char *buf, size_t len);
String string_news(String s);
String string_concat(String a, String b);
String string_substr(String o, size_t ia, size_t ib);
String string_join(Array a, String seq); // needs to be array<String>. Expect UB otherwise.
Array string_split(String s, String seq);

// String operations that do not target nor modify the engine.
// Note that engines still need to exist, as String stores info
// about their targeted engine.
bool string_empty(String s);
bool string_find(String s, String seq, size_t *pos);
char *string_data(String s);
char *string_data_length(String s, size_t *len);
size_t string_length(String s);
char string_char(String s, size_t i);
bool string_equals(String a, String b);
bool string_lt(String a, String b);

// Iterators and Utf-8 Introspection Support.
// None of them modify the engine.
StringIter string_iter_first(String s);
StringIter string_iter_find(String s, size_t offset);
StringIter string_iter_find_utf8(String s, size_t i); // O(i)
StringIter string_iter_last(String s);
StringIter string_iter_fix_utf8(StringIter it);
uint32_t string_iter_unicode(StringIter it);
char string_iter_get(StringIter it);
void string_iter_read(StringIter it, char *c);
void string_iter_next(StringIter *it);
void string_iter_prev(StringIter *it);
void string_iter_next_utf8(StringIter *it);
void string_iter_prev_utf8(StringIter *it);
bool string_iter_end(StringIter it);
bool string_iter_continue(StringIter it);

// Regex Support.
// - Targets and pushes strings to the current bound engine.
String string_regex_replaces(regex_t *reg, String src, String repl);
String string_regex_replacec(regex_t *reg, String src, StringRegexReplaceCallback callback, void *userdata);

// - Doesn't.
bool string_regex_compile(regex_t *reg, String pattern, int flags);
bool string_regex_match(regex_t *reg, String src);
void string_regex_free(regex_t *reg);
StringMatchGroup string_regex_find(regex_t *reg, String src);
Array string_regex_find_all(regex_t *reg, String src);
void string_regex_find_all_result_free(Array *arr);

size_t string_count(const char *s, size_t maxlen);

#endif
