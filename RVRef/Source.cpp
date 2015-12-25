#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>
using namespace std;

#define USE_MOVE_CONSTRUCTOR

#define _cout cout << "   "
template <typename T>
void PrintStep(T n)
{
	_cout << "(Step " << n << ")\n";
}

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
		cout << "\n[" << TestName << "] Start\n";													\
		{																							\

#define TestEnd																						\
			_cout << "(exit scope)\n";																\
		}																							\
		cout << "------------------------------------------------------------\n";					\
	}																								\



Obj VectorReturnFunc1() {
	_cout << "(" << __FUNCTION__ << ")\n";
	Obj temp;
	temp.p.reset(new Obj);
	_cout << "(return " << __FUNCTION__ << ")\n";
	return temp;
}

std::vector<Obj> VectorReturnFunc2() {
	_cout << "(" << __FUNCTION__ << ")\n";
	std::vector<Obj> v;
	v.resize(3);
	for (auto& o : v)
		o.p.reset(new Obj);
	_cout << "(return " << __FUNCTION__ << ")\n";
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

		PrintStep(1);
		o.p.reset(new Obj);

		PrintStep(2);
		o = VectorReturnFunc1();
	TestEnd


	TestStart("Test08")
		std::vector<Obj> v;

		PrintStep(1);
		v.resize(1);

		PrintStep(2);
		v[0].p.reset(new Obj);

		PrintStep(3);
		v = VectorReturnFunc2();
	TestEnd


	TestStart("Test09")
		typedef std::function<void()> Func;
		Obj o(123, "");

		PrintStep(1);
		auto func = [o]()
		{
			_cout << "call func " << o.i << "\n";
		};

		PrintStep(2);
		std::vector<Func> v;
		v.push_back(func);

		PrintStep(3);
		func();
	TestEnd


	TestStart("Test10")
		typedef std::tuple<Obj, Obj, Obj> Tuple;

		PrintStep(1);
		Tuple t1;

		PrintStep(2);
		Tuple t2 = std::move(t1);
	TestEnd
	return 0;
}

/*
                                                        Visual Studio 2015
                 USE_MOVE_CONSTRUCTOR (O)                                            USE_MOVE_CONSTRUCTOR (X)
=================================================================================================================================

[Test01] Start                                                  	[Test01] Start
   Pramater Constructor  000000000028E6E0                       	   Pramater Constructor  00000000002DE8E0
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028E6E0                       	   Destructor            00000000002DE8E0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test02] Start                                                  	[Test02] Start
   Pramater Constructor  000000000028E740                       	   Pramater Constructor  00000000002DE940
   Copy Constructor      000000000028E7A0   000000000028E740    	   Copy Constructor      00000000002DE9A0   00000000002DE940
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028E7A0                       	   Destructor            00000000002DE9A0
   Destructor            000000000028E740                       	   Destructor            00000000002DE940
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test03] Start                                                  	[Test03] Start
   Basic Constructor     000000000028E800                       	   Basic Constructor     00000000002DEA00
   Basic Constructor     000000000028E860                       	   Basic Constructor     00000000002DEA60
   Substitute Operator   000000000028E860   000000000028E800    	   Substitute Operator   00000000002DEA60   00000000002DEA00
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028E860                       	   Destructor            00000000002DEA60
   Destructor            000000000028E800                       	   Destructor            00000000002DEA00
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test04] Start                                                  	[Test04] Start
   Pramater Constructor  000000000028E8C0                       	   Pramater Constructor  00000000002DEAC0
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028E8C0                       	   Destructor            00000000002DEAC0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test05] Start                                                  	[Test05] Start
   Pramater Constructor  000000000028F340                       	   Pramater Constructor  00000000002DF540
   Member Function       000000000028F340                       	   Member Function       00000000002DF540
   Copy Constructor      000000000028E920   000000000028F340    	   Copy Constructor      00000000002DEB20   00000000002DF540
   Destructor            000000000028F340                       	   Destructor            00000000002DF540
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028E920                       	   Destructor            00000000002DEB20
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test06] Start                                                  	[Test06] Start
   Basic Constructor     000000000028E980                       	   Basic Constructor     00000000002DEB80
   Basic Constructor     000000000028E9E0                       	   Basic Constructor     00000000002DEBE0
   Move Operator         000000000028E980   000000000028E9E0    	   Substitute Operator   00000000002DEB80   00000000002DEBE0
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028E9E0                       	   Destructor            00000000002DEBE0
   Destructor            000000000028E980                       	   Destructor            00000000002DEB80
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test07] Start                                                  	[Test07] Start
   Basic Constructor     000000000028EA40                       	   Basic Constructor     00000000002DEC40
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     00000000000BE570                       	   Basic Constructor     00000000003CE570
   (Step 2)                                                     	   (Step 2)
   (VectorReturnFunc1)                                          	   (VectorReturnFunc1)
   Basic Constructor     000000000028E4F0                       	   Basic Constructor     00000000002DE6F0
   Basic Constructor     00000000000BE5F0                       	   Basic Constructor     00000000003CE5F0
   (return VectorReturnFunc1)                                   	   (return VectorReturnFunc1)
   Move Constructor      000000000028F440   000000000028E4F0    	   Copy Constructor      00000000002DF640   00000000002DE6F0
   Destructor            000000000028E4F0                       	   Destructor            00000000002DE6F0
   Move Operator         000000000028EA40   000000000028F440    	   Destructor            00000000003CE570
   Destructor            00000000000BE570                       	   Substitute Operator   00000000002DEC40   00000000002DF640
   Destructor            000000000028F440                       	   Destructor            00000000002DF640
   (exit scope)                                                 	   (exit scope)
   Destructor            00000000000BE5F0                       	   Destructor            00000000003CE5F0
   Destructor            000000000028EA40                       	   Destructor            00000000002DEC40
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test08] Start                                                  	[Test08] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     00000000000BE570                       	   Basic Constructor     00000000003CE570
   (Step 2)                                                     	   (Step 2)
   Basic Constructor     00000000000BE5F0                       	   Basic Constructor     00000000003CE5F0
   (Step 3)                                                     	   (Step 3)
   (VectorReturnFunc2)                                          	   (VectorReturnFunc2)
   Basic Constructor     00000000000BF670                       	   Basic Constructor     00000000003CF670
   Basic Constructor     00000000000BF6B0                       	   Basic Constructor     00000000003CF6B0
   Basic Constructor     00000000000BF6F0                       	   Basic Constructor     00000000003CF6F0
   Basic Constructor     00000000000BF770                       	   Basic Constructor     00000000003CF770
   Basic Constructor     00000000000BF7F0                       	   Basic Constructor     00000000003CF7F0
   Basic Constructor     00000000000BF8A0                       	   Basic Constructor     00000000003CF8A0
   (return VectorReturnFunc2)                                   	   (return VectorReturnFunc2)
   Destructor            00000000000BE5F0                       	   Destructor            00000000003CE5F0
   Destructor            00000000000BE570                       	   Destructor            00000000003CE570
   (exit scope)                                                 	   (exit scope)
   Destructor            00000000000BF770                       	   Destructor            00000000003CF770
   Destructor            00000000000BF670                       	   Destructor            00000000003CF670
   Destructor            00000000000BF7F0                       	   Destructor            00000000003CF7F0
   Destructor            00000000000BF6B0                       	   Destructor            00000000003CF6B0
   Destructor            00000000000BF8A0                       	   Destructor            00000000003CF8A0
   Destructor            00000000000BF6F0                       	   Destructor            00000000003CF6F0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test09] Start                                                  	[Test09] Start
   Pramater Constructor  000000000028EAE0                       	   Pramater Constructor  00000000002DECE0
   (Step 1)                                                     	   (Step 1)
   Copy Constructor      000000000028EB40   000000000028EAE0    	   Copy Constructor      00000000002DED40   00000000002DECE0
   (Step 2)                                                     	   (Step 2)
   Copy Constructor      000000000028F5D8   000000000028EB40    	   Copy Constructor      00000000002DF7D8   00000000002DED40
   Move Constructor      00000000000BE578   000000000028F5D8    	   Copy Constructor      00000000003CE578   00000000002DF7D8
   Destructor            000000000028F5D8                       	   Destructor            00000000002DF7D8
   (Step 3)                                                     	   (Step 3)
   call func 123                                                	   call func 123
   (exit scope)                                                 	   (exit scope)
   Destructor            00000000000BE578                       	   Destructor            00000000003CE578
   Destructor            000000000028EB40                       	   Destructor            00000000002DED40
   Destructor            000000000028EAE0                       	   Destructor            00000000002DECE0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test10] Start                                                  	[Test10] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     000000000028EBE0                       	   Basic Constructor     00000000002DEDE0
   Basic Constructor     000000000028EC20                       	   Basic Constructor     00000000002DEE20
   Basic Constructor     000000000028EC60                       	   Basic Constructor     00000000002DEE60
   (Step 2)                                                     	   (Step 2)
   Move Constructor      000000000028ECC0   000000000028EBE0    	   Copy Constructor      00000000002DEEC0   00000000002DEDE0
   Move Constructor      000000000028ED00   000000000028EC20    	   Copy Constructor      00000000002DEF00   00000000002DEE20
   Move Constructor      000000000028ED40   000000000028EC60    	   Copy Constructor      00000000002DEF40   00000000002DEE60
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000028ED40                       	   Destructor            00000000002DEF40
   Destructor            000000000028ED00                       	   Destructor            00000000002DEF00
   Destructor            000000000028ECC0                       	   Destructor            00000000002DEEC0
   Destructor            000000000028EC60                       	   Destructor            00000000002DEE60
   Destructor            000000000028EC20                       	   Destructor            00000000002DEE20
   Destructor            000000000028EBE0                       	   Destructor            00000000002DEDE0
------------------------------------------------------------    	------------------------------------------------------------
*/



/*
                                              GCC 4.8.3 20140911 (Red Hat 4.8.3-9)
                 USE_MOVE_CONSTRUCTOR (O)                                            USE_MOVE_CONSTRUCTOR (X)
=================================================================================================================================

[Test01] Start                                                  	[Test01] Start
   Pramater Constructor  0x7ffd825ad0e0                         	   Pramater Constructor  0x7ffecb11ad70
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test02] Start                                                  	[Test02] Start
   Pramater Constructor  0x7ffd825ad080                         	   Pramater Constructor  0x7ffecb11ad10
   Copy Constructor      0x7ffd825ad0e0   0x7ffd825ad080        	   Copy Constructor      0x7ffecb11ad70   0x7ffecb11ad10
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
   Destructor            0x7ffd825ad080                         	   Destructor            0x7ffecb11ad10
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test03] Start                                                  	[Test03] Start
   Basic Constructor     0x7ffd825ad080                         	   Basic Constructor     0x7ffecb11ad10
   Basic Constructor     0x7ffd825ad0e0                         	   Basic Constructor     0x7ffecb11ad70
   Substitute Operator   0x7ffd825ad0e0   0x7ffd825ad080        	   Substitute Operator   0x7ffecb11ad70   0x7ffecb11ad10
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
   Destructor            0x7ffd825ad080                         	   Destructor            0x7ffecb11ad10
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test04] Start                                                  	[Test04] Start
   Pramater Constructor  0x7ffd825ad0e0                         	   Pramater Constructor  0x7ffecb11ad70
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test05] Start                                                  	[Test05] Start
   Pramater Constructor  0x7ffd825ad1b0                         	   Pramater Constructor  0x7ffecb11ae40
   Member Function       0x7ffd825ad1b0                         	   Member Function       0x7ffecb11ae40
   Copy Constructor      0x7ffd825ad0e0   0x7ffd825ad1b0        	   Copy Constructor      0x7ffecb11ad70   0x7ffecb11ae40
   Destructor            0x7ffd825ad1b0                         	   Destructor            0x7ffecb11ae40
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test06] Start                                                  	[Test06] Start
   Basic Constructor     0x7ffd825ad080                         	   Basic Constructor     0x7ffecb11ad10
   Basic Constructor     0x7ffd825ad0e0                         	   Basic Constructor     0x7ffecb11ad70
   Move Operator         0x7ffd825ad080   0x7ffd825ad0e0        	   Substitute Operator   0x7ffecb11ad10   0x7ffecb11ad70
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
   Destructor            0x7ffd825ad080                         	   Destructor            0x7ffecb11ad10
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test07] Start                                                  	[Test07] Start
   Basic Constructor     0x7ffd825ad0e0                         	   Basic Constructor     0x7ffecb11ad70
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x1abb010                              	   Basic Constructor     0x2144010
   (Step 2)                                                     	   (Step 2)
   (VectorReturnFunc1)                                          	   (VectorReturnFunc1)
   Basic Constructor     0x7ffd825ad1e0                         	   Basic Constructor     0x7ffecb11ae70
   Basic Constructor     0x1abb060                              	   Basic Constructor     0x2144060
   (return VectorReturnFunc1)                                   	   (return VectorReturnFunc1)
   Move Operator         0x7ffd825ad0e0   0x7ffd825ad1e0        	   Destructor            0x2144010
   Destructor            0x1abb010                              	   Substitute Operator   0x7ffecb11ad70   0x7ffecb11ae70
   Destructor            0x7ffd825ad1e0                         	   Destructor            0x7ffecb11ae70
   (exit scope)                                                 	   (exit scope)
   Destructor            0x1abb060                              	   Destructor            0x2144060
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test08] Start                                                  	[Test08] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x1abb060                              	   Basic Constructor     0x2144060
   (Step 2)                                                     	   (Step 2)
   Basic Constructor     0x1abb010                              	   Basic Constructor     0x2144010
   (Step 3)                                                     	   (Step 3)
   (VectorReturnFunc2)                                          	   (VectorReturnFunc2)
   Basic Constructor     0x1abb0b0                              	   Basic Constructor     0x21440b0
   Basic Constructor     0x1abb0d0                              	   Basic Constructor     0x21440d0
   Basic Constructor     0x1abb0f0                              	   Basic Constructor     0x21440f0
   Basic Constructor     0x1abb120                              	   Basic Constructor     0x2144120
   Basic Constructor     0x1abb150                              	   Basic Constructor     0x2144150
   Basic Constructor     0x1abb1a0                              	   Basic Constructor     0x21441a0
   (return VectorReturnFunc2)                                   	   (return VectorReturnFunc2)
   Destructor            0x1abb010                              	   Destructor            0x2144010
   Destructor            0x1abb060                              	   Destructor            0x2144060
   (exit scope)                                                 	   (exit scope)
   Destructor            0x1abb120                              	   Destructor            0x2144120
   Destructor            0x1abb0b0                              	   Destructor            0x21440b0
   Destructor            0x1abb150                              	   Destructor            0x2144150
   Destructor            0x1abb0d0                              	   Destructor            0x21440d0
   Destructor            0x1abb1a0                              	   Destructor            0x21441a0
   Destructor            0x1abb0f0                              	   Destructor            0x21440f0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test09] Start                                                  	[Test09] Start
   Pramater Constructor  0x7ffd825ad080                         	   Pramater Constructor  0x7ffecb11ad10
   (Step 1)                                                     	   (Step 1)
   Copy Constructor      0x7ffd825ad0e0   0x7ffd825ad080        	   Copy Constructor      0x7ffecb11ad70   0x7ffecb11ad10
   (Step 2)                                                     	   (Step 2)
   Copy Constructor      0x7ffd825ad250   0x7ffd825ad0e0        	   Copy Constructor      0x7ffecb11aee0   0x7ffecb11ad70
   Move Constructor      0x1abb1a0   0x7ffd825ad250             	   Copy Constructor      0x21441a0   0x7ffecb11aee0
   Destructor            0x7ffd825ad250                         	   Destructor            0x7ffecb11aee0
   (Step 3)                                                     	   (Step 3)
   call func 123                                                	   call func 123
   (exit scope)                                                 	   (exit scope)
   Destructor            0x1abb1a0                              	   Destructor            0x21441a0
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
   Destructor            0x7ffd825ad080                         	   Destructor            0x7ffecb11ad10
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test10] Start                                                  	[Test10] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x7ffd825ad080                         	   Basic Constructor     0x7ffecb11ad10
   Basic Constructor     0x7ffd825ad0a0                         	   Basic Constructor     0x7ffecb11ad30
   Basic Constructor     0x7ffd825ad0c0                         	   Basic Constructor     0x7ffecb11ad50
   (Step 2)                                                     	   (Step 2)
   Move Constructor      0x7ffd825ad0e0   0x7ffd825ad080        	   Copy Constructor      0x7ffecb11ad70   0x7ffecb11ad10
   Move Constructor      0x7ffd825ad100   0x7ffd825ad0a0        	   Copy Constructor      0x7ffecb11ad90   0x7ffecb11ad30
   Move Constructor      0x7ffd825ad120   0x7ffd825ad0c0        	   Copy Constructor      0x7ffecb11adb0   0x7ffecb11ad50
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7ffd825ad120                         	   Destructor            0x7ffecb11adb0
   Destructor            0x7ffd825ad100                         	   Destructor            0x7ffecb11ad90
   Destructor            0x7ffd825ad0e0                         	   Destructor            0x7ffecb11ad70
   Destructor            0x7ffd825ad0c0                         	   Destructor            0x7ffecb11ad50
   Destructor            0x7ffd825ad0a0                         	   Destructor            0x7ffecb11ad30
   Destructor            0x7ffd825ad080                         	   Destructor            0x7ffecb11ad10
------------------------------------------------------------    	------------------------------------------------------------
*/



/*
                                                            clang 3.6
                 USE_MOVE_CONSTRUCTOR (O)                                            USE_MOVE_CONSTRUCTOR (X)
=================================================================================================================================

[Test01] Start                                                  	[Test01] Start
   Pramater Constructor  0x7fff4b7d3b50                         	   Pramater Constructor  0x7fff7bf8ec80
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d3b50                         	   Destructor            0x7fff7bf8ec80
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test02] Start                                                  	[Test02] Start
   Pramater Constructor  0x7fff4b7d3b08                         	   Pramater Constructor  0x7fff7bf8ec38
   Copy Constructor      0x7fff4b7d3ac0   0x7fff4b7d3b08        	   Copy Constructor      0x7fff7bf8ebf0   0x7fff7bf8ec38
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d3ac0                         	   Destructor            0x7fff7bf8ebf0
   Destructor            0x7fff4b7d3b08                         	   Destructor            0x7fff7bf8ec38
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test03] Start                                                  	[Test03] Start
   Basic Constructor     0x7fff4b7d3a90                         	   Basic Constructor     0x7fff7bf8ebc0
   Basic Constructor     0x7fff4b7d3a60                         	   Basic Constructor     0x7fff7bf8eb90
   Substitute Operator   0x7fff4b7d3a60   0x7fff4b7d3a90        	   Substitute Operator   0x7fff7bf8eb90   0x7fff7bf8ebc0
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d3a60                         	   Destructor            0x7fff7bf8eb90
   Destructor            0x7fff4b7d3a90                         	   Destructor            0x7fff7bf8ebc0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test04] Start                                                  	[Test04] Start
   Pramater Constructor  0x7fff4b7d3a30                         	   Pramater Constructor  0x7fff7bf8eb60
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d3a30                         	   Destructor            0x7fff7bf8eb60
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test05] Start                                                  	[Test05] Start
   Pramater Constructor  0x7fff4b7d39b8                         	   Pramater Constructor  0x7fff7bf8eae8
   Member Function       0x7fff4b7d39b8                         	   Member Function       0x7fff7bf8eae8
   Copy Constructor      0x7fff4b7d39e8   0x7fff4b7d39b8        	   Copy Constructor      0x7fff7bf8eb18   0x7fff7bf8eae8
   Destructor            0x7fff4b7d39b8                         	   Destructor            0x7fff7bf8eae8
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d39e8                         	   Destructor            0x7fff7bf8eb18
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test06] Start                                                  	[Test06] Start
   Basic Constructor     0x7fff4b7d3970                         	   Basic Constructor     0x7fff7bf8eaa0
   Basic Constructor     0x7fff4b7d3940                         	   Basic Constructor     0x7fff7bf8ea70
   Move Operator         0x7fff4b7d3970   0x7fff4b7d3940        	   Substitute Operator   0x7fff7bf8eaa0   0x7fff7bf8ea70
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d3940                         	   Destructor            0x7fff7bf8ea70
   Destructor            0x7fff4b7d3970                         	   Destructor            0x7fff7bf8eaa0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test07] Start                                                  	[Test07] Start
   Basic Constructor     0x7fff4b7d3910                         	   Basic Constructor     0x7fff7bf8ea40
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x6bac20                               	   Basic Constructor     0xfcbc20
   (Step 2)                                                     	   (Step 2)
   (VectorReturnFunc1)                                          	   (VectorReturnFunc1)
   Basic Constructor     0x7fff4b7d38e0                         	   Basic Constructor     0x7fff7bf8ea10
   Basic Constructor     0x6bac90                               	   Basic Constructor     0xfcbc90
   (return VectorReturnFunc1)                                   	   (return VectorReturnFunc1)
   Move Operator         0x7fff4b7d3910   0x7fff4b7d38e0        	   Destructor            0xfcbc20
   Destructor            0x6bac20                               	   Substitute Operator   0x7fff7bf8ea40   0x7fff7bf8ea10
   Destructor            0x7fff4b7d38e0                         	   Destructor            0x7fff7bf8ea10
   (exit scope)                                                 	   (exit scope)
   Destructor            0x6bac90                               	   Destructor            0xfcbc90
   Destructor            0x7fff4b7d3910                         	   Destructor            0x7fff7bf8ea40
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test08] Start                                                  	[Test08] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x6bac90                               	   Basic Constructor     0xfcbc90
   (Step 2)                                                     	   (Step 2)
   Basic Constructor     0x6bac20                               	   Basic Constructor     0xfcbc20
   (Step 3)                                                     	   (Step 3)
   (VectorReturnFunc2)                                          	   (VectorReturnFunc2)
   Basic Constructor     0x6bad00                               	   Basic Constructor     0xfcbd00
   Basic Constructor     0x6bad30                               	   Basic Constructor     0xfcbd30
   Basic Constructor     0x6bad60                               	   Basic Constructor     0xfcbd60
   Basic Constructor     0x6bada0                               	   Basic Constructor     0xfcbda0
   Basic Constructor     0x6bade0                               	   Basic Constructor     0xfcbde0
   Basic Constructor     0x6bae50                               	   Basic Constructor     0xfcbe50
   (return VectorReturnFunc2)                                   	   (return VectorReturnFunc2)
   Destructor            0x6bac20                               	   Destructor            0xfcbc20
   Destructor            0x6bac90                               	   Destructor            0xfcbc90
   (exit scope)                                                 	   (exit scope)
   Destructor            0x6bae50                               	   Destructor            0xfcbe50
   Destructor            0x6bad60                               	   Destructor            0xfcbd60
   Destructor            0x6bade0                               	   Destructor            0xfcbde0
   Destructor            0x6bad30                               	   Destructor            0xfcbd30
   Destructor            0x6bada0                               	   Destructor            0xfcbda0
   Destructor            0x6bad00                               	   Destructor            0xfcbd00
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test09] Start                                                  	[Test09] Start
   Pramater Constructor  0x7fff4b7d3870                         	   Pramater Constructor  0x7fff7bf8e9a0
   (Step 1)                                                     	   (Step 1)
   Copy Constructor      0x7fff4b7d3828   0x7fff4b7d3870        	   Copy Constructor      0x7fff7bf8e958   0x7fff7bf8e9a0
   (Step 2)                                                     	   (Step 2)
   Copy Constructor      0x7fff4b7d37b0   0x7fff4b7d3828        	   Copy Constructor      0x7fff7bf8e8e0   0x7fff7bf8e958
   Move Constructor      0x6bada8   0x7fff4b7d37b0              	   Copy Constructor      0xfcbda8   0x7fff7bf8e8e0
   Destructor            0x7fff4b7d37b0                         	   Destructor            0x7fff7bf8e8e0
   (Step 3)                                                     	   (Step 3)
   call func 123                                                	   call func 123
   (exit scope)                                                 	   (exit scope)
   Destructor            0x6bada8                               	   Destructor            0xfcbda8
   Destructor            0x7fff4b7d3828                         	   Destructor            0x7fff7bf8e958
   Destructor            0x7fff4b7d3870                         	   Destructor            0x7fff7bf8e9a0
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test10] Start                                                  	[Test10] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x7fff4b7d3720                         	   Basic Constructor     0x7fff7bf8e850
   Basic Constructor     0x7fff4b7d3750                         	   Basic Constructor     0x7fff7bf8e880
   Basic Constructor     0x7fff4b7d3780                         	   Basic Constructor     0x7fff7bf8e8b0
   (Step 2)                                                     	   (Step 2)
   Move Constructor      0x7fff4b7d3690   0x7fff4b7d3720        	   Copy Constructor      0x7fff7bf8e7c0   0x7fff7bf8e850
   Move Constructor      0x7fff4b7d36c0   0x7fff4b7d3750        	   Copy Constructor      0x7fff7bf8e7f0   0x7fff7bf8e880
   Move Constructor      0x7fff4b7d36f0   0x7fff4b7d3780        	   Copy Constructor      0x7fff7bf8e820   0x7fff7bf8e8b0
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fff4b7d36f0                         	   Destructor            0x7fff7bf8e820
   Destructor            0x7fff4b7d36c0                         	   Destructor            0x7fff7bf8e7f0
   Destructor            0x7fff4b7d3690                         	   Destructor            0x7fff7bf8e7c0
   Destructor            0x7fff4b7d3780                         	   Destructor            0x7fff7bf8e8b0
   Destructor            0x7fff4b7d3750                         	   Destructor            0x7fff7bf8e880
   Destructor            0x7fff4b7d3720                         	   Destructor            0x7fff7bf8e850
------------------------------------------------------------    	------------------------------------------------------------
*/