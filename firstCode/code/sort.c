#include<stdio.h>

void InsertSort(int arr[], int n)
{
    int i = 0;
    for(i = 1; i < n; i++)
    {
      	int end = i - 1;
  	    int tmp = arr[i];
  	    while(end >= 0)
  	    {
  	        if(tmp < arr[end])
  	        {
  	            arr[end + 1] = arr[end];
  	            end--;
  	        }
  	        else
  		          break;
  	    }
  	    arr[end + 1] = tmp;
    }
}

int main()
{
    int arr[] = {4, 6, 3, 2, 1, 5, 8, 7, 9};
    int n = sizeof(arr) / sizeof(arr[0]);
    
    InsertSort(arr, n);
    for(int i = 0; i < n; ++i)
      printf("%d ",arr[i]);
    return 0;
}
