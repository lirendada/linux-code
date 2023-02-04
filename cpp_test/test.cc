#include <iostream>
using namespace std;

class parent
{
public:
    static int a;
};
int parent::a = 10;
class Student: public parent
{
public:
    void print()
    {
        a = 20;
        cout << a << endl;
        cout << parent::a << endl;
    }
};
int main()
{
    Student s;
    s.print();
    return 0;
}