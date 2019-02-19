// 这里的实现不是标准库中的实现，标准库中的成员变量应该是一个char* 指针和长度变量

#include "Vec.cpp"
#include "sharedPtr.cpp"
#include <ctype.h>
#include <cstring>
#include <crtdbg.h>
#include <assert.h>

using namespace std;

#define DEBUG

class Str
{
public:
    typedef Vec<char>::size_type size_type;
    typedef char& ref;
    typedef const char& const_ref;
    typedef Vec<char>::iterator iter;
    typedef Vec<char>::const_iterator const_iter;

friend  istream& operator>>(istream& is, Str& s);
private:
    Vec<char> string;
    char* pt;

public:
    Str(): pt(nullptr) {}
    Str(size_type n, char c): string(n,c), pt(nullptr) {}
    Str(const char* cp): pt(nullptr) { string.insert(string.end(), cp, cp +strlen(cp));}
    template<typename In>
    Str(In begin, In end): pt(nullptr) { string.insert(string.end(), begin, end);}

    ref at(size_type n);
    bool empty() const {return string.empty();}
    size_type size() const { return string.size();}
    const char* c_str() {renew_pt(); return pt;}; //返回的是一个临时指针
    iter begin() { return string.begin();}
    const_iter begin() const { return string.begin();} // 如果不加const后缀则无法重载，因为参数是一样的
    iter end() { return string.end();}
    const_iter end() const { return string.end();}

    void clear() { string.clear(); pt = nullptr; }
    void push_back(const char c) { string.push_back(c);}
    void copy_to(char* dest) { copy(string.begin(), string.end(), dest);}

    ref operator[](size_type n) {return string[n];}
    const_ref operator[](size_type n) const { return string[n]; }
    Str& operator+=(const Str& s);
    bool operator==(const Str& s);
    operator const char*() {renew_pt(); return pt; }
    operator bool() { return !string.empty();}
private:
    void renew_pt();
};

/*相关操作的函数*/
// is >> s 等价于 is.operator>>(s),是is被重载的运算符，因此只能用友元函数形式实现
// 如果用成员函数实现，则操作形式为s.operator>>(cin),等价于s>>cin，与习惯操作不同
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
    }

    return is;
}


ostream& operator<<(ostream& os, const Str& s)
{
    for(size_t i = 0; i < s.size(); ++i)
        os << s[i];

    return os;
}

// 由于加号的左边可能不是string类，此时我们仍要支持加法操作，因此只能通过非成员函数实现
// 如果连续多个加号，则此方法需要生成许多临时变量，效率很低，因此这种的string是用别的很繁琐的方式实现的
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
    string.insert(string.end(), s.string.begin(), s.string.end());
    //copy(s.string.begin(), s.string.end(), back_inserter(string));
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

    pt = new char(len + 1);
    copy(string.begin(), string.end(), pt);
    pt[len] = '\0';
}



/* 测试代码 */

#ifdef DEBUG

int main(int argc, char* argv[])
{
	{
        Str str1(5, 'c');
        Str str2 = "abcdefg";
        Str str3(str2.begin(), str2.end());
        assert(str1.size() == 5);
        assert(str3.size() == 7);
        assert(strcmp(str2.c_str(), "abcdefg") == 0);
        str1.clear();
        assert(str1.empty());
        str1.push_back('d');
        char dest[100];
        str2.copy_to(dest);
        assert(strcmp(dest, "abcdefg") == 0);
        str3[0] = 'h';
        assert(strcmp(str3.c_str(), "hbcdefg") == 0);
        str2 += "123";
        assert(strcmp(str2.c_str(), "abcdefg123") == 0);
        assert(str2 == str2);
        cout << str2;
	}

	_CrtDumpMemoryLeaks();
	//由于该检测手法实在main函数推出前检测，而此时各个类实例还没出作用域
	//所以一直会存在内存泄漏，解决方法就是多加一层括号
}

#endif // DEBUG


