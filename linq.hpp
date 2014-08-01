
#ifndef _X_LINQ_HPP_
#define _X_LINQ_HPP_

#include <iostream>
#include <type_traits>
#include <utility>
#include <iterator>

#include <vector>
#include <map>

namespace linq
{
    struct c_failure {};

    template <typename F> struct t_with2args {
        template <typename X> static auto check(X && x) -> decltype(std::declval<F>()(x,(int)0));
        static c_failure check(...);
    };
    template <typename F, typename X, typename CT> struct t_call_impl {
        typedef CT return_type;
        template <typename FT, typename T> static return_type call(FT && f, T && t, int i) { return f(t,i); }
    };
    template <typename F, typename X> struct t_call_impl<F, X, c_failure> {
        typedef decltype(std::declval<F>()(std::declval<X>())) return_type;
        template <typename FT, typename T> static return_type call(FT && f, T && t, int i) { return f(t); }
    };
    template <typename F, typename X> struct t_call {
        typedef decltype(t_with2args<F>::check(std::declval<X>())) check_type;
        typedef t_call_impl<F, X, check_type> impl;
        typedef typename impl::return_type return_type;
        template <typename FT, typename T> static return_type call(FT && f, T && t, int i) { return impl::call(f,t,i); }
    };

    template <typename C> class c_linq
    {
        public:
            C   __c;    // for copy/move ctor
            C & _c;     // for borrowed container
            c_linq(C && c) : __c(std::move(c)), _c(__c)  {} // for debug  { std::cout << "c_linq, move ctor\n"; }
            c_linq(C &  c) :         _c(c)               {} // for debug { std::cout << "c_linq, copy/reference ctor\n"; }

            template <typename F> struct t_func {
                typedef t_call<F, decltype(*_c.begin())> call_type;
                typedef typename call_type::return_type return_type;
                typedef typename std::remove_reference<return_type>::type ele_type;
                typedef std::vector<ele_type> vec_type;
                typedef std::vector<typename std::remove_reference<decltype(*_c.begin())>::type> vec0_type;
            };

            typedef typename std::remove_reference<decltype(*_c.begin())>::type ele_type; 

            template <typename F> auto select(F && f) -> c_linq<typename t_func<F>::vec_type>
            {
                typedef typename t_func<F>::call_type call_type;
                typedef typename t_func<F>::vec_type vec_type;
                vec_type vec;
                vec.reserve(_c.size());
                int i = 0;
                for (auto it = _c.begin(); it != _c.end(); ++it, ++i)
                    vec.push_back(call_type::call(f, *it, i));
                return c_linq<vec_type>(std::move(vec));
            }

            template <typename F> auto where(F && f) -> c_linq<typename t_func<F>::vec0_type>
            {
                typedef typename t_func<F>::call_type call_type;
                typedef typename t_func<F>::vec0_type vec_type;
                vec_type vec;
                vec.reserve(_c.size());
                int i = 0;
                for (auto it = _c.begin(); it != _c.end(); ++it, ++i)
                    if (call_type::call(f, *it, i)) vec.push_back(*it);
                return c_linq<vec_type>(std::move(vec));
            }

            template <typename F> c_linq foreach(F && f)
            {
                typedef typename t_func<F>::call_type call_type;
                int i = 0;
                for (auto it = _c.begin(); it != _c.end(); ++it, ++i) call_type::call(f, *it, i);

                return (*this);
            }

            // only one version will be instantiated
            template <typename A, typename B>   struct t_same       { static const bool value = false;};
            template <typename A>               struct t_same<A,A>  { static const bool value = true;};
            template <typename L, bool> struct t_move_impl { 
                L & linq;
                t_move_impl(L & l) : linq(l) {}
                std::vector<typename L::ele_type> operator()() { /*std::cout << " -> move \n";*/ return std::move(linq.__c); }
            };
            template <typename L> struct t_move_impl<L, false> {    // for different type, need copy
                L & linq;
                t_move_impl(L & l) : linq(l) {}
                std::vector<typename L::ele_type> operator()() { return linq.vec_by_copy(); }
            };

            std::vector<ele_type> tovec() {
                if (&__c == &_c)
                    return t_move_impl<c_linq<C>, t_same<std::vector<ele_type>, C>::value>(*this)();        // if the container is embedded
                else
                    return vec_by_copy();
            }

            std::vector<ele_type> vec_by_copy() {
                // std::cout << " -> copy \n";
                std::vector<ele_type> vec(_c.size());
                std::copy(_c.begin(), _c.end(), vec.begin());
                return std::move(vec);
            }

            template <typename FK, typename FV> auto tomap(FK && fk, FV && fv)
                -> std::map< typename std::remove_reference<typename t_func<FK>::return_type>::type,
                             typename std::remove_reference<typename t_func<FV>::return_type>::type >
            {
                typedef std::map< typename std::remove_reference<typename t_func<FK>::return_type>::type,
                             typename std::remove_reference<typename t_func<FV>::return_type>::type > c_map;
                typedef typename t_func<FK>::call_type fk_call;
                typedef typename t_func<FV>::call_type fv_call;
                c_map map;
                int i = 0;
                for (auto it = _c.begin(); it != _c.end(); ++it, ++i)
                    map.insert(std::make_pair(fk_call::call(fk,*it,i), fv_call::call(fv,*it,i)));
                return std::move(map);
            }
    };

    struct c_range {
        struct iterator {
            int index;
            iterator() : index(-1) {}
            iterator(int i) : index(i) {}
            iterator(const iterator& o) : index(o.index) {}
            iterator& operator=(const iterator& o) { index = o.index; return *this; }
            bool operator==(const iterator& o) const { return index == o.index; }
            bool operator!=(const iterator& o) const { return index != o.index; }
            iterator& operator++() { ++index; return *this; }
            int operator*() { return index; }
        };
        int _start, _end;
        c_range(int s, int e) : _start(s), _end(e) {}
        iterator begin() { return iterator(_start); }
        iterator end()   { return iterator(_end); }
        size_t size() { return _end - _start; }
    };

    // container for range/list from indexed parent container
    // caller make sure the part is within container's range
    template <typename C, typename R> struct c_part {
        struct iterator {
            C * c;
            typedef typename R::iterator riterator;
            riterator rit;
            iterator() : c(NULL) {}
            iterator(C * xc, riterator xrit) : c(xc), rit(xrit) {}
            iterator(const iterator& o) : c(o.c), rit(o.rit) {}
            iterator& operator=(const iterator& o) { c = o.c; rit = o.rit; }
            iterator& operator++() { ++rit; }
            bool operator==(const iterator & o) { return c==o.c && rit==o.rit; }
            bool operator!=(const iterator & o) { return ! ((*this)==o) ; }
            decltype((*c)[0]) operator*() { return (*c)[*rit]; }

            typedef ptrdiff_t difference_type; //almost always ptrdif_t
            typedef typename std::remove_reference<decltype((*c)[0])>::type value_type; //almost always T
            typedef value_type & reference; //almost always T& or const T&
            typedef value_type *  pointer;  //almost always T* or const T*
            typedef std::forward_iterator_tag iterator_category;  //usually std::forward_iterator_tag or similar
        };

        C & c;
        R range;
        iterator begin() { return iterator(&c, range.begin()); }
        iterator end() { return iterator(&c, range.end()); }

        c_part(C & co, R ro) : c(co), range(std::move(ro)) {}
        size_t size() { return range.size(); }
    };

    // -- API for user --
    template <typename C> c_linq<typename std::remove_reference<C>::type> list(C && c)
    { 
        return c_linq<typename std::remove_reference<C>::type>(std::forward<C>(c)); 
    }

    // c must be reference (NON temp!)
    template <typename C, typename R> auto list(C& c, R && range)
        -> c_linq<c_part<typename std::remove_reference<C>::type, typename std::remove_reference<R>::type> >
    { 
        typedef c_part<typename std::remove_reference<C>::type, typename std::remove_reference<R>::type> P;
        return c_linq<P>( P(c, std::forward<R>(range)) );
    }

    c_linq<std::vector<int> > seq(int start, int end)
    {
        std::vector<int> vec;
        vec.reserve(start < end ? end - start : 0);
        for (int i = start; i < end; ++i) vec.push_back(i);
        return list(std::move(vec));
    }
}

#endif  // _X_LINQ_HPP_

