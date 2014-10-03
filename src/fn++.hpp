#ifndef _1af7690b_7fca_4464_9e14_c71bc5a29ee9
#define _1af7690b_7fca_4464_9e14_c71bc5a29ee9

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

#ifndef _MSC_VER
#define FN_TYPENAME typename
#else
#define FN_TYPENAME
#endif

namespace fn{

template<typename T> class optional;

namespace fn_ {

/*
 *  remove reference from a type, similar to std::remove_reference
 */
template<class R> struct remove_reference      {typedef R T;};
template<class R> struct remove_reference<R&>  {typedef R T;};
template<class R> struct remove_reference<R&&> {typedef R T;};

/*
 *  remove const from a type
 */
template<typename C> struct noconst{ typedef C T; };
template<typename C> struct noconst<C const&>{ typedef C T; };
template<typename C> struct noconst<C const>{ typedef C T; };

/*
 *  cast to rvalue reference, similar to std::move
 */
template <class T>
typename remove_reference<T>::T&&
move(T&& a)
{
    return ((typename remove_reference<T>::T&&)a);
}


/*
 * Iterator factory for iteration over ranges of values
 */
template<typename T>
class Range
{
    class IT
    {
    public:
        typedef T value_type;

        IT(T const& position):
            position(position)
        {}

        IT(T const& position, T const& step):
            position(position),
            step(step)
        {}

        bool operator!=(IT const& other) const
        {
            return position != other.position;
        }

        IT const& operator++()
        {
            position += step;
            return *this;
        }

        T const& operator*() const
        {
            return position;
        }

    private:
        T position;
        T step = 1;
    };

    IT from;
    IT const to;

public:
    Range(T const& to):
        from(0),
        to(to)
    {}

    Range(T const& from, T const& to):
        from(from),
        to(to)
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};


/*
 * Factory to create an iterator that applies a function to every
 * element of an other iterator.
 */
template<typename FN,typename G, typename OtherIT>
class Map
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;

    public:
        IT(FN fn,OtherIT other_it):
            fn(fn),
            other_it(other_it)
        {}

        bool operator!=(IT& other)
        {
            return other_it != other.other_it;
        }

        IT const& operator++()
        {
            ++other_it;
            return *this;
        }

        auto operator*() const -> decltype(fn(*other_it))
        {
            return fn(*other_it);
        }
    };

    IT const from;
    IT const to;

public:
    Map(FN fn, G const& g):
        from(fn,g.begin()),
        to(fn,g.end())
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};

/*
 * Makes begin/end iterator pairs usable with range based for
 */
template<typename OtherIT>
class AsRange
{
    OtherIT const from;
    OtherIT const to;

public:
    AsRange(OtherIT from, OtherIT to):
        from(from),
        to(to)
    {}

    OtherIT const& begin() const
    {
        return from;
    }

    OtherIT const& end() const
    {
        return to;
    }
};

/*
 * Creates an iterator that allows simultaneous iteration
 * over two other iterators.
 * The iteration will terminate if either of the two input
 * iterators end.
 */
template<
    template<typename,typename> class PairT,
    typename A,
    typename B,
    typename OtherIT1,
    typename OtherIT2
>
class Zip
{
    class IT
    {
    private:
        typename fn_::noconst<OtherIT1>::T other_it1;
        typename fn_::noconst<OtherIT2>::T other_it2;
        typedef PairT<decltype(*other_it1),decltype(*other_it2)> Pair;

    public:
        typedef Pair value_type;

        IT(OtherIT1 other_it1,OtherIT2 other_it2):
            other_it1(other_it1),other_it2(other_it2)
        {}

        bool operator!=(IT& other)
        {
            return other_it1!=other.other_it1
                && other_it2 != other.other_it2;
        }

        IT& operator++()
        {
            ++other_it1;
            ++other_it2;
            return *this;
        }

        Pair operator*() {
            return Pair(*other_it1,*other_it2);
        }
    };

    IT const from;
    IT const to;

public:
    Zip(A& a, B& b):
        from(a.begin(),b.begin()),
        to(a.end(),b.end())
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};


/*
 * The value type used by Zip.
 * It aliases the first and second element to a few
 * other names that make more sense depending on context.
 */
template<typename A, typename B>
struct Pair
{
    Pair(A& a, B& b):
        first(a),second(b),
        key(a),value(b),
        nr(a),item(b)
    {}

    A& first;
    B& second;

    A& key;
    B& value;

    A& nr;
    B& item;
};

/*
 * Creates an iterator that only iterates over only those
 * elements of another iterator, that match a condition supplied
 * as a function.
 */
template<typename FN,typename G, typename OtherIT>
class Filter
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;
    public:
        IT(FN fn,OtherIT other_it):
            fn(fn),
            other_it(other_it)
        {}

        bool operator!=(IT& other)
        {
            while(other_it!=other.other_it && !fn(*other_it)){
                ++other_it;
            }
            return other_it != other.other_it;
        }

        IT const& operator++()
        {
            ++other_it;
            return *this;
        }

        auto operator*() const -> decltype(*other_it)
        {
            return *other_it;
        }
    };

    IT const from;
    IT const to;

public:
    Filter(FN fn, G const& g):
        from(fn,g.begin()),
        to(fn,g.end())
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};

template<typename T>
struct IsTrue
{
    bool operator()(T i) const
    {
        return !!i;
    }
};


template<typename T> class optional_ref;
template<typename T> class optional_value;

template<typename T> struct return_cast;

/*
 *  This helper allows for easier usage of a functions return type
 *  in a type deduction context.
 *  Additionally it works around a strange msvc internal compiler error.
 */
template<class F, class T>
static auto return_type(F&& f,T&& value)->decltype(f(value))
{
    return f(value);
}


template<typename T>
class optional_base
{
public:
    typedef T Type;
    bool valid() const { return !!value; }

protected:
    optional_base(T* const p):
        value(p)
    {}

private:
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T&>;
    friend class optional<T>;
    friend class optional_value<T>;
    friend class optional_ref<T>;

    T* value = nullptr;

public:
    template<typename F>
    T operator|(F fallback) const
    {
        return valid() ? *value : fallback;
    }

    bool operator==(T const& other_value) const
    {
        return valid() && (*value == other_value);
    }

    bool operator!=(T const& other_value) const
    {
        return !((*this) == other_value);
    }

    bool operator==(optional_base const& other) const
    {
        return (valid() && other.valid()) && (*value == *other.value);
    }

    bool operator!=(optional_base const& other) const
    {
        return !(*this == other);
    }

    template<typename EmptyF>
    auto operator||(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        if(!valid()){
            return handle_no_value();
        }
        else {
            return return_cast<decltype(handle_no_value())>::value(*value);
        }
    }

    T operator~() const
    {
        return (*this) | T{};
    }
};

template<typename T>
class optional_value : public optional_base<T>
{
    friend class optional<T>;
    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

    using optional_base<T>::value;

protected:
    unsigned char value_mem[sizeof(T)];

    optional_value():
        optional_base<T>(nullptr)
    {}

    optional_value(T* v) :
        optional_base<T>(v)
    {}

    ~optional_value()
    {
        if(optional_base<T>::valid()){
            reinterpret_cast<T*>(value_mem)->~T();
        }
    }

public:
    using optional_base<T>::valid;

    template<typename ValueF>
    auto operator>>(ValueF const& handle_value) const
        ->decltype(
            return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value)
        )
    {
        if(valid()){
            return return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value);
        }
        else{
            return {};
        }
    }
};

template<typename T>
class optional_ref : public optional_base<T>
{
    friend class optional<T>;
    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

    using optional_base<T>::value;

protected:
    optional_ref():
        optional_base<T>(nullptr)
    {}

    optional_ref(T* v):
        optional_base<T>(v)
    {}

    optional_ref(T&& value):
        optional_base<T>(true,value)
    {}

public:
    template<typename F>
    T& operator|(F& fallback) const
    {
        if(optional_base<T>::valid()){
            return *optional_base<T>::value;
        }
        else{
            return fallback;
        }
    }

    template<typename F>
    T operator|(F const& fallback) const
    {
        if(optional_base<T>::valid()){
            return *optional_base<T>::value;
        }
        else{
            return fallback;
        }
    }

    optional_ref& operator=(optional_ref const& other)
    {
        optional_base<T>::value = other.value;
        return *this;
    }

public:
    using optional_base<T>::valid;

    template<typename ValueF>
    auto operator>>(
        ValueF const& handle_value) const
        ->decltype(
            return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value)
        )
    {
        if(valid()){
            return return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value);
        }
        else{
            return {};
        }
    }
};

}

template<typename T>
class optional final : public fn_::optional_value<T>
{
    using fn_::optional_value<T>::value_mem;

    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

public:
    optional():
        fn_::optional_value<T>()
    {}

    optional(T&& v):
        fn_::optional_value<T>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{fn_::move(v)};
    }

    optional(optional<T>&& original):
        fn_::optional_value<T>(
            original.valid()
                ? reinterpret_cast<T*>(value_mem)
                : nullptr
        )
    {
        if(original.valid()){
            new (value_mem) T{fn_::move(*original.value)};
            original.value = nullptr;
            reinterpret_cast<T*>(original.value_mem)->~T();
        }
    }

    optional& operator=(optional&& other)
    {
        if(other.valid()){
            if(fn_::optional_value<T>::valid()){
                *fn_::optional_value<T>::value = fn_::move(*other.value);
            }
            else{
                new (value_mem) T{fn_::move(*other.value)};
                fn_::optional_value<T>::value = reinterpret_cast<T*>(value_mem);
            }
        }
        else{
            if(fn_::optional_value<T>::valid()){
                reinterpret_cast<T*>(value_mem)->~T();
            }
            fn_::optional_value<T>::value = nullptr;
        }
        return *this;
    }

    optional& operator=(optional const& other)
    {
        if(other.valid()){
            if(fn_::optional_value<T>::valid()){
                *fn_::optional_value<T>::value = *other.value;
            }
            else{
                new (value_mem) T{*other.value};
                fn_::optional_value<T>::value = reinterpret_cast<T*>(value_mem);
            }
        }
        else{
            if(fn_::optional_value<T>::valid()){
                reinterpret_cast<T*>(value_mem)->~T();
            }
            fn_::optional_value<T>::value = nullptr;
        }
        return *this;
    }

    optional(T const& v):
        fn_::optional_value<T>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{v};
    }

    optional(optional<T&> const& original):
        fn_::optional_value<T>(
            original.valid()
                ? reinterpret_cast<T*>(value_mem)
                : nullptr
        )
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T const&> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T const> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }
};

template<typename T>
class optional<T const> final : public fn_::optional_value<T const>
{
    using fn_::optional_value<T const>::value_mem;

    friend class optional<T>;

public:
    optional():
        fn_::optional_value<T const>()
    {}

    optional(T const& v):
        fn_::optional_value<T const>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{v};
    }

    optional(optional<T>&& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        if(original.valid()){
            new (value_mem) T{fn_::move(*original.value)};
            original.value = nullptr;
            reinterpret_cast<T*>(value_mem)->~T();
        }
    }

    optional(optional<T> const& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T&> const& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }
};

template<typename T>
class optional<T&> final : public fn_::optional_ref<T>
{
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T>()
    {}

    optional(T& v):
        fn_::optional_ref<T>(&v)
    {}

    optional(optional<T> const& original):
        fn_::optional_ref<T>(original.value)
    {}
};

template<typename T>
class optional<T const&> final : public fn_::optional_ref<T const>
{
    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T const>()
    {}

    optional(T const& v):
        fn_::optional_ref<T const>(&v)
    {}

    optional(optional<T> const& original):
        fn_::optional_ref<T const>(original.value)
    {}

    optional(optional<T&> const& original):
        fn_::optional_ref<T const>(original.value)
    {}
};

template<>
class optional<void> final
{
    bool no_value;
public:
    optional(): no_value(true) {}
    optional(bool): no_value(false) {}

    template<typename EmptyF>
    void operator||(EmptyF const& handle_no_value) const
    {
        if(no_value){
            return handle_no_value();
        }
    }
};

namespace fn_ {

/*
 *  Convert a functions return value to an optional.
 *  This is needed for optional handler chaining.
 *  In particular a specialization is implemented that
 *  avoids getting optional<optional<T>>.
 */
template<typename T>
struct return_cast {
    template<typename F,typename  V>
    static optional<T> func(F&& f,V&& v) {return f(v);}

    template<typename V>
    static T value(V&& v) {return v;}
};

template<>
struct return_cast<void> {
    template<typename F,typename  V>
    static
    optional<void> func(F&& f,V&& v) {f(v); return true;}

    template<typename V>
    static void value(V&&) {}
};

template<typename T>
struct return_cast<optional<T>> {

    template<typename F,typename  V>
    static
    optional<T> func(F&& f,V&& v) {return f(v);}

    template<typename V>
    static T value(V&& v) {return v;}
};

template<class T>
class Element
{
    T const i;

    template<typename C>
    class IT
    {
        C& container;
        unsigned int position;
        int step = 1;

    public:
        typedef T value_type;
        IT(C& container, T const& position):
            container(container),
            position(position)
        {}

        IT(IT const& o, int step):
            container(o.container),
            position(o.position),
            step(step)
        {}

        bool operator!=(IT const& other) const
        {
            return position < other.position;
        }

        IT const& operator++()
        {
            position += step;
            return *this;
        }

        auto operator*() const
            ->decltype(container[position])
        {
            return container[position];
        }
    };

    template<class Container>
    class Range
    {
        IT<Container> from;
        IT<Container> const to;
        int step = 1;

    public:
        Range(Container& c, T const& from, T const& to):
            from(c,from),
            to(c,to)
        {}

        Range(Range const& o, IT<Container> from):
            from(from),
            to(o.to),
            step(step)
        {}

        IT<Container> const& begin() const { return from; }
        IT<Container> const& end() const { return to; }

        Range by(int step)
        {
            return {*this,{from,step}};
        }
    };

    class UpperLimit
    {
        Element from;
        Element to{0};
    public:
        UpperLimit(Element const from, Element const to):
            from(from),
            to(to)
        {}

        UpperLimit(Element const from):
            from(from)
        {}

        template<class Container>
        auto of(Container& c) const
            ->Range<Container>
        {
            auto const size = c.size();
            auto index = from.get_index(size);
            if(index > size){
                index = 0;
            }

            auto limit = to.i ? to.get_index(size) : size;
            if(limit > size){
                limit = size;
            }

            if(limit < index){
                limit = index = 0;
            }

            return Range<Container>(c,index,limit);
        }
    };

public:
    Element(T const i): i{i} {}

    template<typename S>
    S get_index(S const& size) const
    {
        return static_cast<S>(i<0?(size+i):i);
    }

    template<class Container>
    auto of(Container& c) const
        ->optional<decltype(c[0])&>
    {
        auto const size = c.size();
        auto index = get_index(size);

        if(index < size){
            return c[index];
        }
        else{
            return {};
        }

        (void)c.back(); // protect against using with std::map
    }

    UpperLimit to_last()
    {
        return UpperLimit(i);
    }

    UpperLimit to(Element limit)
    {
        return UpperLimit(i,limit);
    }


    template<class Container>
    auto in(Container& c) const
        ->optional<decltype(c.at(i))>
    {
        if(c.count(i)){
            return c.at(i);
        }
        else{
            return {};
        }
    }
};
}

/*
 * The following functions allow instantiation of the above classes,
 * without explicitly specifying the template arguments.
 * This is necessary, because function templates support
 * automatic type deduction by parameter type, while classes do not.
 */
template<typename T,typename ...Args>
auto range(T const& t,Args const& ...args) -> fn_::Range<T>
{
    return fn_::Range<T>(t,args...);
}

template<typename FN,typename G>
auto map(FN const& fn,G const& g)
    ->fn_::Map<FN,G,typename fn_::noconst<decltype(g.begin())>::T>
{
    return fn_::Map<FN,G,typename fn_::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename OtherIT>
auto as_range(OtherIT const& b, OtherIT const& e) -> fn_::AsRange<OtherIT>
{
    return fn_::AsRange<OtherIT>(b,e);
}

template<
    template<typename,typename> class PairT=fn_::Pair,
    typename A,
    typename B
>
auto zip(A&& a,B&& b)
    -> fn_::Zip<PairT,A,B, decltype(a.begin()), decltype(b.begin())>
{
    return fn_::Zip<PairT,A,B, decltype(a.begin()), decltype(b.begin())>(a,b);
}

template<typename A>
auto enumerate(A&& a) -> decltype(zip(range(-1),a))
{
    return zip(range(-1),a);
}

template<typename FN,typename G>
auto filter(FN const& fn,G const& g)
    ->fn_::Filter<FN,G, typename fn_::noconst<decltype(g.begin())>::T>
{
    return fn_::Filter<FN,G,typename fn_::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename G>
auto filter(G const& g)
    ->decltype(filter(fn_::IsTrue<decltype(*g.begin())>{},(*g.begin(),g)))
{
    return filter(fn_::IsTrue<decltype(*g.begin())>{},g);
}

template<class T>
fn_::Element<T> element(T const i)
{
    return fn_::Element<T>(i);
}

/*
 * Reduction is so simple, it does not need a helper class
 */
template<typename I,typename T, typename F>
T reduce(I const& iter, T const& neutral, F const& f)
{
    T v = neutral;
    for(auto const& x: iter){
        v = f(v,x);
    }
    return v;
}


}

/*
 * Some macros to make optional<T> usable with complex types.
 * As soon as generic lambdas can be used (C++14), these won't be
 * necessary anymore, and should not be used from that point onwards.
 */
#define FN_OTYPE(X) FN_TYPENAME fn::fn_::remove_reference<decltype(X)>::T::Type
#define use_(X) X >>[&](FN_OTYPE(X)&
#define _as(X) X)
#define with_(X) use_(X)_as(X)

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
