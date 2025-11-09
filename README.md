# C Structs

This project is just a repository of a bunch of data structures I came up with in C, which I've been perfecting over the years. I've attempted to imitate most of the C++ stl functionality, such as the reserve/resize/fit, insert/erase calls, and iterators, as to ease their usage through code.

These data structures are agnostic of data types and certain errors (which are obviously the client's fault). If a data structure needs to store certain amounts of data inside it, the client is responsible for passing how many bytes are stored. Furthermore, the it's not the data structures fault for not handling certain errors. Its responsability of the user to validate and make sure its using the data structures correctly, taking care of pending pointers, avoiding memory leaks, and not passing wrong data types each function call.

I attempted to cover most test cases with them, although its not impossible for them to still contain bugs or unexpected logic errors inside them.

---
# Memory.h

Uses malloc internally.

I created this one to avoid checking the return of malloc every time. In case its `NULL`, it runs the recovery callback, which by default, exits the main application. If you come up with a better solution at global scope, you can always set your custom callback to handle these errors.

---
# Array.h

They're basically vectors in C++.

To summarise, they're arrays with a current length and capacity. In order to avoid repetitive malloc calls, arrays always allocate more space than requested, roughly 1/3 of its requested length. This amount can be changed at compile time, if needed, although it'll affect all arrays on the entire application.

They can only store items of same size in bytes inside them. If you have, for example, two different byte streams of different sizes, this data structure is not enough to store them.

Arrays are the simplest data structures that this project has, and are used to implement all the advanced ones. Arrays only work if the current system has `sizeof(uint8_t)` as `1 byte`. Otherwise, expect trouble.

---
# VArray.h

They're basically vectos in C++, with the ability to store items of variable size inside them.

In order to grant the array this ability, varray is composed of 2 arrays: data and metadata.

The data array is an array of bytes. It doesn't know the boundaries of each element inside it.

The metadata array is an array of {Offset, Length} of each element. It's used to provide a O(1) index time, and tell the boundaries of each element inside the byte stream array.

With this combination, one can use varrays to surpass the limitations of a plain array.

---
# String Engine.h

A few problems I have with strings in C:
   - They're not immutable objects
   - You need to call malloc/free, or depend on statically allocated char buffers
   - Lack of support for manipulating strings: concat, substr, find, split, join...

In order to create a counter measure against all of these cases, I created what's known as a String Engine. Imagine this thing as an actual variable, that'll store all allocated strings when its bound to the global state. Imitating a OpenGL call `bind`, only one string engine can be bound at a time.

When bound to a global state, whenever you'd create a new string, instead of you calling malloc and free afterwards, the string engine does all of that for you behind the scenes.

By using the concepts of scopes, you can push/pop a scope. All strings created inside a scope will be deallocated once the scope has been popped. Similarly, whenever you call `string_engine_free`, all memory allocated from the engine is released from memory.

While the idea sounds interesting, it has a few drawbacks:
   - Strings are only alive when you're inside an actual scope.
   - Strings do not know wether their data is being owned by another resource.
   - Thus, calling pop on an owned string, causes undefined behavior.

So, just like any use of pointers in C, its easy to get lost and do something wrong, if you accidentally leave a scope "by accident".

Here's an example on how to use it:
```
#include "string_engine.h"

int main(void) {
   StringEngine se;
   String a, b, c;

   string_engine_init(&se);
   string_engine_bind(&se);

   a = string_newf("a");
   b = string_newf("b");
   c = string_concat(a, b);

   printf("%u bytes: `%s`\n", string_length(c), string_data(c));

   string_engine_free(&se);

   return 0;
}
```

The header file contains more information about which operations create a string and target the current engine in order to write data into them. Furthermore, this module, unlike the others, was not meant to be thread-safe.

The string engine provides support for UTF-8 introspection, and regex in C. For this reason, it needs more compiler flags in order to fully compile the code.

