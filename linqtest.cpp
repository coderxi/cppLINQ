
#include "linq.hpp"

void test()
{
    std::cout << "-- test for ctor.\n";
    {
        std::vector<int> vec;
        linq::list(vec);
    }

    std::cout << "-- test for select.\n";
    {
        linq::seq(0,5).select([](int i, int j) { std::cout << i << ", " << j << "\n"; return i*j; });
    }

    std::cout << "-- test for foreach\n";
    {
        linq::seq(0,5).foreach([](int i, int j) { std::cout << i << ", " << j << "\n"; return i*j; });
    }

    std::cout << "-- test for where\n";
    {
        linq::seq(0,5).where([](int i, int j) { std::cout << i << ", " << j << ", return " << (i>4) << "\n"; return i > 4; })
            .foreach([](int i, int j) { std::cout << i << ", " << j << "\n"; });
        linq::seq(0,5).where([](int i, int j) { std::cout << i << ", " << j << ", return " << (i>4) << "\n"; return i > 4; })
            .foreach([](int i) { std::cout << i << "\n"; });
    }

    std::cout << "-- test for to/tomap\n";
    {
        auto vec = linq::seq(0,5).tovec();
        std::cout << "to vector, size " << vec.size() << " : \n";
        linq::list(vec).foreach([](int k, int i) { std::cout << "i " << i << " " << k << "\n"; });
        std::cout << "to map : \n";
        auto map = linq::seq(0,5).tomap([](int v, int i) { return i; }, [](int v, int i) { return i*v; });
        linq::list(map).foreach([](decltype(*map.begin()) p) { std::cout << "key " << p.first << ", value " << p.second << "\n"; });
    }

    std::cout << "-- test for range\n";
    {
        auto vec = linq::seq(0,5).tovec();
        auto x = linq::list(vec, linq::c_range(2,5));
        x.foreach([](int k) { std::cout << k << "\n"; });
        linq::list(vec, linq::c_range(2,5)).foreach([](int k, int i) { std::cout << "i " << i << ", " << k << "\n"; });
    }

    std::cout << "-- test for range + to\n";
    {
        auto lambda = [](int k, int i) { std::cout << "i " << i << ", k " << k << "\n"; };
        auto vec = linq::seq(0,5).tovec();
        auto x = linq::list(vec, linq::c_range(2,5));
        auto v2 = x.tovec();
        std::cout << "full list : \n";
        linq::list(vec).foreach(lambda);
        std::cout << "range list : \n";
        linq::list(v2).foreach(lambda);
    }

    std::cout << "-- test for vector range\n";
    {
        auto lambda = [](int k, int i) { std::cout << "i " << i << ", k " << k << "\n"; };
        auto vec = linq::seq(0,5).tovec();
        auto range = linq::seq(2,5).tovec();
        auto x = linq::list(vec, range);
        auto v2 = x.tovec();
        std::cout << "full list : \n";
        linq::list(vec).foreach(lambda);
        std::cout << "range list : \n";
        linq::list(v2).foreach(lambda);
    }

    std::cout << "-- test for change content via foreach\n";
    {
        auto out = [](int k, int i) { std::cout << "i " << i << ", k " << k << "\n"; };
        auto chg = [](int & k, int i) { k += i; };
        auto vec = linq::seq(0,5).tovec();
        std::cout << "full list : \n";
        linq::list(vec).foreach(out);
        linq::list(vec).foreach(chg);
        std::cout << "after change list : \n";
        linq::list(vec).foreach(out);

        auto map = linq::seq(0,5).tomap([](int v, int i) { return i; }, [](int v, int i) { return v; });
        typedef std::remove_reference<decltype(*map.begin())>::type ele_type;
        auto chg2 = [](ele_type & e, int i) { e.second += i + e.first; };
        std::cout << "full map list : \n";
        linq::list(map).foreach([](ele_type & e, int i) { std::cout << "i " << i << ", " << e.first << " - " << e.second << "\n"; });
        linq::list(map).foreach(chg2);
        std::cout << "after change list : \n";
        linq::list(map).foreach([](ele_type & e, int i) { std::cout << "i " << i << ", " << e.first << " - " << e.second << "\n"; });
    }

    std::cout << "-- test for foreach/sort \n";
    {
        linq::seq(1,10).foreach([](int i) { std::cout << "i " << i << "\n"; })
            .foreach([](int i) { std::cout << "i " << i*i << "\n"; });

    }

    std::cout << "-- test for ....\n";
    {
    }
}


#if 1
int main(int argc, char * argv[])
{
    test();
}
#endif

// g++ -std=c++0x linqtest.cpp -o linqtest.exe


