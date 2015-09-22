#include <iostream>
#include <vector>
#include <memory>
using namespace std;

#define USE_MOVE_CONSTRUCTOR

/*
    MSVC2013 - USE_MOVE_CONSTRUCTOR(O)           MSVC2013 - USE_MOVE_CONSTRUCTOR(X)             GCC - USE_MOVE_CONSTRUCTOR(O)               GCC - USE_MOVE_CONSTRUCTOR(X)
================================================================================================================================================================================
[Test01] Start                          .    [Test01] Start                          .    [Test01] Start                          .    [Test01] Start                          .
   Pramater Constructor  000000000024FC80       Pramater Constructor  000000000027FBE0       Pramater Constructor  0x7fff7e34e3e0       Pramater Constructor  0x7fffc531b900
   Move Constructor      000000000024FCB8       Copy Constructor      000000000027FC18       Destructor            0x7fff7e34e3e0       Destructor            0x7fffc531b900
   Destructor            000000000024FC80       Destructor            000000000027FBE0    -----------------------------------------    -----------------------------------------
   Destructor            000000000024FCB8       Destructor            000000000027FC18        
-----------------------------------------    -----------------------------------------    [Test02] Start                          .    [Test02] Start                          .
                                                                                             Pramater Constructor  0x7fff7e34e3e0       Pramater Constructor  0x7fffc531b900
[Test02] Start                          .    [Test02] Start                          .       Copy Constructor      0x7fff7e34e3d0       Copy Constructor      0x7fffc531b8f0
   Pramater Constructor  000000000024FC78       Pramater Constructor  000000000027FBD8       Destructor            0x7fff7e34e3d0       Destructor            0x7fffc531b8f0
   Copy Constructor      000000000024FCC8       Copy Constructor      000000000027FC28       Destructor            0x7fff7e34e3e0       Destructor            0x7fffc531b900
   Destructor            000000000024FCC8       Destructor            000000000027FC28    -----------------------------------------    -----------------------------------------
   Destructor            000000000024FC78       Destructor            000000000027FBD8        
-----------------------------------------    -----------------------------------------    [Test03] Start                          .    [Test03] Start                          .
                                                                                             Basic Constructor     0x7fff7e34e3f0       Basic Constructor     0x7fffc531b910
[Test03] Start                          .    [Test03] Start                          .       Basic Constructor     0x7fff7e34e3e0       Basic Constructor     0x7fffc531b900
   Basic Constructor     000000000024FCB8       Basic Constructor     000000000027FC18       Equal Operator        0x7fff7e34e3e0       Equal Operator        0x7fffc531b900
   Basic Constructor     000000000024FD08       Basic Constructor     000000000027FC68       Destructor            0x7fff7e34e3e0       Destructor            0x7fffc531b900
   Equal Operator        000000000024FD08       Equal Operator        000000000027FC68       Destructor            0x7fff7e34e3f0       Destructor            0x7fffc531b910
   Destructor            000000000024FD08       Destructor            000000000027FC68    -----------------------------------------    -----------------------------------------
   Destructor            000000000024FCB8       Destructor            000000000027FC18        
-----------------------------------------    -----------------------------------------    [Test04] Start                          .    [Test04] Start                          .
                                                                                             Pramater Constructor  0x7fff7e34e3e0       Pramater Constructor  0x7fffc531b900
[Test04] Start                          .    [Test04] Start                          .       Destructor            0x7fff7e34e3e0       Destructor            0x7fffc531b900
   Pramater Constructor  000000000024FCC8       Pramater Constructor  000000000027FC28    -----------------------------------------    -----------------------------------------
   Move Constructor      000000000024FC88       Copy Constructor      000000000027FBE8        
   Destructor            000000000024FCC8       Destructor            000000000027FC28    [Test05] Start                          .    [Test05] Start                          .
   Destructor            000000000024FC88       Destructor            000000000027FBE8       Pramater Constructor  0x7fff7e34e3e0       Pramater Constructor  0x7fffc531b900
-----------------------------------------    -----------------------------------------       Member Function       0x7fff7e34e3e0       Member Function       0x7fffc531b900
                                                                                             Copy Constructor      0x7fff7e34e3d0       Copy Constructor      0x7fffc531b8f0
[Test05] Start                          .    [Test05] Start                          .       Destructor            0x7fff7e34e3e0       Destructor            0x7fffc531b900
   Pramater Constructor  000000000024FCC8       Pramater Constructor  000000000027FC28       Destructor            0x7fff7e34e3d0       Destructor            0x7fffc531b8f0
   Member Function       000000000024FCC8       Member Function       000000000027FC28    -----------------------------------------    -----------------------------------------
   Copy Constructor      000000000024FC88       Copy Constructor      000000000027FBE8        
   Destructor            000000000024FCC8       Destructor            000000000027FC28    [Test06] Start                          .    [Test06] Start                          .
   Destructor            000000000024FC88       Destructor            000000000027FBE8       Basic Constructor     0x7fff7e34e3f0       Basic Constructor     0x7fffc531b910
-----------------------------------------    -----------------------------------------       Basic Constructor     0x7fff7e34e3e0       Basic Constructor     0x7fffc531b900
                                                                                             Move Operator         0x7fff7e34e3f0       Equal Operator        0x7fffc531b910
[Test06] Start                          .    [Test06] Start                          .       Destructor            0x7fff7e34e3e0       Destructor            0x7fffc531b900
   Basic Constructor     000000000024FCB8       Basic Constructor     000000000027FC18       Destructor            0x7fff7e34e3f0       Destructor            0x7fffc531b910
   Basic Constructor     000000000024FD08       Basic Constructor     000000000027FC68    -----------------------------------------    -----------------------------------------
   Move Operator         000000000024FCB8       Equal Operator        000000000027FC18        
   Destructor            000000000024FD08       Destructor            000000000027FC68        
   Destructor            000000000024FCB8       Destructor            000000000027FC18        
-----------------------------------------    -----------------------------------------        

*/

#define _cout cout << "   "
struct Obj
{
	int i;
	string s;
	shared_ptr<Obj> p;

	~Obj() {
		p.reset();
		_cout << "Destructor            " << this << endl;
	}
	Obj() {
		_cout << "Basic Constructor     " << this << endl;
	}
	Obj(int i, string s) {
		this->i = i;
		this->s = s;
		_cout << "Pramater Constructor  " << this << endl;
	}
	Obj(const Obj& o) {
		i = o.i;
		s = o.s;
		p = o.p;
		_cout << "Copy Constructor      " << this << "   " << &o << endl;
	}
	Obj& operator=(const Obj& o) {
		i = o.i;
		s = o.s;
		p = o.p;
		_cout << "Substitute Operator   " << this << "   " << &o << endl;
		return *this;
	}
#ifdef USE_MOVE_CONSTRUCTOR
	Obj(Obj&& o) {
		std::swap(i, o.i);
		std::swap(s, o.s);
		std::swap(p, o.p);
		_cout << "Move Constructor      " << this << "   " << &o << endl;
	}
	Obj& operator=(Obj&& o) {
		std::swap(i, o.i);
		std::swap(s, o.s);
		std::swap(p, o.p);
		_cout << "Move Operator         " << this << "   " << &o << endl;
		return *this;
	}
#endif
	Obj& SomeFunc() {
		_cout << "Member Function       " << this << endl;
		return *this;
	}
};

#define TestStart(TestName)																			\
	{																								\
		cout << "\n[" << TestName << "] Start                                             .\n";		\
		{																							\

#define TestEnd																						\
			_cout << "exit scope\n";																\
		}																							\
		cout << "------------------------------------------------------------\n";					\
	}																								\



Obj Func1() {
	_cout << "init...\n";
	Obj temp;
	temp.p.reset(new Obj);
	_cout << "return...\n";
	return temp;
}

std::vector<Obj> Func2() {
	_cout << "init...\n";
	std::vector<Obj> v;
	v.resize(3);
	for (auto& o : v)
		o.p.reset(new Obj);
	_cout << "return...\n";
	return v;
}


int main() {
	TestStart("Test01")
		Obj o(Obj(10, "Param"));
	TestEnd


	TestStart("Test02")
		Obj o1(11, "Param");
		Obj o2 = o1;
	TestEnd


	TestStart("Test03")
		Obj o1;
		Obj o2;
		o2 = o1;
	TestEnd


	TestStart("Test04")
		Obj o = Obj(12, "Param");
	TestEnd


	TestStart("Test05")
		Obj o = Obj(12, "Param").SomeFunc();
	TestEnd


	TestStart("Test06")
		Obj o1;
		Obj o2;
		o1 = std::move(o2);
	TestEnd


	TestStart("Test07")
		Obj o;
		o.p.reset(new Obj);
		o = Func1();
	TestEnd


	TestStart("Test08")
		std::vector<Obj> v;
		v.resize(1);
		v[0].p.reset(new Obj);
		v = Func2();
	TestEnd

	return 0;
}
