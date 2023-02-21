// #include <iostream>
// using namespace std;

// class parent
// {
// public:
//     static int a;
// };
// int parent::a = 10;
// class Student: public parent
// {
// public:
//     void print()
//     {
//         a = 20;
//         cout << a << endl;
//         cout << parent::a << endl;
//     }
// };
// int main()
// {
//     Student s;
//     s.print();
//     return 0;
// }

// #include <iostream>
// #include <vector>
// #include <algorithm>
// using namespace std;
// int main()
// {
//   // 请在此输入您的代码
//   int n = 0;
//   cin >> n;
//   cout << n << endl;
//   vector<vector<int>> vv(n + 2, vector<int>(n + 2, -1));
//   for(int i = 1; i < n + 1; ++i)
//   {
//     for(int j = 1; j <= i; ++j)
//       cin >> vv[i][j];
//   }
// // for(int i = 0; i < n + 2; ++i)
// //   {
// //     for(int j = 0; j < n + 2; ++j)
// //       cout << vv[i][j] << " ";
// //     cout << endl;
// //   }
// //   for(int i = 2; i < n + 1; ++i)
// //   {
// //     if(i % 2 == 0)
// //     {
// //       int a1 = i / 2;
// //       int a2 = i / 2 + 1;
// //       int up = vv[i][a1];
// //       vv[i][a1] += up;
// //       vv[i][a2] += up;
// //     }
// //     else
// //     {
// //       int a = i / 2 + 1;
// //       vv[i][a] = max(vv[i - 1][a - 1], vv[i - 1][a]) + vv[i][a];
// //     }
// //   }
// //   for(int i = 0; i < n + 2; ++i)
// //   {
// //     for(int j = 0; j < n + 2; ++j)
// //       cout << vv[i][j] << " ";
// //     cout << endl;
// //   }
//   if(n % 2 == 0)
//     cout << max(vv[n][n / 2], vv[n][n/2 + 1]) << endl;
//   else
//     cout << vv[n][n / 2 + 1] << endl;
//   return 0;
// }


#include <iostream>
#include <vector>
#include <map>
using namespace std;

int main()
{ 
    // 内置类型变量
    int x1 = {10};
    int x2{10};
    int x3 = 1+2;
    int x4 = {1+2};
    int x5{1+2};
    // 数组
    int arr1[5] = {1,2,3,4,5};
    int arr2[]{1,2,3,4,5};

    // 动态数组，在C++98中不支持
    int* arr3 = new int[5]{1,2,3,4,5};

    // 标准容器
    vector<int> v1{1,2,3,4,5};
    vector<int> v2 = {1,2,3,4,5};
    
    map<int, int> m1{{1,1}, {2,2},{3,3},{4,4}};
    map<int, int> m2 = {{1,1}, {2,2},{3,3},{4,4}};
    
    return 0;
}