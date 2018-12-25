#include "Vec.h"
#include "sharedPtr.h"
#include <ctype.h>
#include <cstring>

using namespace std;

//#define DEBUG

class Str
{
public:
    typedef Vec<char>::size_type size_type;
    typedef char& ref;
    typedef const char& const_ref;

friend  istream& operator>>(istream& is, Str& s);
private:
    Vec<char> string;
    char* pt;

public:
    Str(): pt(nullptr) {}
    Str(size_type n, char c): string(n,c), pt(nullptr) {}
    Str(const char* cp): pt(nullptr) { copy(cp, cp + strlen(cp), back_inserter(string));}
    template<typename In>
    Str(In begin, In end): pt(nullptr) { copy(begin, end, back_inserter(string));}

    ref at(size_type n);
    bool empty() const {return string.empty();}
    size_type size() const { return string.size();}
    const char* c_str() const {return pt;}; //返回的是一个临时指针

    bool clear() { string.clear(); pt == nullptr; }
    void push_back(const char c) { string.push_back(c); renew_pt();}
    void copy_to(char* dest, size_type n) { copy(string.begin(), string.end(), dest);}

    ref operator[](size_type n) {return string[n];}
    const_ref operator[](size_type n) const { return string[n]; }
    Str& operator+=(const Str& s);
    bool operator==(const Str& s);
    operator char*() {return pt; }
    operator const char*() const {return pt; }
    operator bool() { return !string.empty();}

private:
    void renew_pt();
};

/*相关操作的函数*/
istream& operator>>(istream& is, Str& s)
{
    s.clear();

    char c;
    while(is.get(c) && isspace(c))
        ;

    if(is)
    {
        do
        {
            s.push_back(c);
        } while (is.get(c) && !isspace(c));

        s.renew_pt();
    }


}


ostream& operator<<(ostream& os, const Str& s);
{
    for(size_type i = 0; i < s.size(); ++i)
        os << s[i];

    return os;
}


Str operator+(const Str& left, const Str& right)//通过隐式构造函数可以由const char* 转换成Str类型
{
    Str result = left;
    result += right;
    return result;
}


/* 公有成员函数的实现 */

Str::ref Str::at(size_type n)
{
    if(string.begin() + n >= string.end())
        throw "illegal position";

    return string[n];
}


Str& Str::operator+=(const Str& s)
{
    copy(s.string.begin(), s.string.end(), back_inserter(s));
    renew_pt();
    return *this;
}


bool Str::operator==(const Str& s)
{
    if (size() != s.size())
        return false;

    for(size_t i = 0; i < size(); ++i)
    {
        if(string[i] != s[i])
            return false;
    }

    return true;
}


/* 私有成员函数的实现 */

void Str::renew_pt()
{
    size_type len = string.size();
    if (pt != nullptr)
        delete pt;

    pt = new char(len);
    copy(string.begin(), string.end(), pt);

}



/* 测试代码 */

#ifdef DEBUG

int main(int argc, char* argv[])
{
	{

	}

	_CrtDumpMemoryLeaks();
	//由于该检测手法实在main函数推出前检测，而此时各个类实例还没出作用域
	//所以一直会存在内存泄漏，解决方法就是多加一层括号
}

#endif // DEBUG


