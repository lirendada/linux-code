#include <iostream>
#include <string>
#include <vector>
using namespace std;
/*
        字符串匹配算法
        1、BF算法(效率低）×
        2、KMP算法 ✔
    str: 代表主串
    sub：代表子串
    next：代表的就是next数组
    KMP函数要求：返回匹配位置的下标，若不匹配则返回-1
*/

void getNext(const string& sub, vector<int>& next, int n)
{
    // 先将两个固定的值赋值
    next[0] = -1;
    if(n >= 2)
        next[1] = 0;    // 判断一下防止越界，但是next[0]就不用了，因为在KMP中我们已经判断字符串起码有一个字符了
    
    // 这里两个下标是与解析对应起来的
    int i = 1;
    int k = 0;
    while(i < n - 1)    // 这里遍历到n-1是因为下面填next数组的时候是next[i + 1]
    {
        if(k == -1 || sub[i] == sub[k])
        {
            // 如果k==-1（表示出界）或者当前两个字符相等的话，则进行赋值
            next[i + 1] = k + 1;
            ++i;
            ++k;
        }
        else
        {
            // 否则跳到next数组当前位置所指的前个位置
            k = next[k];
        }
    }
}

int KMP(const string& str, const string& sub)
{
    // 特殊情况处理
    if(str.empty() || sub.empty())
        return -1;
    
    int str_size = str.size();
    int sub_size = sub.size();
    
    // 创建next数组，初始化为0
    vector<int> next(sub_size, 0);
    getNext(sub, next, sub_size);

    int i = 0; // 遍历主串
    int j = 0; // 遍历子串
    while(i < str_size && j < sub_size)
    {
        if(j == -1 || str[i] == sub[j])
        {
            // 如果j==-1（表示出界）或者当前两个字符相等的话，则向后走
            i++;
            j++;
        }
        else
        {
            // 否则跳到next数组当前位置所指的前个位置
            j = next[j];
        }
    }

    if(j < sub_size)
        return -1;
    return i - j;
}

int main()
{
    string str = "abcababcabc";
    cout << KMP(str, string("abcabc")) << endl;
    cout << KMP(str, string("abc")) << endl;
    cout << KMP(str, string("abcd")) << endl;
    cout << KMP(str, string("--")) << endl;
    cout << KMP(str, string("")) << endl;
    return 0;
}