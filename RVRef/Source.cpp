#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>
using namespace std;

#define USE_MOVE_OPERATION

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
#ifdef USE_MOVE_OPERATION
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
	PrintStep(__FUNCTION__);
	Obj temp;
	temp.p.reset(new Obj);
	PrintStep("return " __FUNCTION__);
	return temp;
}

std::vector<Obj> VectorReturnFunc2() {
	PrintStep(__FUNCTION__);
	std::vector<Obj> v;
	v.resize(3);
	for (auto& o : v)
		o.p.reset(new Obj);
	PrintStep("return " __FUNCTION__);
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
		auto func1 = [o]()
		{
			_cout << "call func1 " << o.i << "\n";
		};

		PrintStep(2);
		auto func2 = [mv = std::move(o)]() // C++14
		{
			_cout << "call func2 " << mv.i << "\n";
		};

		PrintStep(3);
		std::vector<Func> v;
		v.push_back(func1);

		PrintStep(4);
		v.push_back(func2);

		PrintStep(5);
		func1();
		func2();
	TestEnd


	TestStart("Test10")
		typedef std::function<void()> Func;
		Obj o(123, "");

		PrintStep(1);
		auto func = [mv = std::move(o)]() // C++14
		{
			_cout << "call func " << mv.i << "\n";
		};

		PrintStep(2);
		Func f1 = std::move(func);

		PrintStep(3);
		Func f2 = std::move(f1);

		PrintStep(4);
		Func f3 = f2;
		f3();
	TestEnd


	TestStart("Test11")
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
                 USE_MOVE_OPERATION (O)                                            USE_MOVE_OPERATION (X)
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
*/



/*
                                              GCC 4.8.3 20140911 (Red Hat 4.8.3-9)
                 USE_MOVE_OPERATION (O)                                            USE_MOVE_OPERATION (X)
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
*/



/*
                                                            clang 3.6
                 USE_MOVE_OPERATION (O)                                            USE_MOVE_OPERATION (X)
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
*/