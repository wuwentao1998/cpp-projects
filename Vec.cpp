#include <memory>
#include <iostream>
#include <algorithm>
#include <string>
#include <assert.h>
#include <crtdbg.h>

using namespace std;

#define DEBUG

template<typename T>
class Vec
{
public:
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef size_t size_type;
    typedef T& ref;
    typedef const T& const_ref;

private:
    iterator base;
    iterator limit;
    iterator avail;

    allocator<T> alloc; //由于new在分配内存的同时还执行了多余的默认初始化操作，因此我们用allocator类代替

public:
    Vec() { create();}
    explicit Vec(size_type n, const_ref val = T()) { create(n, val); }
    Vec(const Vec& v) { create(v.begin(), v.end());}
    Vec& operator=(const Vec& v);
    ~Vec();

    bool empty() const { return base == avail;}
    iterator begin() { return base; }
    const_iterator begin() const { return base; }
    iterator end() { return avail; }
    const_iterator end() const { return avail; }
    size_type size() const { return avail - base; }
    size_type capacity() const { return limit - base; }
    const_ref front() const { return *base; }
    const_ref back() const { return *(avail- 1); }


    void push_back(const_ref val);
    void clear();
    iterator insert(iterator pos, const_ref val);
    template<typename In>
    iterator insert(iterator pos, In first, In last);
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);
    ref at(size_type n);

    ref operator[](size_type n) {return base[n]; }
    const_ref operator[](size_type n) const { return base[n]; }
    bool operator==(const Vec& v);



private:
    void create(size_type n = 0, const_ref val = T());
    void create(const_iterator begin, const_iterator end);
    void del();
    void grow();
};



/* 公有成员函数的实现 */

template<typename T>
Vec<T>& Vec<T>::operator=(const Vec& v) //类的作用域运算符之后才不用显式声明<T>
{
    if( &v != this)
    {
        del(); //如果不加判断，那么当传入对象是自身的时候，在执行删除操作后，传入的v其实也被删除了
        create(v.begin(), v.end());
    }

    return *this;
}


template<typename T>
Vec<T>::~Vec()
{
    del();
}


template<typename T>
void Vec<T>::push_back(const_ref val)
{
    if (avail + 1 > limit)
        grow();

    alloc.construct(avail++, val); //avail指向最后一个元素的后一个地址
}


template<typename T>
void Vec<T>::clear()
{
    auto it = base;
    while(it != avail)
        alloc.destroy(it++);

    avail = base;
}


template<typename T>
typename Vec<T>::iterator Vec<T>::insert(iterator pos, const_ref val)
{
    // 要特别考虑空容器的情况
    if (pos == base && base == avail)
    {
        push_back(val);
        return pos;
    }

    if (pos >= avail)
        throw "illegal input iterator";

    if (avail + 1 > limit)
    {
        int offset = pos - base;
        grow();
        pos = base + offset;
    }

    auto temp = back();
    alloc.construct(avail++, temp);

    if (pos + 1 != avail)
    {
        for(auto i = avail - 3; i >= pos; --i)
            *(i+1) = *i;
    }

    *pos = val;
    return pos;
}


template<typename T>
template<typename In>
typename Vec<T>::iterator Vec<T>::insert(iterator pos, In first, In last)
{
    if ((base != avail && pos >= avail) || last <= first)
        throw "illegal input iterator";

    if (base == avail && pos == avail)
    {
        while (first != last)
        {
            push_back(*first);
            first++;
        }
        return pos;
    }

    size_type add = last - first;

    if (avail + add > limit)
    {
        int offset = pos - base; // 因为grow后pos迭代器会失效，所以讲迭代器转换为偏移量
        grow();
        pos = base + offset;
    }

    if (pos + 1 == avail)
    {
        uninitialized_copy(first, last, avail);
        avail += add;
        return pos;
    }

    // 分类讨论
    size_type remains = avail - pos;
    if (remains > add)
    {
        uninitialized_copy(avail - remains, avail, avail);
        copy_backward(pos, avail - remains, avail); // 逆序复制元素
        copy(first, last, pos);
    }
    else
    {
        uninitialized_copy(pos, pos + add, avail + add - remains);
        copy(first, first + remains, pos);
        uninitialized_copy(first + remains, last, avail);
    }

    avail += add;
    return pos;
}


template<typename T>
typename Vec<T>::iterator Vec<T>::erase(iterator pos)
{
    alloc.destroy(pos);
    for(auto it = pos + 1 ; it < avail; ++it)
        *(it - 1) = *it;

    --avail;

    return pos;
}


template<typename T>
typename Vec<T>::iterator Vec<T>::erase(iterator first, iterator last)
{
    if (!(first >= base && last < avail))
        throw "illegal input iterator";

    for(auto it = first; it != last;++it)
        alloc.destroy(it);

    size_type minus = last - first;

    for (auto it = last; it < avail; ++it)
        *(it - minus) = *it;

    avail -= minus;

    return first;
}


template<typename T>
typename Vec<T>::ref Vec<T>::at(size_type n)
{
    if(base + n >= avail)
        throw "illegal position";

    return base[n];
}


template<typename T>
bool Vec<T>::operator==(const Vec& v)
{
    if (size() != v.size())
        return false;

    for(size_t i = 0; i < size(); ++i)
    {
        if(base[i] != v[i])
            return false;
    }

    return true;
}



/* 私有成员函数的实现 */

template<typename T>
void Vec<T>::create(size_type n , const_ref val)
{
    if (n == 0)
        base = limit = avail = nullptr;
    else
    {
        base = alloc.allocate(n); //因为alloc本身已经声明过类型了(T)，所以这里只需用n而不是n*sizeof(T)
        limit = avail = base + n;
        uninitialized_fill(base, limit, val);
    }
}


template<typename T>
void Vec<T>::create(const_iterator begin, const_iterator end)
{
    base = alloc.allocate(end - begin);
    limit = avail = uninitialized_copy(begin, end, base);
}


template<typename T>
void Vec<T>::del()
{
    if(base != nullptr)
    {
        iterator it = limit;
        while(it != base)
            alloc.destroy(--it); //destory实际上就是运行了类的析构函数，如果类中存在指针，那么缺少这一步会造成内存泄漏

        alloc.deallocate(base, limit - base);
        base = limit = avail = nullptr;
    }
}


template<typename T>
void Vec<T>::grow()
{
    size_type new_size = (base == limit) ? 1 : 2*(limit - base);
    iterator new_base = alloc.allocate(new_size);
    iterator new_avail = uninitialized_copy(base, avail, new_base);

    del();

    base = new_base;
    avail = new_avail;
    limit = base + new_size;
}


/* 测试代码 */

#ifdef DEBUG

int main(int argc, char* argv[])
{
	{
        Vec<int> v1;
        Vec<string> v2(5, "hape");
        Vec<string> v3(v2);
        Vec<int> v4 = v1;

        assert(v2.size() == 5);
        v4.push_back(3);
        assert(!v4.empty());
        assert(v3.front() == "hape");
        v2.clear();
        assert(v2.empty());
        v4.insert(v4.begin(), 2);
        v4.insert(v4.begin(), 1);
        assert(v4[0] == 1 && v4[2] == 3);
        v1.insert(v1.begin(), v4.begin(), v4.end());
        assert(v1 == v4);
        v1.erase(v1.begin());
        assert(v1.front() == 2);
        v4.erase(v4.begin(), v4.begin() + 2);
        assert(v4.front() == 3);
	}

	_CrtDumpMemoryLeaks();
}

#endif // DEBUG



