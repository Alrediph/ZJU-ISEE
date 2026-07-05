#编程语言 #C语言 #大一 #备考
# 镇楼之宝：求水仙花数
[[一些计算方法的集合#1.算水仙花数]]
```
为什么从小到大一直不会？
对于位数不同的而言：
最好定义一个数组来储存n次方的问题，可以大幅度简化复杂性
```
### 12.26重新做题经验：
1.在一个循环结束后记得把sum归0（😀）
## 题干
![[Pasted image 20251118143850.png]]
```
#include <stdio.h>
#include <math.h>

int main() {
    int N;
    scanf("%d", &N);
    
    int start = pow(10, N - 1);
    int end = pow(10, N) - 1;
    
    // 预计算0-9的N次幂，避免重复计算
    int powers[10];
    for (int i = 0; i < 10; i++) {
        powers[i] = pow(i, N);
    }
    
    for (int num = start; num <= end; num++) {
        int sum = 0;
        int temp = num;
        
        while (temp > 0) {
            sum += powers[temp % 10];  // 使用预计算结果
            temp /= 10;
        }
        
        if (sum == num) {
            printf("%d\n", num);
        }
    }
    
    return 0;
}
```

# 求组合数
```
注意：数据越界（常用long long）
```
## 题干：
![[Pasted image 20251113221245.png]]
### 可实现代码:

```
#include <stdio.h>

double fact(int n);  // 函数声明

int main() {
    int m, n;
    scanf("%d %d", &m, &n);  // 输入m和n
    
    // 计算组合数 C(n, m) = n! / (m! * (n-m)!)
    double result = fact(n) / (fact(m) * fact(n - m));
    
    printf("result = %.0f\n", result);  // 输出整数结果
    
    return 0;
}

// 计算阶乘的函数
double fact(int n) {
    double factorial = 1.0;
    for (int i = 1; i <= n; i++) {
        factorial *= i;
    }
    return factorial;
}
```

# 统计学生平均成绩与及格人数
```
注意：
1.学生人数等于0的情况
2.注意换行符细节
3.下面的程序可以运行但绝对不是最好的
```
## 题干
![[Pasted image 20251114204807.png]]
### 可实现代码:
```
#include <stdio.h>

int main() {
    int n;
    scanf("%d",&n);
    if (n==0) {
        printf("average = 0.0\n");
        printf("count = 0\n");
        return 0;
    }
    int a[n];
    for(int i=0;i<n;i++) {
        scanf("%d",&a[i]);
    }
    int sum=0;
    for(int i=0;i<n;i++) {
        sum+=a[i];
    }
    double aver=1.0*sum/n;
    printf("average = %.1lf\n",aver);
    int count=0;
    for(int i=0;i<n;i++) {
        if(a[i]>=60) {
            count++;
        }
    }
    printf("count = %d\n",count);
    return 0;
}
```

# ==排序类题目==
#  1.比较大小
[[语句#四、冒泡排序]]
[[一些计算方法的集合#7.冒泡法]]

```
attention!
1.使用使用冒泡排序对三个数进行排序，不要用先调整最大值后改变其他值
2.for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2 - i; j++) {
            if (a[j] > a[j + 1]) {
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
```
## 题干

![[Pasted image 20251115090613.png]]
```
#include <stdio.h>

int main() {
    int a[3];

    // 读取三个整数
    for (int i = 0; i < 3; i++) {
        scanf("%d", &a[i]);
    }

    // 使用冒泡排序对三个数进行排序
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2 - i; j++) {
            if (a[j] > a[j + 1]) {
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }

    // 按格式输出结果
    printf("%d->%d->%d\n", a[0], a[1], a[2]);

    return 0;
}
```

# 2.冒泡法排序
[[语句#四、冒泡排序]]
[[一些计算方法的集合#7.冒泡法]]
```
1.直接用外层for循环控制循环次数
```
## 题干：
![[Pasted image 20251121102357.png]]
## 可运行代码
```
#include <stdio.h>

int main() {
    int n, k;
    scanf("%d %d", &n, &k);
    int a[100]; // 题目中N≤100，所以数组大小设为100足够
    // 读取N个待排序的整数
    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    
    int count = 0; // 记录当前完成的遍数
    for (int i = 0; i < k; i++) { // 只进行K遍扫描
        for (int j = 0; j < n - i - 1; j++) { // 每遍扫描的比较范围：前n-i-1个元素
            if (a[j] > a[j + 1]) { // 若前一个元素大于后一个，交换
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
        count++; // 每完成一遍扫描，计数加1
        if (count == k) { // 完成K遍后，跳出循环
            break;
        }
    }
    
    // 输出第K遍后的中间结果
    for (int i = 0; i < n; i++) {
        if (i > 0) { // 不是第一个元素时，先输出空格
            printf(" ");
        }
        printf("%d", a[i]);
    }
    printf("\n"); // 输出换行
    
    return 0;
}
```

# 3.字符串排序
## 题干
![[Pasted image 20251121104432.png]]
## 可运行代码
```
#include <stdio.h>

#define MAX_STR_LEN 80  // 每个字符串的最大长度（小于80）
#define NUM_STRS 5      // 字符串的数量

// 自定义字符串比较函数，返回值与strcmp类似
int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 < *s2) return -1;
        if (*s1 > *s2) return 1;
        s1++;
        s2++;
    }
    if (*s1 == *s2) return 0;
    return (*s1 == '\0') ? -1 : 1;
}

// 自定义字符串复制函数，返回值与strcpy类似
void my_strcpy(char *dest, const char *src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

int main() {
    char strs[NUM_STRS][MAX_STR_LEN];  // 存储5个字符串
    char temp[MAX_STR_LEN];            // 用于交换的临时字符串

    // 读取5个字符串
    for (int i = 0; i < NUM_STRS; i++) {
        scanf("%s", strs[i]);
    }

    // 冒泡排序（按字典序升序）
    for (int i = 0; i < NUM_STRS - 1; i++) {
        for (int j = 0; j < NUM_STRS - i - 1; j++) {
            // 比较两个字符串的字典序
            if (my_strcmp(strs[j], strs[j + 1]) > 0) {
                // 交换两个字符串
                my_strcpy(temp, strs[j]);
                my_strcpy(strs[j], strs[j + 1]);
                my_strcpy(strs[j + 1], temp);
            }
        }
    }

    // 输出结果
    printf("After sorted:\n");
    for (int i = 0; i < NUM_STRS; i++) {
        printf("%s\n", strs[i]);
    }

    return 0;
}
```

## 4.英文单词排序
![[Pasted image 20251217232318.png]]
```
答案似乎在用一种很指针的思维做这道题
```
### 可运行代码
```
#include <stdio.h>
#include <string.h>

#define MAX_WORDS 20
#define MAX_LEN 10

int main() {
    char words[MAX_WORDS][MAX_LEN + 1];  // 存储单词
    int count = 0;
    
    while (count < MAX_WORDS) {
        char buffer[MAX_LEN + 1];
        if (scanf("%s", buffer) != 1) {
            break;  // 读取失败
        }
        
        if (strcmp(buffer, "#") == 0) {
            break;  // 遇到结束标志
        }
        
        // 复制单词到数组中
        strcpy(words[count], buffer);
        count++;
    }
    
    for (int i = 1; i < count; i++) {
        char temp[MAX_LEN + 1];
        strcpy(temp, words[i]);
        int j = i - 1;
        
        // 将比当前单词长的单词后移
        while (j >= 0 && strlen(words[j]) > strlen(temp)) {
            strcpy(words[j + 1], words[j]);
            j--;
        }
        strcpy(words[j + 1], temp);
    }
    
    // 输出结果，每个单词后加一个空格
    for (int i = 0; i < count; i++) {
        printf("%s ", words[i]);
    }
    printf("\n");
    
    return 0;
}
```

# 求给定精度的简单交错序列部分和
```
注意这是一个典型的do while语句的应用，能有效解决最后一项相加后再停止
```
![[Pasted image 20251117201108.png]]

```
#include <stdio.h>
#include <math.h>

int main() {
    double eps;
    scanf("%lf", &eps);
    
    double sum = 0.0;
    int sign = 1;  // 符号：1表示正，-1表示负
    int denominator = 1;  // 分母，从1开始
    double term;  // 当前项的值
    
    do {
        term = sign * 1.0 / denominator;  // 计算当前项
        sum += term;                      // 累加到总和
        sign = -sign;                     // 符号取反
        denominator += 3;                 // 分母增加3
    } while (fabs(term) > eps);          // 直到最后一项的绝对值不大于eps
    
    printf("sum = %.6f\n", sum);
    
    return 0;
}
```

# 求e的近似值
```
第一种思路是引入阶乘，设置一个函数，但很明显会越界，导致n取最大值时输出inf
```
![[Pasted image 20251117212758.png]]
```
#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);  // 读取非负整数n
    
    double e = 1.0;  // 初始化为1（级数第一项1）
    double factorial = 1.0;  // 阶乘值，初始为0! = 1
    
    // 计算级数前n+1项：从1/0! 到 1/n!
    for (int i = 1; i <= n; i++) {
        factorial *= i;  // 计算i的阶乘 = i!
        e += 1.0 / factorial;  // 加上当前项 1/i!
    }
    
    // 输出结果，保留8位小数
    printf("%.8f\n", e);
    
    return 0;
}
```

# 求奇数和
```
我也没搞懂我的程序怎么错了（哭）
```
## 题干
![[Pasted image 20251117215207.png]]
## 可实现代码:
```
#include <stdio.h>

int main() {
    int sum = 0;    // 奇数和
    int num;        // 当前输入的数字
    
    // 循环读取输入，直到遇到0或负数
    while (1) {
        scanf("%d", &num);
        
        // 遇到0或负数，结束输入
        if (num <= 0) {
            break;
        }
        
        // 如果是奇数，累加到总和中
        if (num % 2 == 1) {
            sum += num;
        }
    }
    
    // 输出奇数和
    printf("%d\n", sum);
    
    return 0;
}
```

```
12.26又做了一次，这次对了！！
#include <stdio.h>  
int main(){  
    int a[1000];  
    int n;  
    int count=0;  
    for(int i=0;i<1000;i++) {  
        scanf("%d",&n);  
        if (n>0) {  
            a[i] = n;  
            count++;  
        }else {  
            break;  
        }  
    }  
    int sum=0;  
    for(int i=0;i<count;i++) {  
        if((a[i]%2)!=0) {  
            sum+=a[i];  
        }  
    }  
    printf("%d\n",sum);  
  
  
    return 0;  
}
```
# 求素数！！！
```
1.可以预置函数用于判断素数，且可以通过去掉所有2的倍数减少运算次数
```
## 题干
![[Pasted image 20251117222408.png]]
## 可实现代码:
```
#include <stdio.h>
#include <math.h>

int isPrime(int num) {
    if (num <= 1) return 0;           // 1不是素数
    if (num == 2) return 1;           // 2是素数
    if (num % 2 == 0) return 0;       // 偶数不是素数
    
    int limit = sqrt(num) + 1;         // 优化：检查到平方根即可
    for (int i = 3; i <= limit; i += 2) {  // 只检查奇数因子
        if (num % i == 0) return 0;
    }
    return 1;
}

int main() {
    int M, N;
    scanf("%d %d", &M, &N);
    
    int count = 0, sum = 0;
    
    // 遍历M到N之间的所有数
    for (int num = M; num <= N; num++) {
        if (isPrime(num)) {
            count++;    // 素数计数
            sum += num; // 素数累加
        }
    }
    
    printf("%d %d\n", count, sum);
    return 0;
}
```

# 兔子繁殖
```
注意：
（1）考虑兔子数为1的情况直接返回1
（2）将三个月的兔子同时考虑，类似于数列递推
（3）兔子很可爱，不要讨厌它
```
### 这道题我在2025年12月底又做了一次，其实没有那么复杂，难点在于搞清楚第三个月的兔子数量如何通过前两个月相加得到就好了！
## 题干：
![[Pasted image 20251119125208.png]]
## 可实现代码
```
#include <stdio.h>

int main() {
    int N;
    scanf("%d", &N);
    
    if (N <= 1) {
        printf("1\n");
        return 0;
    }
    
    int month = 1;
    int prev1 = 1;  // 上个月兔子对数
    int prev2 = 0;  // 上上个月兔子对数
    int current = 1; // 当前兔子对数
    
    while (current < N) {
        month++;
        // 本月兔子数 = 上月兔子数 + 新出生兔子（上上个月兔子数）
        current = prev1 + prev2;
        // 更新前两个月的值
        prev2 = prev1;
        prev1 = current;
    }
    
    printf("%d\n", month);
    return 0;
}
```

# 英文字母替换加密
```
attention:
1.在while语句后不要再有c=getchar(),这样会跳过一个字符读取（但如果想要只读两个中的一个或许可以考虑）
2.简化一点就把c=‘z'||c='Z'的情况放进小循环中，anyway，能跑就行
```
## 题干
![[Pasted image 20251119183724.png]]
## 可运行代码
```
#include <stdio.h>  
int main() {  
    char c;  
    while ((c = getchar()) != '\n') {  
  
        if (c<'z'&&c>='a') {  
            c=c-'a'+'A'+1;  
            putchar(c);  
        }else if (c=='z') {  
            c='A';  
            putchar(c);  
        }else if (c>='A'&&c<'Z') {  
            c=c-'A'+'a'+1;  
            putchar(c);  
        }else if (c=='Z') {  
            c='a';  
            putchar(c);  
        }else {  
            putchar(c);  
        }  
    }  
    return 0;  
}
```

# 遍历类：
# 1.找出不是两个数组共有的元素
```
1.要遍历两个数组
2.要检查数组中有没有出现
```
## 题干：
![[Pasted image 20251119202910.png]]
## 可运行代码：
```
#include <stdio.h>

int main() {
    int n1, n2;
    int a[20], b[20];
    
    // 读入第一个数组
    scanf("%d", &n1);
    for (int i = 0; i < n1; i++) {
        scanf("%d", &a[i]);
    }
    
    // 读入第二个数组
    scanf("%d", &n2);
    for (int i = 0; i < n2; i++) {
        scanf("%d", &b[i]);
    }
    
    int result[40]; // 存放结果
    int cnt = 0;    // 结果个数
    
    // 检查 a 中的元素
    for (int i = 0; i < n1; i++) {
        int in_b = 0;
        // 检查 a[i] 是否在 b 中出现
        for (int j = 0; j < n2; j++) {
            if (a[i] == b[j]) {
                in_b = 1;
                break;
            }
        }
        // 如果不在 b 中，再检查是否已经加入结果
        if (!in_b) {
            int already = 0;
            for (int k = 0; k < cnt; k++) {
                if (result[k] == a[i]) {
                    already = 1;
                    break;
                }
            }
            if (!already) {
                result[cnt++] = a[i];
            }
        }
    }
    
    // 检查 b 中的元素
    for (int i = 0; i < n2; i++) {
        int in_a = 0;
        // 检查 b[i] 是否在 a 中出现
        for (int j = 0; j < n1; j++) {
            if (b[i] == a[j]) {
                in_a = 1;
                break;
            }
        }
        // 如果不在 a 中，再检查是否已经加入结果
        if (!in_a) {
            int already = 0;
            for (int k = 0; k < cnt; k++) {
                if (result[k] == b[i]) {
                    already = 1;
                    break;
                }
            }
            if (!already) {
                result[cnt++] = b[i];
            }
        }
    }
    
    // 输出结果
    for (int i = 0; i < cnt; i++) {
        if (i > 0) printf(" ");
        printf("%d", result[i]);
    }
    printf("\n");
    
    return 0;
}
```

# 2.查找指定字符
```
注意：
1.在输入c后要加入一个getchar（）读取换行符
2.str[i] = '\0';  // 手动添加字符串结束符：这一步挺重要的
```
## 题干
![[Pasted image 20251120223131.png]]
## 可实现程序：
```
#include <stdio.h>
int main() {
    char target;
    char str[81];
    int i = 0, index = -1;  // 初始化为-1表示没找到
    char ch;
    // 读取待查找的字符
    target = getchar();
    // 跳过第一行的换行符
    getchar();
    // 逐个读取字符直到换行符或达到最大长度
    while ((ch = getchar()) != '\n' && i < 80) {
        str[i] = ch;
        i++;
    }
    str[i] = '\0';  // 手动添加字符串结束符
    
    // 查找字符在字符串中的最大下标
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == target) {
            index = i;  // 记录最后一次出现的位置
        }
    }
    // 输出结果
    if (index != -1) {
        printf("index = %d\n", index);
    } else {
        printf("Not Found\n");
    }
    return 0;
}
```

# 3.找完数
```
注意点：
1.在求素数时只遍历到sqrt(m)，但是因子会出现在大于sqrt(m)的部分，所以以sqrt(m)为上限时需要顺便将乘积的另一半加入数组，此时要注意重复出现的情况
2.或者直接遍历1-m，也是一种选择，这样就不用排序了
```
## 题干
![[Pasted image 20251224124944.png]]
## 可运行代码
```
#include <stdio.h>
#include <math.h>

// 收集一个数的所有真因子（不包含自身）
int getFactors(int a[], int num) {
    int count = 0;
    for (int i = 1; i <= sqrt(num); i++) {  // 遍历到平方根，避免重复
        if (num % i == 0) {
            if (i != num) {  // 排除自身
                a[count++] = i;
            }
            if (i != 1 && i != num / i && num / i != num) {  // 避免重复添加（如i=2和num/i=3）
                a[count++] = num / i;
            }
        }
    }
    // 对因子排序（保证输出顺序递增）
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (a[i] > a[j]) {
                int temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
    }
    return count;  // 返回因子的个数
}

int main() {
    int m, n;
    int flag = 0;  // 标记是否找到完数
    scanf("%d %d", &m, &n);

    for (int num = m; num <= n; num++) {
        int factors[100];
        int factorCount = getFactors(factors, num);
        
        // 计算真因子的和
        int sum = 0;
        for (int i = 0; i < factorCount; i++) {
            sum += factors[i];
        }

        // 判断是否是完数
        if (sum == num) {
            flag = 1;
            // 按格式输出
            printf("%d = %d", num, factors[0]);
            for (int i = 1; i < factorCount; i++) {
                printf(" + %d", factors[i]);
            }
            printf("\n");
        }
    }

    // 若未找到完数
    if (!flag) {
        printf("None\n");
    }

    return 0;
}
```
# 判断上三角矩阵
```
注意运行超时问题
```
## 题干
![[Pasted image 20251114105925.png]]
## 可实现代码:
```
#include <stdio.h>

int main() {
    int t;
    scanf("%d", &t); 
    
    for (int k = 0; k < t; k++) {
        int n;
        scanf("%d", &n);  // 读取矩阵大小
        int a[n][n];
        
        // 读取矩阵元素
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                scanf("%d", &a[i][j]);
            }
        }
        
        int flag = 1;  // 假设是上三角矩阵
        
        // 检查主对角线以下的元素是否全为0
        for (int i = 1; i < n; i++) {        // 从第2行开始检查
            for (int j = 0; j < i; j++) {    // 检查每行中主对角线之前的元素
                if (a[i][j] != 0) {         // 发现非0元素
                    flag = 0;               // 不是上三角矩阵
                    break;                  // 跳出内层循环
                }
            }
            if (flag == 0) break;           // 跳出外层循环
        }
        
        // 输出结果
        if (flag == 1) {
            printf("YES\n");
        } else {
            printf("NO\n");
        }
    }
    
    return 0;
}
```

# 输出杨辉三角
```
注意点：
1.生成杨辉三角时内嵌循环用j <= i，而不用j<n,可以排掉j>i的列表元素
2.占四位用%4d，因为有两位数，不要“   %d”
```
## 题干
![[Pasted image 20251114123354.png]]
## 可实现代码:
```
include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);  // 输入N

    int triangle[10][10];  // 存储杨辉三角

    // 生成杨辉三角
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            if (j == 0 || j == i) {
                triangle[i][j] = 1;  // 每行首尾为1
            } else {
                triangle[i][j] = triangle[i-1][j-1] + triangle[i-1][j];
            }
        }
    }

    for (int i = 0; i < n; i++) {
        for (int k = 1; k < n-i; k++) {
            printf(" ");
        }
        for (int j = 0; j <= i; j++) {
            printf("%4d", triangle[i][j]);  // 输出数字
            }
        
        printf("\n");  // 每行结束后换行
    }

    return 0;
}
``` 

# 寻找鞍点
```
注意点：
1.直接定义matrix[6][6]相比n输入后定义n*n矩阵不会浪费太多内存，而且适用于旧版编译器，简单可靠。
2.考虑同一行可能出现2个及以上最大值
3.matrix[i][j] == max_val用于判断所有的最大值是否为该列的最小值
4.if (found) break注意学会使用这种写法
```
## 题干
![[Pasted image 20251114190222.png]]
##可实现代码
```
#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);
    
    int matrix[6][6];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%d", &matrix[i][j]);
        }
    }
    
    int found = 0;
    
    for (int i = 0; i < n; i++) {
        // 找第 i 行的最大值
        int max_val = matrix[i][0];
        for (int j = 1; j < n; j++) {
            if (matrix[i][j] > max_val) {
                max_val = matrix[i][j];
            }
        }
        
        // 遍历该行所有等于最大值的元素
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] == max_val) {
                // 检查在 j 列上是否是最小值
                int is_min = 1;
                for (int k = 0; k < n; k++) {
                    if (matrix[k][j] < max_val) {
                        is_min = 0;
                        break;
                    }
                }
                if (is_min) {
                    printf("%d %d\n", i, j);
                    found = 1;
                    break;
                }
            }
        }
        if (found) break;
    }
    
    if (!found) {
        printf("NONE\n");
    }
    
    return 0;
}
```

# 螺旋方阵
```
解题关键：不断的缩小四方的边界值
```

![[Pasted image 20251114201611.png]]
```
#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);
    
    int matrix[10][10] = {0};  // 初始化矩阵为0
    int num = 1;               // 当前要填入的数字
    int top = 0, bottom = n - 1, left = 0, right = n - 1;  // 边界
    
    while (num <= n * n) {
        // 从左到右填充上边
        for (int i = left; i <= right && num <= n * n; i++) {
            matrix[top][i] = num++;
        }
        top++;  // 上边界下移
        
        // 从上到下填充右边
        for (int i = top; i <= bottom && num <= n * n; i++) {
            matrix[i][right] = num++;
        }
        right--;  // 右边界左移
        
        // 从右到左填充下边
        for (int i = right; i >= left && num <= n * n; i--) {
            matrix[bottom][i] = num++;
        }
        bottom--;  // 下边界上移
        
        // 从下到上填充左边
        for (int i = bottom; i >= top && num <= n * n; i--) {
            matrix[i][left] = num++;
        }
        left++;  // 左边界右移
    }
    
    // 输出螺旋方阵
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%3d", matrix[i][j]);  // 每个数字占3位
        }
        printf("\n");
    }
    
    return 0;
}
```

# 单词首字母大写
```
不知道为什么我的程序运行超时
注意点：存在不止一个空格
```
## 题干
![[Pasted image 20251226140133.png]]
## 可运行代码
```
#include<stdio.h>
int main()
{
	int w = 1;
	char c;
	while((c = getchar())!='\n')    //如果碰到回车，程序结束
	{
		if(c==' ')    //遇到空格，说明下一个字符就是单词首字母
		{
			w = 1;    //w发生改变
		}
		else if(w==1)
		{
			w = 0;    //w复位，为下一次循环准备
			if(c>='a'&&c<='z')    //如果首字符为小写字母
			{
				c-=32;    //小写变大写
			}
		}
		putchar(c);
	}
}
```

# 求一元二次方程的根
```
出现分类讨论的情况较多，注意应分尽分
虚数根的情况比较难处理，应当注意
```
## 题干：
![[Pasted image 20251226180828.png]]
## 可运行代码
```
#include<stdio.h>
#include<math.h>
int main(){
    double a,b,c;
    scanf("%lf %lf %lf",&a,&b,&c);
    //特定条件的判断
    if(a == 0 && b == 0 && c == 0 ){
        printf("Zero Equation\n");
        return 0;
    }
    if(a == 0 && b == 0 && c != 0){
        printf("Not An Equation\n");
        return 0;
    }
    if(a == 0){
        printf("%.2lf\n",-c/b);
        return 0;
    }
    //求△
    int t=pow(b,2)-(4*a*c);
    if(t > 0){//两个实数根
        printf("%.2lf\n",-b/(2*a)+sqrt(t)/(2*a));
        printf("%.2lf\n",-b/(2*a)-sqrt(t)/(2*a));
    }else if(t == 0){//单根
         printf("%.2lf\n",-b/(2*a));
    }else{//两个虚数根
        t=-t;
        //注意：0.00会在gcc下被输出为-0.00，需要做特殊处理，输出正确的0.00。故先编号
        if(b == 0){
            b=-b;
        }
        printf("%.2lf+%.2lfi\n",-b/(2*a),sqrt(t)/(2*a));
        printf("%.2lf-%.2lfi\n",-b/(2*a),sqrt(t)/(2*a));
    }
    return 0;
}

```

# 统计单词长度
```
重点在于解决全空格+以空格结尾的情况应当如何考虑。
```
## 题干：
![[Pasted image 20251226231357.png]]
```
#include <stdio.h>
int main() {
    char c;
    int count = 0;
    int flag = 0;
    int has_word = 0;

    while ((c = getchar()) != '\n') {
        if (c != ' ') {
            flag = 1;
            count++;
            has_word = 1; // 只要有非空格字符，标记为有有效单词
        } else {
            if (flag == 1) { // 遇到空格且之前有单词，输出长度
                printf("%d ", count);
                count = 0;
                flag = 0;
            }
        }
    }

    // 情况1：循环结束后有未输出的单词（以单词结尾）
    if (flag == 1) {
        printf("%d ", count);
        has_word = 1;
    }

    if (has_word == 0) {
        printf("0 ");
    }
```