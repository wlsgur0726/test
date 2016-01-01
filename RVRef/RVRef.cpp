#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>
using namespace std;

//#define USE_MOVE_OPERATION
#define ALLOW_COPY_OPERATION

#define _cout cout << "   "
#define PrintStep(n) _cout << "(Step " << n << ")\n"

struct Obj
{
	int i;
	string s;
	shared_ptr<Obj> p;

	~Obj() {
		p.reset();
		_cout << "Destructor            " << this << " (" << i << "," << s.c_str() << ")\n";
	}
	Obj() {
		_cout << "Basic Constructor     " << this << endl;
	}
	Obj(int i, string s) {
		this->i = i;
		this->s = s;
		_cout << "Pramater Constructor  " << this << " (" << i << "," << s.c_str() << ")\n";
	}
#ifdef ALLOW_COPY_OPERATION
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
#endif
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
	PrintStep("return " <<  __FUNCTION__);
	return temp;
}

std::vector<Obj> VectorReturnFunc2() {
	PrintStep(__FUNCTION__);
	std::vector<Obj> v;
	v.resize(3);
	for (auto& o : v)
		o.p.reset(new Obj);
	PrintStep("return " << __FUNCTION__);
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
		Func func3 = [mv = std::move(o)]() // C++14
		{
			_cout << "call func3 " << mv.i << "\n";
		};

		PrintStep(4);
		std::vector<Func> v;
		v.push_back(func1);

		PrintStep(5);
		v.push_back(func2);

		PrintStep(6);
		v.push_back(std::move(func3));

		PrintStep(7);
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


	TestStart("Test12")
		Obj o1(111, "o1");
		Obj o2(222, "o2");
		o1 = std::move(o2);
	TestEnd
	return 0;
}

/*******************************************************************************************************************************
                                                        Visual Studio 2015
                     USE_MOVE_OPERATION (O)                                              USE_MOVE_OPERATION (X)
[Test01] Start                                                  	[Test01] Start
   Pramater Constructor  000000000023F820 (10,Param)            	   Pramater Constructor  00000000002DF910 (10,Param)
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F820 (10,Param)            	   Destructor            00000000002DF910 (10,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test02] Start                                                  	[Test02] Start
   Pramater Constructor  000000000023F4C0 (11,Param)            	   Pramater Constructor  00000000002DF5B0 (11,Param)
   Copy Constructor      000000000023F398   000000000023F4C0    	   Copy Constructor      00000000002DF4F8   00000000002DF5B0
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F398 (11,Param)            	   Destructor            00000000002DF4F8 (11,Param)
   Destructor            000000000023F4C0 (11,Param)            	   Destructor            00000000002DF5B0 (11,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test03] Start                                                  	[Test03] Start
   Basic Constructor     000000000023F360                       	   Basic Constructor     00000000002DF488
   Basic Constructor     000000000023F2F0                       	   Basic Constructor     00000000002DF4C0
   Substitute Operator   000000000023F2F0   000000000023F360    	   Substitute Operator   00000000002DF4C0   00000000002DF488
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F2F0 (0,)                  	   Destructor            00000000002DF4C0 (4745856,)
   Destructor            000000000023F360 (0,)                  	   Destructor            00000000002DF488 (4745856,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test04] Start                                                  	[Test04] Start
   Pramater Constructor  000000000023F858 (12,Param)            	   Pramater Constructor  00000000002DF948 (12,Param)
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F858 (12,Param)            	   Destructor            00000000002DF948 (12,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test05] Start                                                  	[Test05] Start
   Pramater Constructor  000000000023F288 (12,Param)            	   Pramater Constructor  00000000002DF378 (12,Param)
   Member Function       000000000023F288                       	   Member Function       00000000002DF378
   Copy Constructor      000000000023F890   000000000023F288    	   Copy Constructor      00000000002DF980   00000000002DF378
   Destructor            000000000023F288 (12,Param)            	   Destructor            00000000002DF378 (12,Param)
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F890 (12,Param)            	   Destructor            00000000002DF980 (12,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test06] Start                                                  	[Test06] Start
   Basic Constructor     000000000023F410                       	   Basic Constructor     00000000002DF418
   Basic Constructor     000000000023F448                       	   Basic Constructor     00000000002DF450
   Move Operator         000000000023F410   000000000023F448    	   Substitute Operator   00000000002DF418   00000000002DF450
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F448 (2669696,)            	   Destructor            00000000002DF450 (0,)
   Destructor            000000000023F410 (0,)                  	   Destructor            00000000002DF418 (0,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test07] Start                                                  	[Test07] Start
   Basic Constructor     000000000023F328                       	   Basic Constructor     00000000002DF3E0
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0000000000288E20                       	   Basic Constructor     00000000004C8E20
   (Step 2)                                                     	   (Step 2)
   (Step VectorReturnFunc1)                                     	   (Step VectorReturnFunc1)
   Basic Constructor     000000000023F250                       	   Basic Constructor     00000000002DF340
   Basic Constructor     0000000000288E60                       	   Basic Constructor     00000000004C8E60
   (Step return VectorReturnFunc1)                              	   (Step return VectorReturnFunc1)
   Move Operator         000000000023F328   000000000023F250    	   Destructor            00000000004C8E20 (234,)
   Destructor            0000000000288E20 (234,)                	   Substitute Operator   00000000002DF3E0   00000000002DF340
   Destructor            000000000023F250 (128,)                	   Destructor            00000000002DF340 (0,)
   (exit scope)                                                 	   (exit scope)
   Destructor            0000000000288E60 (238,)                	   Destructor            00000000004C8E60 (238,)
   Destructor            000000000023F328 (0,)                  	   Destructor            00000000002DF3E0 (0,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test08] Start                                                  	[Test08] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0000000000288E60                       	   Basic Constructor     00000000004C8E60
   (Step 2)                                                     	   (Step 2)
   Basic Constructor     0000000000288E20                       	   Basic Constructor     00000000004C8E20
   (Step 3)                                                     	   (Step 3)
   (Step VectorReturnFunc2)                                     	   (Step VectorReturnFunc2)
   Basic Constructor     000000000028E780                       	   Basic Constructor     00000000004CE780
   Basic Constructor     000000000028E7B8                       	   Basic Constructor     00000000004CE7B8
   Basic Constructor     000000000028E7F0                       	   Basic Constructor     00000000004CE7F0
   Basic Constructor     0000000000288EA0                       	   Basic Constructor     00000000004C8EA0
   Basic Constructor     0000000000288EE0                       	   Basic Constructor     00000000004C8EE0
   Basic Constructor     0000000000288F20                       	   Basic Constructor     00000000004C8F20
   (Step return VectorReturnFunc2)                              	   (Step return VectorReturnFunc2)
   Destructor            0000000000288E20 (238,)                	   Destructor            00000000004C8E20 (238,)
   Destructor            0000000000288E60 (230,)                	   Destructor            00000000004C8E60 (230,)
   (exit scope)                                                 	   (exit scope)
   Destructor            0000000000288EA0 (242,)                	   Destructor            00000000004C8EA0 (242,)
   Destructor            000000000028E780 (2359640,)            	   Destructor            00000000004CE780 (4718936,)
   Destructor            0000000000288EE0 (246,)                	   Destructor            00000000004C8EE0 (246,)
   Destructor            000000000028E7B8 (0,)                  	   Destructor            00000000004CE7B8 (0,)
   Destructor            0000000000288F20 (250,)                	   Destructor            00000000004C8F20 (250,)
   Destructor            000000000028E7F0 (0,)                  	   Destructor            00000000004CE7F0 (0,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test09] Start                                                  	[Test09] Start
   Pramater Constructor  000000000023F4F8 (123,)                	   Pramater Constructor  00000000002DF658 (123,)
   (Step 1)                                                     	   (Step 1)
   Copy Constructor      000000000023F5A8   000000000023F4F8    	   Copy Constructor      00000000002DF708   00000000002DF658
   (Step 2)                                                     	   (Step 2)
   Move Constructor      000000000023F570   000000000023F4F8    	   Copy Constructor      00000000002DF6D0   00000000002DF658
   (Step 3)                                                     	   (Step 3)
   Move Constructor      000000000023F140   000000000023F4F8    	   Copy Constructor      00000000002DF230   00000000002DF658
   Move Constructor      0000000000289BA8   000000000023F140    	   Copy Constructor      00000000004C9BA8   00000000002DF230
   Destructor            000000000023F140 (6488169,)            	   Destructor            00000000002DF230 (123,)
   (Step 4)                                                     	   (Step 4)
   Copy Constructor      000000000023F178   000000000023F5A8    	   Copy Constructor      00000000002DF1F8   00000000002DF708
   Move Constructor      0000000000289BF8   000000000023F178    	   Copy Constructor      00000000004C9BF8   00000000002DF1F8
   Destructor            000000000023F178 (7274610,)            	   Destructor            00000000002DF1F8 (123,)
   (Step 5)                                                     	   (Step 5)
   Copy Constructor      000000000023F108   000000000023F570    	   Copy Constructor      00000000002DF1C0   00000000002DF6D0
   Move Constructor      0000000000289C98   000000000023F108    	   Copy Constructor      00000000004C9C98   00000000002DF1C0
   Destructor            000000000023F108 (7536741,)            	   Destructor            00000000002DF1C0 (123,)
   (Step 6)                                                     	   (Step 6)
   (Step 7)                                                     	   (Step 7)
   call func1 123                                               	   call func1 123
   call func2 123                                               	   call func2 123
   (exit scope)                                                 	   (exit scope)
   Destructor            0000000000289BF8 (123,)                	   Destructor            00000000004C9BF8 (123,)
   Destructor            0000000000289C98 (123,)                	   Destructor            00000000004C9C98 (123,)
   Destructor            0000000000289BA8 (0,)                  	   Destructor            00000000004C9BA8 (123,)
   Destructor            000000000023F570 (123,)                	   Destructor            00000000002DF6D0 (123,)
   Destructor            000000000023F5A8 (123,)                	   Destructor            00000000002DF708 (123,)
   Destructor            000000000023F4F8 (-392674,)            	   Destructor            00000000002DF658 (123,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test10] Start                                                  	[Test10] Start
   Pramater Constructor  000000000023F6C8 (123,)                	   Pramater Constructor  00000000002DF828 (123,)
   (Step 1)                                                     	   (Step 1)
   Move Constructor      000000000023F690   000000000023F6C8    	   Copy Constructor      00000000002DF7F0   00000000002DF828
   (Step 2)                                                     	   (Step 2)
   Move Constructor      000000000023F0D0   000000000023F690    	   Copy Constructor      00000000002DF268   00000000002DF7F0
   Move Constructor      0000000000289BA8   000000000023F0D0    	   Copy Constructor      00000000004C9BA8   00000000002DF268
   Destructor            000000000023F0D0 (0,)                  	   Destructor            00000000002DF268 (123,)
   (Step 3)                                                     	   (Step 3)
   (Step 4)                                                     	   (Step 4)
   Copy Constructor      0000000000289C98   0000000000289BA8    	   Copy Constructor      00000000004C9C98   00000000004C9BA8
   call func 123                                                	   call func 123
   (exit scope)                                                 	   (exit scope)
   Destructor            0000000000289C98 (123,)                	   Destructor            00000000004C9C98 (123,)
   Destructor            0000000000289BA8 (123,)                	   Destructor            00000000004C9BA8 (123,)
   Destructor            000000000023F690 (0,)                  	   Destructor            00000000002DF7F0 (123,)
   Destructor            000000000023F6C8 (6029413,)            	   Destructor            00000000002DF828 (123,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test11] Start                                                  	[Test11] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     000000000023F5E0                       	   Basic Constructor     00000000002DF740
   Basic Constructor     000000000023F618                       	   Basic Constructor     00000000002DF778
   Basic Constructor     000000000023F650                       	   Basic Constructor     00000000002DF7B0
   (Step 2)                                                     	   (Step 2)
   Move Constructor      000000000023F770   000000000023F5E0    	   Copy Constructor      00000000002DF860   00000000002DF740
   Move Constructor      000000000023F7A8   000000000023F618    	   Copy Constructor      00000000002DF898   00000000002DF778
   Move Constructor      000000000023F7E0   000000000023F650    	   Copy Constructor      00000000002DF8D0   00000000002DF7B0
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F7E0 (0,)                  	   Destructor            00000000002DF8D0 (7536759,)
   Destructor            000000000023F7A8 (0,)                  	   Destructor            00000000002DF898 (7340147,)
   Destructor            000000000023F770 (4096,)               	   Destructor            00000000002DF860 (0,)
   Destructor            000000000023F650 (-792248928,)         	   Destructor            00000000002DF7B0 (7536759,)
   Destructor            000000000023F618 (2379600,)            	   Destructor            00000000002DF778 (7340147,)
   Destructor            000000000023F5E0 (-792248928,)         	   Destructor            00000000002DF740 (0,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test12] Start                                                  	[Test12] Start
   Pramater Constructor  000000000023F738 (111,o1)              	   Pramater Constructor  00000000002DF5E8 (111,o1)
   Pramater Constructor  000000000023F700 (222,o2)              	   Pramater Constructor  00000000002DF620 (222,o2)
   Move Operator         000000000023F738   000000000023F700    	   Substitute Operator   00000000002DF5E8   00000000002DF620
   (exit scope)                                                 	   (exit scope)
   Destructor            000000000023F700 (111,o1)              	   Destructor            00000000002DF620 (222,o2)
   Destructor            000000000023F738 (222,o2)              	   Destructor            00000000002DF5E8 (222,o2)
------------------------------------------------------------    	------------------------------------------------------------
*/




/*******************************************************************************************************************************
                                                              GCC 5.2
                     USE_MOVE_OPERATION (O)                                              USE_MOVE_OPERATION (X)
[Test01] Start                                                  	[Test01] Start
   Pramater Constructor  0x7fffe8a35660 (10,Param)              	   Pramater Constructor  0x7fffd78d5a80 (10,Param)
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a35660 (10,Param)              	   Destructor            0x7fffd78d5a80 (10,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test02] Start                                                  	[Test02] Start
   Pramater Constructor  0x7fffe8a355b0 (11,Param)              	   Pramater Constructor  0x7fffd78d59d0 (11,Param)
   Copy Constructor      0x7fffe8a35660   0x7fffe8a355b0        	   Copy Constructor      0x7fffd78d5a80   0x7fffd78d59d0
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a35660 (11,Param)              	   Destructor            0x7fffd78d5a80 (11,Param)
   Destructor            0x7fffe8a355b0 (11,Param)              	   Destructor            0x7fffd78d59d0 (11,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test03] Start                                                  	[Test03] Start
   Basic Constructor     0x7fffe8a355b0                         	   Basic Constructor     0x7fffd78d59d0
   Basic Constructor     0x7fffe8a35660                         	   Basic Constructor     0x7fffd78d5a80
   Substitute Operator   0x7fffe8a35660   0x7fffe8a355b0        	   Substitute Operator   0x7fffd78d5a80   0x7fffd78d59d0
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a35660 (11,)                   	   Destructor            0x7fffd78d5a80 (11,)
   Destructor            0x7fffe8a355b0 (11,)                   	   Destructor            0x7fffd78d59d0 (11,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test04] Start                                                  	[Test04] Start
   Pramater Constructor  0x7fffe8a35660 (12,Param)              	   Pramater Constructor  0x7fffd78d5a80 (12,Param)
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a35660 (12,Param)              	   Destructor            0x7fffd78d5a80 (12,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test05] Start                                                  	[Test05] Start
   Pramater Constructor  0x7fffe8a35660 (12,Param)              	   Pramater Constructor  0x7fffd78d5a80 (12,Param)
   Member Function       0x7fffe8a35660                         	   Member Function       0x7fffd78d5a80
   Copy Constructor      0x7fffe8a355b0   0x7fffe8a35660        	   Copy Constructor      0x7fffd78d59d0   0x7fffd78d5a80
   Destructor            0x7fffe8a35660 (12,Param)              	   Destructor            0x7fffd78d5a80 (12,Param)
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a355b0 (12,Param)              	   Destructor            0x7fffd78d59d0 (12,Param)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test06] Start                                                  	[Test06] Start
   Basic Constructor     0x7fffe8a355b0                         	   Basic Constructor     0x7fffd78d59d0
   Basic Constructor     0x7fffe8a35660                         	   Basic Constructor     0x7fffd78d5a80
   Move Operator         0x7fffe8a355b0   0x7fffe8a35660        	   Substitute Operator   0x7fffd78d59d0   0x7fffd78d5a80
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a35660 (12,)                   	   Destructor            0x7fffd78d5a80 (12,)
   Destructor            0x7fffe8a355b0 (12,)                   	   Destructor            0x7fffd78d59d0 (12,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test07] Start                                                  	[Test07] Start
   Basic Constructor     0x7fffe8a355b0                         	   Basic Constructor     0x7fffd78d59d0
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0xa77c20                               	   Basic Constructor     0x9f1c20
   (Step 2)                                                     	   (Step 2)
   (Step VectorReturnFunc1)                                     	   (Step VectorReturnFunc1)
   Basic Constructor     0x7fffe8a35660                         	   Basic Constructor     0x7fffd78d5a80
   Basic Constructor     0xa77c80                               	   Basic Constructor     0x9f1c80
   (Step return VectorReturnFunc1)                              	   (Step return VectorReturnFunc1)
   Move Operator         0x7fffe8a355b0   0x7fffe8a35660        	   Destructor            0x9f1c20 (0,)
   Destructor            0xa77c20 (0,)                          	   Substitute Operator   0x7fffd78d59d0   0x7fffd78d5a80
   Destructor            0x7fffe8a35660 (12,)                   	   Destructor            0x7fffd78d5a80 (12,)
   (exit scope)                                                 	   (exit scope)
   Destructor            0xa77c80 (0,)                          	   Destructor            0x9f1c80 (0,)
   Destructor            0x7fffe8a355b0 (12,)                   	   Destructor            0x7fffd78d59d0 (12,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test08] Start                                                  	[Test08] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0xa77c80                               	   Basic Constructor     0x9f1c80
   (Step 2)                                                     	   (Step 2)
   Basic Constructor     0xa77c20                               	   Basic Constructor     0x9f1c20
   (Step 3)                                                     	   (Step 3)
   (Step VectorReturnFunc2)                                     	   (Step VectorReturnFunc2)
   Basic Constructor     0xa77ce0                               	   Basic Constructor     0x9f1ce0
   Basic Constructor     0xa77d18                               	   Basic Constructor     0x9f1d18
   Basic Constructor     0xa77d50                               	   Basic Constructor     0x9f1d50
   Basic Constructor     0xa77d90                               	   Basic Constructor     0x9f1d90
   Basic Constructor     0xa77dd0                               	   Basic Constructor     0x9f1dd0
   Basic Constructor     0xa77e30                               	   Basic Constructor     0x9f1e30
   (Step return VectorReturnFunc2)                              	   (Step return VectorReturnFunc2)
   Destructor            0xa77c20 (0,)                          	   Destructor            0x9f1c20 (0,)
   Destructor            0xa77c80 (10976272,)                   	   Destructor            0x9f1c80 (10427408,)
   (exit scope)                                                 	   (exit scope)
   Destructor            0xa77d90 (0,)                          	   Destructor            0x9f1d90 (0,)
   Destructor            0xa77ce0 (0,)                          	   Destructor            0x9f1ce0 (0,)
   Destructor            0xa77dd0 (0,)                          	   Destructor            0x9f1dd0 (0,)
   Destructor            0xa77d18 (0,)                          	   Destructor            0x9f1d18 (0,)
   Destructor            0xa77e30 (0,)                          	   Destructor            0x9f1e30 (0,)
   Destructor            0xa77d50 (0,)                          	   Destructor            0x9f1d50 (0,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test09] Start                                                  	[Test09] Start
   Pramater Constructor  0x7fffe8a354b0 (123,)                  	   Pramater Constructor  0x7fffd78d58d0 (123,)
   (Step 1)                                                     	   (Step 1)
   Copy Constructor      0x7fffe8a354f0   0x7fffe8a354b0        	   Copy Constructor      0x7fffd78d5910   0x7fffd78d58d0
   (Step 2)                                                     	   (Step 2)
   Move Constructor      0x7fffe8a35530   0x7fffe8a354b0        	   Copy Constructor      0x7fffd78d5950   0x7fffd78d58d0
   (Step 3)                                                     	   (Step 3)
   Move Constructor      0x7fffe8a35570   0x7fffe8a354b0        	   Copy Constructor      0x7fffd78d5990   0x7fffd78d58d0
   Move Constructor      0xa77e30   0x7fffe8a35570              	   Copy Constructor      0x9f1e30   0x7fffd78d5990
   Destructor            0x7fffe8a35570 (10976704,)             	   Destructor            0x7fffd78d5990 (123,)
   (Step 4)                                                     	   (Step 4)
   Copy Constructor      0x7fffe8a355b0   0x7fffe8a354f0        	   Copy Constructor      0x7fffd78d59d0   0x7fffd78d5910
   Move Constructor      0xa77dd0   0x7fffe8a355b0              	   Copy Constructor      0x9f1dd0   0x7fffd78d59d0
   Destructor            0x7fffe8a355b0 (10976640,)             	   Destructor            0x7fffd78d59d0 (123,)
   (Step 5)                                                     	   (Step 5)
   Copy Constructor      0x7fffe8a35660   0x7fffe8a35530        	   Copy Constructor      0x7fffd78d5a80   0x7fffd78d5950
   Move Constructor      0xa77d90   0x7fffe8a35660              	   Copy Constructor      0x9f1d90   0x7fffd78d5a80
   Copy Constructor      0xa77c80   0xa77dd0                    	   Copy Constructor      0x9f1c80   0x9f1dd0
   Destructor            0xa77dd0 (123,)                        	   Destructor            0x9f1dd0 (123,)
   Destructor            0x7fffe8a35660 (10976368,)             	   Destructor            0x7fffd78d5a80 (123,)
   (Step 6)                                                     	   (Step 6)
   Copy Constructor      0xa77dd0   0xa77c80                    	   Copy Constructor      0x9f1dd0   0x9f1c80
   Copy Constructor      0xa77c20   0xa77d90                    	   Copy Constructor      0x9f1c20   0x9f1d90
   Destructor            0xa77c80 (123,)                        	   Destructor            0x9f1c80 (123,)
   Destructor            0xa77d90 (123,)                        	   Destructor            0x9f1d90 (123,)
   (Step 7)                                                     	   (Step 7)
   call func1 123                                               	   call func1 123
   call func2 123                                               	   call func2 123
   (exit scope)                                                 	   (exit scope)
   Destructor            0xa77dd0 (123,)                        	   Destructor            0x9f1dd0 (123,)
   Destructor            0xa77c20 (123,)                        	   Destructor            0x9f1c20 (123,)
   Destructor            0xa77e30 (1,)                          	   Destructor            0x9f1e30 (123,)
   Destructor            0x7fffe8a35530 (123,)                  	   Destructor            0x7fffd78d5950 (123,)
   Destructor            0x7fffe8a354f0 (123,)                  	   Destructor            0x7fffd78d5910 (123,)
   Destructor            0x7fffe8a354b0 (10976480,)             	   Destructor            0x7fffd78d58d0 (123,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test10] Start                                                  	[Test10] Start
   Pramater Constructor  0x7fffe8a35570 (123,)                  	   Pramater Constructor  0x7fffd78d5990 (123,)
   (Step 1)                                                     	   (Step 1)
   Move Constructor      0x7fffe8a355b0   0x7fffe8a35570        	   Copy Constructor      0x7fffd78d59d0   0x7fffd78d5990
   (Step 2)                                                     	   (Step 2)
   Move Constructor      0x7fffe8a35660   0x7fffe8a355b0        	   Copy Constructor      0x7fffd78d5a80   0x7fffd78d59d0
   Move Constructor      0xa77c20   0x7fffe8a35660              	   Copy Constructor      0x9f1c20   0x7fffd78d5a80
   Destructor            0x7fffe8a35660 (-1464170632,)          	   Destructor            0x7fffd78d5a80 (123,)
   (Step 3)                                                     	   (Step 3)
   (Step 4)                                                     	   (Step 4)
   Copy Constructor      0xa77c60   0xa77c20                    	   Copy Constructor      0x9f1c60   0x9f1c20
   call func 123                                                	   call func 123
   (exit scope)                                                 	   (exit scope)
   Destructor            0xa77c60 (123,)                        	   Destructor            0x9f1c60 (123,)
   Destructor            0xa77c20 (123,)                        	   Destructor            0x9f1c20 (123,)
   Destructor            0x7fffe8a355b0 (-391948688,)           	   Destructor            0x7fffd78d59d0 (123,)
   Destructor            0x7fffe8a35570 (10976640,)             	   Destructor            0x7fffd78d5990 (123,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test11] Start                                                  	[Test11] Start
   (Step 1)                                                     	   (Step 1)
   Basic Constructor     0x7fffe8a355b0                         	   Basic Constructor     0x7fffd78d59d0
   Basic Constructor     0x7fffe8a355e8                         	   Basic Constructor     0x7fffd78d5a08
   Basic Constructor     0x7fffe8a35620                         	   Basic Constructor     0x7fffd78d5a40
   (Step 2)                                                     	   (Step 2)
   Move Constructor      0x7fffe8a35660   0x7fffe8a355b0        	   Copy Constructor      0x7fffd78d5a80   0x7fffd78d59d0
   Move Constructor      0x7fffe8a35698   0x7fffe8a355e8        	   Copy Constructor      0x7fffd78d5ab8   0x7fffd78d5a08
   Move Constructor      0x7fffe8a356d0   0x7fffe8a35620        	   Copy Constructor      0x7fffd78d5af0   0x7fffd78d5a40
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a356d0 (64971124,)             	   Destructor            0x7fffd78d5af0 (64971124,)
   Destructor            0x7fffe8a35698 (-1452980896,)          	   Destructor            0x7fffd78d5ab8 (1185432928,)
   Destructor            0x7fffe8a35660 (-391948688,)           	   Destructor            0x7fffd78d5a80 (123,)
   Destructor            0x7fffe8a35620 (6327040,)              	   Destructor            0x7fffd78d5a40 (64971124,)
   Destructor            0x7fffe8a355e8 (1,)                    	   Destructor            0x7fffd78d5a08 (1185432928,)
   Destructor            0x7fffe8a355b0 (-1464170632,)          	   Destructor            0x7fffd78d59d0 (123,)
------------------------------------------------------------    	------------------------------------------------------------
                                                                	
[Test12] Start                                                  	[Test12] Start
   Pramater Constructor  0x7fffe8a355b0 (111,o1)                	   Pramater Constructor  0x7fffd78d59d0 (111,o1)
   Pramater Constructor  0x7fffe8a35660 (222,o2)                	   Pramater Constructor  0x7fffd78d5a80 (222,o2)
   Move Operator         0x7fffe8a355b0   0x7fffe8a35660        	   Substitute Operator   0x7fffd78d59d0   0x7fffd78d5a80
   (exit scope)                                                 	   (exit scope)
   Destructor            0x7fffe8a35660 (111,o1)                	   Destructor            0x7fffd78d5a80 (222,o2)
   Destructor            0x7fffe8a355b0 (222,o2)                	   Destructor            0x7fffd78d59d0 (222,o2)
------------------------------------------------------------    	------------------------------------------------------------
*/