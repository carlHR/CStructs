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

The string engine provides support for UTF-8 introspection, and regex in C. For this reason, it needs more compiler flags in order to fully compile the code. With msys2 on windows, the compiler flags needed are `-IC:/msys64/mingw64/bin/../include -D__USE_MINGW_ANSI_STDIO=1 -LC:/msys64/mingw64/bin/../lib -lregex -ltre -pipe -lintl`, which is the result of `pkgconf.exe --libs --cflags regex`.

---
# Hashmap

Behaves closely to C++ map, although it's keys and values must be of a constant size in bytes.

Can be used to associate key and value, regardless of their data type, treating both the key and value as byte streams. It makes use of a hashing function from hash.h, to convert the key with a huge number.

The hashmap makes use of a bunch of buckets, which are arrays that stores the key-value pairs contiguously. Whenever the user requests to search for an item, the key is hashed into a number, this number is then pointed at one of the buckets, and for all items of that bucket, all keys are compared as strings, character by character. The longer the keys, the longer it takes to compare them fully and retrieve the item.

The only drawback of this data structure is that values are not tightly packed with each other (as there're keys in between), and the entire data is not contiguous (as each bucket has a different data memory location). 

---
# VHashmap

Behaves like C++ map, granting an associative storage between a key of variable size, and a value of constant size in bytes.

Behaves much like hashmap.h internally, but makes use of VArray in each bucket to store the key-value pair. Unlike hashmap.h, the data structure does not store the `klen` attribute anymore, as key length is of variable size. Only `vlen` remains.

The only drawback of this data structure is the same as hashmap.h: values are not tightly packed with each other (as there're keys in between), and the entire data is not contiguous (as each bucket has a different data memory location). 

---
# Cache

Behaves like Hashmap, but ensures that all values are tightly packed inside a single contiguous dynamic memory block.
Keys must be of constant size. Values must be of constant size.

---
# VCache

Behaves like Cache, but values can be of variable size.

---
# Dict

Behaves like VHashmap, but ensures that all values are tightly packed inside a single contiguous dynamic memory block.
Keys can have variable size from 0..N, given that you pass the size in bytes each time a key is requested.
Values must be of constant size.

---
# VDict

Behaves like Dict, but values can be of variable size.

---
# Set

Sets are different structures who also depend on hashes. Their main use is to store unordered items inside a tightly packed and contiguous dynamic memory block. The main goal of using hashes is to decrease the time to find the item's index.

So, for example, if you want to tightly store structs inside a single array, and for some reason, you need to find where those items are located inside the array, using no other key but the value itself, sets are perfect for this job. As the main goal here is to find elements regardless of their order, I didn't implemented functions to insert values in specific positions, or change data.

As each value stored is also a key, all values need to be unique. So, as to avoid creating duplicates, you shouldn't set data in a set manually. If you need to, then remove the value, and then add the new value right after. Each add/del function returns true\|false depending wether they succeed. 

As indexing uses hashmaps buckets, implemented internally from scratch, their search time should be optimal even at worst case. if you allocate a set with lots of buckets, the search time may hopefully stay optimal in most cases.

<sub>I got the idea to create such a data structure, when attempting to program with OpenGL. In this context, you commonly need to store shader vertex data, such as position, texture coordinates, normals, and vertex colors somehow, and I commonly go with the route of using a single VBO. So, whenever you need to create a mesh, you must find each vertex data position, and assign those indices to your mesh (which is the element array buffer). So, a set, in this particular case, fits perfectly.</sub>

---
# About iterators

Iterators insert/erase are messy. They allow you to insert and erase values inside containers while you're iterating through it. However, they're tricky to use, and you need to know firsthand how the data structure works internally, before you attempt to use them.

For arrays and varrays, its very direct. For hashmaps, caches, dicts, its not that simple, although they all behave the same within this group. Strings and sets do not have such operations.

### General Example

Although their return values differ in context, they syntax is the same for all iterator insert/erase functions:

```
Array a;
ArrayIter it, ot;

// [...] Initialize the array...

it = array_iter_first(&a);
ot = array_iter_insert(&it);
ot = array_iter_erase(&it);

// [...] Free the array...

```

As you can see, these functions have 1 input value: `it`, and 2 return values: `it` and `ot`.

### For arrays / varrays:

Inserting items always occurs before the specified position. You can test this with plain `array_insert`. For iterators its the same. The returned value of `it` continues pointing to the current item, as if no insertion occurred, and the returned value of `ot` points to the inserted item.

This means that if you're iterating forward using next, use the value of `it` to avoid infinite loops. If you're iterating in the reverse order using prev, use the value of `ot`.

Now, when erase, its oddly similar. `it` refers to the previous element, as the current one was already eliminated. `ot` refers to the next element. When iterating forward, if you use `ot`, you'll skip one element. This doesn't happen if you use `it`. When iterating on the reverse order, you must use `ot`, otherwise you'll skip one element.

Sounds weird, but its simple:
   - Are you iterating forward? Use `it`.
   - Are you iterating backwards? Use `ot`.

### For hashmaps, caches and dicts:

Iterators points to keys-values/indices within each bucket. When the iterator reaches the end of a bucket, the iterator simply jumps to the beggining of the next bucket, until all buckets are searched. When iterating on the reverse order, iterators jump to the end of a bucket, going backwards one element by one.

When inserting elements, there can be 3 behaviors:
   1. The key passed is already present on the container. So, it acts as data overwrite.
   2. The key passed is not present, and will be inside the same bucket as the iterator.
   3. The key passed is not present, and won't be inside the same bucket as the iterator.

When situation 1 happens, both `it` and `ot` won't change. Nothing is inserted.

When situation 2 happens, the function behaves exactly like array and varray. `it` points to the current element as if nothing had been inserted, and `ot` points to the inserted element.

When situation 3 happens, `it` points to the current element as if nothing had been inserted, and `ot` points to the inserted element.

Now, situations 2 and 3 differ in the sense that, in situation 2, you'll always skip the inserted element during the iteration. In situation 3, this might not happen. Depending on which bucket the item gets inserted, you may yet iterate over it later on.

When erasing elements, it'll attempt to erase only the current iterator, so the operation is guaranteeded to return:
   - `it` as the previous element
   - `ot` as the next element

For all iterators, if `it` or `ot` points to an element that is out of bounds, the iterator itself is invalid, and iter_end will return *true*, and iter_continue will return *false*.

