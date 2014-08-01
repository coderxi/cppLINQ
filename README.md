cppLINQ
=======

LINQ style processing for C++, function/lambda connections as where/select/foreach (NOT SQL-style which need compiler support)

usage:

linq::seq(1,10).foreach([](int i) { std::cout << i << " "; }); // output
auto vec = linq::seq(1,10).select([](int i) { return i*i; }); // 1 4 9 16 ....
