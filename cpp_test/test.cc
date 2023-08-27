// // // #include <iostream>
// // // using namespace std;

// // // class parent
// // // {
// // // public:
// // //     static int a;
// // // };
// // // int parent::a = 10;
// // // class Student: public parent
// // // {
// // // public:
// // //     void print()
// // //     {
// // //         a = 20;
// // //         cout << a << endl;
// // //         cout << parent::a << endl;
// // //     }
// // // };
// // // int main()
// // // {
// // //     Student s;
// // //     s.print();
// // //     return 0;
// // // }

// // // #include <iostream>
// // // #include <vector>
// // // #include <algorithm>
// // // using namespace std;
// // // int main()
// // // {
// // //   // 请在此输入您的代码
// // //   int n = 0;
// // //   cin >> n;
// // //   cout << n << endl;
// // //   vector<vector<int>> vv(n + 2, vector<int>(n + 2, -1));
// // //   for(int i = 1; i < n + 1; ++i)
// // //   {
// // //     for(int j = 1; j <= i; ++j)
// // //       cin >> vv[i][j];
// // //   }
// // // // for(int i = 0; i < n + 2; ++i)
// // // //   {
// // // //     for(int j = 0; j < n + 2; ++j)
// // // //       cout << vv[i][j] << " ";
// // // //     cout << endl;
// // // //   }
// // // //   for(int i = 2; i < n + 1; ++i)
// // // //   {
// // // //     if(i % 2 == 0)
// // // //     {
// // // //       int a1 = i / 2;
// // // //       int a2 = i / 2 + 1;
// // // //       int up = vv[i][a1];
// // // //       vv[i][a1] += up;
// // // //       vv[i][a2] += up;
// // // //     }
// // // //     else
// // // //     {
// // // //       int a = i / 2 + 1;
// // // //       vv[i][a] = max(vv[i - 1][a - 1], vv[i - 1][a]) + vv[i][a];
// // // //     }
// // // //   }
// // // //   for(int i = 0; i < n + 2; ++i)
// // // //   {
// // // //     for(int j = 0; j < n + 2; ++j)
// // // //       cout << vv[i][j] << " ";
// // // //     cout << endl;
// // // //   }
// // //   if(n % 2 == 0)
// // //     cout << max(vv[n][n / 2], vv[n][n/2 + 1]) << endl;
// // //   else
// // //     cout << vv[n][n / 2 + 1] << endl;
// // //   return 0;
// // // }


// // // #include <iostream>
// // // #include <vector>
// // // #include <map>
// // // using namespace std;

// // // int main()
// // // { 
// // //     // 内置类型变量
// // //     int x1 = {10};
// // //     int x2{10};
// // //     int x3 = 1+2;
// // //     int x4 = {1+2};
// // //     int x5{1+2};
// // //     // 数组
// // //     int arr1[5] = {1,2,3,4,5};
// // //     int arr2[]{1,2,3,4,5};

// // //     // 动态数组，在C++98中不支持
// // //     int* arr3 = new int[5]{1,2,3,4,5};

// // //     // 标准容器
// // //     vector<int> v1{1,2,3,4,5};
// // //     vector<int> v2 = {1,2,3,4,5};
    
// // //     map<int, int> m1{{1,1}, {2,2},{3,3},{4,4}};
// // //     map<int, int> m2 = {{1,1}, {2,2},{3,3},{4,4}};
    
// // //     return 0;
// // // }

// // #include <iostream>
// // #include <memory>
// // using namespace std;


// // template<class T>
// // struct DeleteArray
// // {
// // 	void operator()(const T* ptr)
// // 	{
// // 		delete[] ptr;
// // 		cout << "delete[] " << ptr << endl;
// // 	}
// // };

// // int main()
// // {
// // 	//shared_ptr<int> sp1(new int[10]); // 不一定会报错，因为是内置类型
// // 	//shared_ptr<string> sp2(new string[10]); // 肯定会报错，因为是自定义类型

// // 	// 注意下述传递的第二个参数是函数对象而不是类型，所以需要加()
// // 	shared_ptr<int> sp1(new int[10], DeleteArray<int>());
// // 	shared_ptr<string> sp2(new string[10], DeleteArray<string>());

// // 	// 还可用使用lambda表达式
// // 	shared_ptr<string> sp3(new string[10], [](string* ptr) 
// // 		{
// // 			delete[] ptr;
// // 			cout << "lambda delete[] " << ptr << endl;
// // 		});

// // 	// 还可以是文件类型
// // 	shared_ptr<FILE> sp4(fopen("test.txt", "w"), [](FILE* ptr)
// // 		{
// // 			fclose(ptr);
// // 			cout << "file delete[] " << ptr << endl;
// // 		}); 

// // 	// 使用库中的默认删除器
// // 	// shared_ptr<string> sp3(new string[10], default_delete<string>());
// // 	return 0;
// // }




// #include <iostream>
// #include <stack>
// #include <string>
// using namespace std;

// bool isValid(const string& s)
// {
// 	if(s.empty())
// 		return false;
// 	stack<char> st;
// 	for(int i = 0; i < s.size(); ++i)
// 	{
// 		if(s[i] == '(' || s[i] == '{' || s[i] == '[')
// 			st.push(s[i]);
// 		else
// 		{
// 			if(st.empty())
// 				return false;
// 			if(s[i] == ')' && st.top() != '(')
// 				return false;
// 			else if(s[i] == ']' && st.top() != '[')
// 				return false;
// 			else if(s[i] == '}' && st.top() != '{')
// 				return false;
			
// 			st.pop();
// 		}
// 	}
// 	if(st.empty())
// 		return true;
// 	return false;
// }

// int main()
// {
// 	string tmp = "()[]{}";
// 	cout << isValid(tmp) << endl;
// 	tmp = "()[";
// 	cout << isValid(tmp) << endl;
// 	tmp = "([)]";
// 	cout << isValid(tmp) << endl;
// 	return 0;
// }

#include <iostream>
using namespace std;
//多继承
class Base1 
{
public:
	virtual void func1() { cout << "Base1::func1" << endl; }
	virtual void func2() { cout << "Base1::func2" << endl; }
private:
	int b1;
};

class Base2 
{
public:
	virtual void func1() { cout << "Base2::func1" << endl; }
	virtual void func2() { cout << "Base2::func2" << endl; }
private:
	int b2;
};

class Derive : public Base1, public Base2 
{
public:
	virtual void func1() { cout << "Derive::func1" << endl; }
	virtual void func3() { cout << "Derive::func3" << endl; }
private:
	int d1;
};

typedef void(*VFunc)();  // 由于等会要传_vfptr也就是存函数指针的数组指针，类型是void*，所以我们把他们都统一重命名为VFTunc

void PrintVFT(VFunc* ptr)  // 这里ptr是个存函数指针的数组指针
{
	// 依次取虚表中的虚函数指针打印并调用。调用就可以看出存的是哪个函数
	printf("_vfptr:%p\n", ptr);

	for (int i = 0; i <= 2; ++i)
	{
		printf("_vfptr[%d]:%p --> ", i, ptr[i]);
		ptr[i]();
	}
	cout << endl;
}

int main()
{
    Base1 b1;
	Base2 b2;

	Derive d;
	PrintVFT((VFunc*)(*((int*)&d)));
	PrintVFT((VFunc*)(*((int*)((char*)&d + sizeof(Base1))))); // 括号比较多，看的时候注意看仔细
    
    return 0;
}