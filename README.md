# bench-exists
Simple benchmark of std::vector, std::set and std::unordered_set only checking if an element exists

When a container needs to be chosen and the **only** thing that matters is access time (i.e. time to create/populate/destroy is **NOT** important) a question often arises: *Will the simple std::vector be enough or do we need one of the more complex containers such as std::set or std::unordered_set?* Since we don't care if the elements are ordered, std::set feels like an overkill. But, on the other hand, we know that std::vector needs to be searched element-by-element which is not very optimal. Sure, if the data is sorted we can use std::binary_search, but that's seldom the case. Then again, std::vector is in cache-friendly contiguous memory... And last, but not least, there also is the mysterious std::unordered_set whose content cannot be changed (which is OK for us).

I decided to write a simple benchmark to answer some of the questions for myself, especially:
* Which one is the fastest?
* Is it true that for a small amount of data it's best to just use std::vector?
* Is it better to cache the value of size() or the end() iterator for use in the loop?

# How it works
It's rather self-explanatory, just **look at the code**. The benchmark always does two main passes:
- **PASS0**: searching for a non-existent element
- **PASS1**: searching for the middle element

All times are measured and later compared relatively against the fastest performer. This is done for each pass. There is a "cutoff" value (in GBs of searched data per test) for *std::vector*'s linear tests as those take **SIGNIFICANTLY** longer than the rest. If this value is given and exceeded most of the *std::vector* tests will be skipped. This is for a reason: on my laptop, when I reach an uncomfortable benchmark runtime of a few minutes for std::vector, the time for std::unordered_set is still too small to be properly measured...

For std::vector there are 5 tests:
1. for loop indexed via size_t and limited by size() [called on each iteration]
2. for loop indexed via size_t and limited by a cached value of size() [called once]
3. for loop indexed via iterators and limited by end() [called on each iteration]
4. for loop indexed via iterators and limited by cached value of end() [called once]
5. std::binary_search (yes, the data happens to be sorted)

and for both sets there are 2 tests:
1. using count()
2. using find()


# Results
So, without further ado, below are the (trimmed) results of running the benchmark on my oldish T61 laptop. The benchmark was compiled using g++-6.2 using the supplied Makefile (with full optimizations).
```
Benchmark with 1000 elements (data per loop = 0.008MB) and 10000000 loops
Searching through 0.073TB of data per test with vector cutoff limit = 700GB

Searching for non-existent elements:
Relative comparison (= slower than the fastest by this ratio):
[             unordered_set, using find()] 1.000x
[            unordered_set, using count()] 1.006x
[                      set, using count()] 17.585x
[                       set, using find()] 18.007x
[    vector, sorted, std::binary_search()] 26.124x
[          vector, iterated, cached end()] 1066.443x
[         vector, iterated, calling end()] 1066.922x
[vector, indexed by size_t, cached size()] 1068.718x
[ vector, indexed by size_t, using size()] 1072.595x

Searching for middle element:
Relative comparison (= slower than the fastest by this ratio):
[             unordered_set, using find()] 1.000x
[            unordered_set, using count()] 9.474x
[                      set, using count()] 24.903x
[                       set, using find()] 25.499x
[    vector, sorted, std::binary_search()] 34.807x
[          vector, iterated, cached end()] 332.291x
[         vector, iterated, calling end()] 333.338x
[vector, indexed by size_t, cached size()] 334.558x
[ vector, indexed by size_t, using size()] 336.657x 
```

# Conclusions
So, from the results it seems that:
* **It doesn't matter if you index the vector "by hand" (using an index) or via iterators**
* **It doesn't matter if you cache the "limiting" value** (*size()* or *end()*)
* **Always** use *find()* instead of *count()* when you're just checking for existence in a *std::set* or *std::unordered_set*
* *std::unordered_set* is always faster than *std::vector* by at least an **order of magnitude** (15x faster with 1000 elements and growing...)
* *std::unordered_set* is **always** the fastest
* Even for **small numbers of elements (\<1000)** *std::set* is **faster** than *std::vector* and *std::unordered_set* is even **faster**

And remember: **only the time needed to check if a value exists** was benchmarked. So this is **only valid** where you don't care about adding/removing elements or creating/destroying the container. And, as always, YMMV - run the benchmark yourself to be sure.
