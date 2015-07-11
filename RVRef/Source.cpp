#include <iostream>
using namespace std;

//#define USE_MOVE_CONSTRUCTOR

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

struct Obj
{
	int i;
	string s;

	~Obj() {
		cout << "   Destructor            " << this << endl;
	}
	Obj() {
		cout << "   Basic Constructor     " << this << endl;
	}
	Obj(int i, string a) {
		cout << "   Pramater Constructor  " << this << endl;
	}
	Obj(const Obj& o) {
		cout << "   Copy Constructor      " << this << endl;
	}
	Obj& operator=(const Obj& o) {
		cout << "   Equal Operator        " << this << endl;
		return *this;
	}
#ifdef USE_MOVE_CONSTRUCTOR
	Obj(Obj&& o) {
		cout << "   Move Constructor      " << this << endl;
	}
	Obj& operator=(const Obj&& o) {
		cout << "   Move Operator         " << this << endl;
		return *this;
	}
#endif
	Obj& SomeFunc() {
		cout << "   Member Function       " << this << endl;
		return *this;
	}
};

#define TestStart(TestName) \
	void TestName() { \
		cout << "\n[" << __FUNCTION__ << "] Start                          .\n"; \
		{ \

#define TestEnd \
		} \
		cout << "-----------------------------------------\n"; \
	} \




TestStart(Test01)
	Obj o(Obj(10, "Param"));
TestEnd

TestStart(Test02)
	Obj o1(11, "Param");
	Obj o2 = o1;
TestEnd

TestStart(Test03)
	Obj o1;
	Obj o2;
	o2 = o1;
TestEnd

TestStart(Test04)
	Obj o = Obj(12, "Param");
TestEnd

TestStart(Test05)
	Obj o = Obj(12, "Param").SomeFunc();
TestEnd

TestStart(Test06)
	Obj o1;
	Obj o2;
	o1 = std::move(o2);
TestEnd



int main() {
	Test01();
	Test02();
	Test03();
	Test04();
	Test05();
	Test06();
	return 0;
}
