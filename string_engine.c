#include "string_engine.h"

static int _id = 0;
static StringEngine *_current = NULL;

static void string_engine_scope_add(int i, size_t k) {
	size_t n;

	if (_current == NULL)
		return;

	array_read(&(_current->scopes), _current->scopes.length-1, &n);
	if (i >= 0) {
		n += k;
	} else {
		n -= k;
	}
	array_set(&(_current->scopes), _current->scopes.length-1, &n);
}

// String Engine
void string_engine_init(StringEngine *se) {
	size_t n = 0;
	varray_init(&(se->buffer));
	array_init(&(se->scopes), sizeof(size_t));
	array_push(&(se->scopes), &n);
	se->id = ++_id;
	if (_current == NULL) {
		_current = se;
	}
}

void string_engine_free(StringEngine *se) {
	if (_current != NULL && string_engine_equals(se, _current)) {
		_current = NULL;
	}

	varray_free(&(se->buffer));
	array_free(&(se->scopes));
}

void string_engine_bind(StringEngine *se) {
	_current = se;
}

void string_engine_push() {
	size_t n;

	if (_current != NULL) {
		n = 0;
		array_push(&(_current->scopes), &n);
	}
}

void string_engine_pop() {
	size_t n;
	if (_current != NULL) {
		array_read(&(_current->scopes), _current->scopes.length-1, &n);
		array_pop(&(_current->scopes));

		for (size_t i = 0; i < n; ++i) {
			varray_pop(&(_current->buffer));
		}
	}
}

char *string_engine_clone(String s, size_t *len) {
	char *tmp;
	size_t n;

	n = string_length(s);

	if (n > 0) {
		tmp = memory_malloc(n+1);
		memmove(tmp, string_data(s), n);
		tmp[n] = '\0';
	} else {
		tmp = NULL;
	}

	if (len != NULL)
		*len = n;

	return tmp;
}

StringEngine *string_engine_current() {
	return _current;
}

// Comparing pointers can be bad. It's better to ensure they're allocated and equal.
bool string_engine_equals(StringEngine *seA, StringEngine *seB) {
	if (seA != NULL && seB != NULL) {
		if (seA->id == seB->id) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

size_t string_engine_depth() {
	if (_current == NULL)
		return 0;

	return _current->scopes.length;
}

// String operations that target the current bound engine.
String string_newf(const char *fmt, ...) {
	va_list args;
	String s;

	if (_current == NULL) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	va_start(args, fmt);
	s = string_newv(fmt, args);
	va_end(args);

	return s;
}

String string_newv(const char *fmt, va_list args) {
	va_list _args;
	size_t len;
	size_t n;
	String s;

	if (_current == NULL) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	va_copy(_args, args);
	len = vsnprintf(NULL, 0, fmt, _args);
	va_end(_args);

	if (len == 0) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	} else {
		s.engine = _current;
		s.index = _current->buffer.metadata.length;
		varray_push(&(_current->buffer), NULL, len+1);
		vsnprintf(varray_get(&(_current->buffer), s.index, NULL), len+1, fmt, args);
		string_engine_scope_add(1, 1);
		return s;
	}
}

String string_newb(char *buf, size_t len) {
	String s;

	if (_current == NULL || len == 0) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	s.engine = _current;
	s.index = _current->buffer.metadata.length;
	varray_push(&(_current->buffer), NULL, len+1);

	if (buf != NULL)
		memmove(varray_get(&(_current->buffer), s.index, NULL), buf, len);

	string_engine_scope_add(1, 1);

	return s;
}

String string_news(String s) {
	if (_current == NULL) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	if (string_engine_equals(s.engine, _current)) {
		return s;
	} else {
		return string_newb(string_data(s), string_length(s));
	}
}

String string_concat(String a, String b) {
	char *tmp;
	String s;
	size_t na;
	size_t nb;

	if (_current == NULL) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	na = string_length(a);
	nb = string_length(b);

	if (na + nb == 0) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	s = string_newb(NULL, na + nb);

	tmp = string_data(s);

	if (!string_empty(a)) {
		memmove(&(tmp[0]), string_data(a), na);
	}

	if (!string_empty(b)) {
		memmove(&(tmp[na]), string_data(b), nb);
	}

	return s;
}

String string_substr(String o, size_t ia, size_t ib) {
	char *tmp;
	String s;
	size_t n;

	if (_current == NULL || string_empty(o)) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	if (ib < ia) {
		n = ib;
		ib = ia;
		ia = n;
	}

	n = string_length(o);

	if (ia > n) {
		ia = n;
	}

	if (ib > n) {
		ib = n;
	}

	if (ib - ia == 0) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	s = string_newb(NULL, ib - ia);
	tmp = string_data(o);

	memmove(string_data(s), &(tmp[ia]), ib - ia);

	return s;
}

// The function expects an array of strings including the NUL terminal.
String string_join(Array a, String seq) {
	String s;
	String o;
	char *tmp;
	char *foo;
	char *data;
	size_t index;
	size_t len;
	size_t n;
	size_t m;

	if (a.length == 0) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	data = string_data_length(seq, &len);

	// Calculating the final string size.
	n = (a.length-1) * len;
	for (ArrayIter it = array_iter_first(&a); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &o);
		n += string_length(o);
	}

	// Allocating the string.
	s = string_newb(NULL, n);
	tmp = string_data(s);

	// Joining everything.
	index = 0;
	for (ArrayIter jt, it = array_iter_first(&a); !array_iter_end(it); array_iter_next(&it)) {
		array_iter_read(it, &o);
		foo = string_data(o);

		if (foo != NULL) {
			m = string_length(o);
			memmove(&(tmp[index]), foo, m);
			index += m;
		}

		jt = it;
		array_iter_next(&jt);
		if (array_iter_continue(jt) && data != NULL) {
			memmove(&(tmp[index]), data, len);
			index += len;
		}
	}

	return s;
}

Array string_split(String s, String seq) {
	Array ls;
	String o;
	char *tmp;
	char *data;
	size_t len;
	size_t old;
	size_t n;

	data = string_data_length(seq, &len);
	tmp = string_data_length(s, &n);

	array_init(&ls, sizeof(String));

	if (len > 0 && data != NULL && tmp != NULL) {
		old = 0;
		for (size_t i = 0; i <= n; ++i) {
			if (n - i >= len) {
				// If a seq was found, append everything from [old, i) to ls.
				if (memcmp(&(tmp[i]), data, len) == 0) {
					o = string_substr(s, old, i);
					array_push(&ls, &o);
					i += len-1;
					old = i+1;
				}
			} else {
				// No more separators. Append everything from [old, n) and break.
				o = string_substr(s, old, n);
				array_push(&ls, &o);
				break;
			}
		}
	}

	return ls;
}


// String operations that do not target nor modify the engine.
// Note that engines still need to exist, as String stores info
// about their targeted engine.
bool string_empty(String s) {
	if (s.index == ((size_t) -1) || s.engine == NULL) {
		return true;
	} else {
		return false;
	}
}

bool string_find(String s, String seq, size_t *pos) {
	return false;
}

// Data returned by this function is owned by s.engine
char *string_data(String s) {
	if (string_empty(s)) {
		return "";
	}

	return varray_get(&(s.engine->buffer), s.index, NULL);
}

char *string_data_length(String s, size_t *len) {
	char *data;

	if (string_empty(s)) {
		return "";
	}

	data = varray_get(&(s.engine->buffer), s.index, len);

	if (len != NULL)
		*len -= 1;

	return data;
}

size_t string_length(String s) {
	VArrayMetadata md;

	if (string_empty(s)) {
		return 0;
	}

	array_read(&(s.engine->buffer.metadata), s.index, &md);

	return md.length-1;
}

char string_char(String s, size_t i) {
	char *data;
	size_t len;
	data = string_data_length(s, &len);
	if (i < len) {
		return data[i];
	} else {
		return '\0';
	}
}

bool string_equals(String a, String b) {
	size_t n = string_length(a);
	if (n == string_length(b)) {
		if (memcmp(string_data(a), string_data(b), n) == 0) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

// Needs testing...
bool string_lt(String a, String b) {
	size_t la = string_length(a);
	size_t lb = string_length(b);
	size_t n;

	if (la <= lb) {
		n = la;
	} else if (la > lb) {
		n = lb;
	}

	switch (memcmp(string_data(a), string_data(b), n)) {
	case 0:
	case 1:
		return false;
	default:
		return true;
	}
}

// Iterators and Utf-8 Introspection Support.
// None of them modify the engine.
StringIter string_iter_first(String s) {
	StringIter it;

	it.string = s;
	it.offset = 0;

	if (string_empty(s)) {
		it.offset = STRING_ENGINE_INVALID_OFFSET;
	}

	return it;
}

StringIter string_iter_find(String s, size_t offset) {
	StringIter it;

	it.string = s;
	it.offset = offset;

	if (string_empty(s) || offset >= string_length(s)) {
		it.offset = STRING_ENGINE_INVALID_OFFSET;
	}

	return it;
}

StringIter string_iter_find_utf8(String s, size_t i) {
	StringIter it;

	for (it = string_iter_first(s); !string_iter_end(it); string_iter_next_utf8(&it)) {
		if (i == 0) {
			return it;
		}
		--i;
	}

	it.offset = STRING_ENGINE_INVALID_OFFSET;

	return it;
}

StringIter string_iter_last(String s) {
	StringIter it;

	it.string = s;
	it.offset = string_length(s) - 1;

	if (string_empty(s)) {
		it.offset = STRING_ENGINE_INVALID_OFFSET;
	}

	return it;
}

StringIter string_iter_fix_utf8(StringIter it) {
	while (string_iter_continue(it) && ((unsigned char) string_char(it.string, it.offset)) & _UTF8_MASK_11000000 == _UTF8_MASK_10000000) {
		string_iter_prev(&it);
	}
	return it;
}

uint32_t string_iter_unicode(StringIter it) {
	uint32_t u = 0;
	unsigned char b;

	it = string_iter_fix_utf8(it);

	b = (unsigned char) string_char(it.string, it.offset);

	if ((b & _UTF8_MASK_10000000) == 0) {
		// 1 byte
		u += (uint32_t) b;
		return u;
	}

	if ((b & _UTF8_MASK_11100000) == _UTF8_MASK_11000000) {
		// 2 bytes
		u += (((uint32_t) b) & _UTF8_MASK_00011111) << 6;

		string_iter_next(&it);
		b = (unsigned char) string_iter_get(it);
		u += ((uint32_t) b) & _UTF8_MASK_00111111;
	
		return u;
	}

	if ((b & _UTF8_MASK_11110000) == _UTF8_MASK_11100000) {
		// 3 bytes
		u += (((uint32_t) b) & _UTF8_MASK_00001111) << 12;

		string_iter_next(&it);
		b = (unsigned char) string_iter_get(it);
		u += (((uint32_t) b) & _UTF8_MASK_00111111) << 6;

		string_iter_next(&it);
		b = (unsigned char) string_iter_get(it);
		u += ((uint32_t) b) & _UTF8_MASK_00111111;

		return u;
	}

	if ((b & _UTF8_MASK_11111000) == _UTF8_MASK_11110000) {
		// 4 bytes
		u += (((uint32_t) b) & _UTF8_MASK_00000111) << 18;

		string_iter_next(&it);
		b = (unsigned char) string_iter_get(it);
		u += (((uint32_t) b) & _UTF8_MASK_00111111) << 12;

		string_iter_next(&it);
		b = (unsigned char) string_iter_get(it);
		u += (((uint32_t) b) & _UTF8_MASK_00111111) << 6;

		string_iter_next(&it);
		b = (unsigned char) string_iter_get(it);
		u += ((uint32_t) b) & _UTF8_MASK_00111111;
	
		return u;
	}

	return STRING_ENGINE_INVALID_UNICODE;
}

char string_iter_get(StringIter it) {
	if (string_iter_continue(it)) {
		return string_char(it.string, it.offset);
	} else {
		return '\0';
	}
}

void string_iter_read(StringIter it, char *c) {
	if (string_iter_continue(it) && c != NULL) {
		*c = string_char(it.string, it.offset);
	}
}

void string_iter_next(StringIter *it) {
	if (string_iter_continue(*it)) {
		++(it->offset);
	}
}

void string_iter_prev(StringIter *it) {
	if (string_iter_continue(*it)) {
		--(it->offset);
	}
}

void string_iter_next_utf8(StringIter *it) {
	uint32_t u;
	*it = string_iter_fix_utf8(*it);
	if (string_iter_continue(*it)) {
		u = string_iter_unicode(*it);

		if (u >= 0x80 && u <= 0x7FF) {
			string_iter_next(it);
			string_iter_next(it);
		} else if (u >= 0x800 && u <= 0xFFFF) {
			string_iter_next(it);
			string_iter_next(it);
			string_iter_next(it);
		} else if (u >= 0x1000 && u <= 0x10FFFF) {
			string_iter_next(it);
			string_iter_next(it);
			string_iter_next(it);
			string_iter_next(it);
		} else {
			string_iter_next(it);
		}
	}
}

void string_iter_prev_utf8(StringIter *it) {
	*it = string_iter_fix_utf8(*it);
	if (string_iter_continue(*it)) {
		string_iter_prev(it);
		*it = string_iter_fix_utf8(*it);
	}
}

bool string_iter_end(StringIter it) {
	if (it.offset >= string_length(it.string)) {
		return true;
	} else {
		return false;
	}
}

bool string_iter_continue(StringIter it) {
	if (it.offset >= string_length(it.string)) {
		return false;
	} else {
		return true;
	}
}


// Regex Support.
// - Targets and pushes strings to the current bound engine.
String string_regex_replaces(regex_t *reg, String src, String repl) {
	Array ls;
	String s;
	StringMatchGroup group;
	StringSpan span;
	char *tmp;
	size_t n;
	size_t old;

	if (_current == NULL) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	s = string_newf("");

	ls = string_regex_find_all(reg, src);

	old = 0;
	string_engine_push();

	n = string_length(src);
	for (ArrayIter it = array_iter_first(&ls); !array_iter_end(it) && old < n; array_iter_next(&it)) {
		array_iter_read(it, &group);
		array_read(&group, 0, &span);

		s = string_concat(s, string_substr(src, old, span.ia));
		s = string_concat(s, repl);
		old = span.ib;
	}

	if (old < n) {
		s = string_concat(s, string_substr(src, old, n));
	}

	tmp = string_engine_clone(s, &n);
	string_engine_pop();

	string_regex_find_all_result_free(&ls);

	s = string_newb(tmp, n);
	memory_free(tmp);
	tmp = NULL;

	return s;
}

String string_regex_replacec(regex_t *reg, String src, StringRegexReplaceCallback callback, void *userdata) {
	Array ls;
	String s;
	String o;
	StringEngine *engine;
	StringMatchGroup group;
	StringSpan span;
	char *tmp;
	size_t n;
	size_t old;
	size_t depth;

	if (_current == NULL) {
		s.engine = NULL;
		s.index = ((size_t) -1);
		return s;
	}

	if (callback == NULL) {
		return string_news(src);
	}

	s = string_newf("");

	ls = string_regex_find_all(reg, src);

	old = 0;
	engine = _current;
	depth = string_engine_depth();
	string_engine_push();
	n = string_length(src);
	for (ArrayIter it = array_iter_first(&ls); !array_iter_end(it) && old < n; array_iter_next(&it)) {
		array_iter_read(it, &group);
		array_read(&group, 0, &span);

		string_engine_bind(engine);
		s = string_concat(s, string_substr(src, old, span.ia));
		string_engine_bind(engine);
		o = callback(src, group, userdata);
		string_engine_bind(engine);
		s = string_concat(s, o);
		old = span.ib;
	}

	if (old < n) {
		s = string_concat(s, string_substr(src, old, n));
	}

	tmp = string_engine_clone(s, &n);

	while (string_engine_depth() > depth) {
		string_engine_pop();
	}

	string_regex_find_all_result_free(&ls);

	s = string_newb(tmp, n);
	memory_free(tmp);
	tmp = NULL;

	return s;
}


// - Doesn't.
bool string_regex_compile(regex_t *reg, String pattern, int flags) {
	if (regcomp(reg, string_data(pattern), flags | REG_EXTENDED) != 0)
		return false;
	else
		return true;
}

bool string_regex_match(regex_t *reg, String src) {
	regmatch_t match;

	if (!regexec(reg, string_data(src), 1, &match, 0)) {
		if (match.rm_so == 0 && match.rm_eo == string_length(src)) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void string_regex_free(regex_t *reg) {
	regfree(reg);
}

StringMatchGroup string_regex_find(regex_t *reg, String src) {
	StringMatchGroup group;
	StringSpan span;
	regmatch_t *matches;

	matches = memory_malloc(sizeof(regmatch_t) * (reg->re_nsub + 1));

	array_init(&group, sizeof(StringSpan));
	if (!regexec(reg, string_data(src), (reg->re_nsub + 1), matches, 0)) {
		for (size_t i = 0; i <= reg->re_nsub; ++i) {
			span.ia = matches[i].rm_so;
			span.ib = matches[i].rm_eo;
			array_push(&group, &span);
		}
	}

	memory_free(matches);
	matches = NULL;

	return group;
}

Array string_regex_find_all(regex_t *reg, String src) {
	Array ls;
	StringSpan span;
	regmatch_t *matches;
	StringMatchGroup group;
	size_t offset;
	char *tmp;
	bool succ;

	tmp = string_data(src);

	offset = 0;
	matches = memory_malloc(sizeof(regmatch_t) * (reg->re_nsub + 1));
	array_init(&ls, sizeof(StringMatchGroup));

	while (!regexec(reg, &(tmp[offset]), (reg->re_nsub + 1), matches, 0)) {
		array_init(&group, sizeof(StringSpan));

		for (size_t i = 0; i <= reg->re_nsub; ++i) {
			span.ia = offset + matches[i].rm_so;
			span.ib = offset + matches[i].rm_eo;
			array_push(&group, &span);
		}

		array_push(&ls, &group);
		offset += matches[0].rm_eo;
	}

	memory_free(matches);
	matches = NULL;

	return ls;
}

void string_regex_find_all_result_free(Array *arr) {
	for (ArrayIter it = array_iter_first(arr); !array_iter_end(it); array_iter_next(&it)) {
		array_free(array_iter_get(it));
	}
	array_free(arr);
}

size_t string_count(const char *s, size_t maxlen) {
	for (size_t i = 0; i < maxlen; ++i) {
		if (s[i] == '\0') {
			return i;
		}
	}

	return maxlen;
}
