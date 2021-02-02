#include <stdio.h>

unsigned half_adder(unsigned m1, unsigned m2, unsigned &out_carry)
{
    unsigned r = m1 ^ m2;
    out_carry = m1 & m2;
    return r;
}

unsigned adder(unsigned m1, unsigned m2, unsigned int carry, unsigned &out_carry)
{

    unsigned x = 0;
    unsigned y = half_adder(m1, m2, x);

    unsigned carry2 = 0;
    unsigned w = half_adder(y, carry, carry2);

    out_carry = carry2 | x;
    return w;
}

unsigned int add(unsigned a, unsigned b, unsigned &carry_left)
{

    unsigned r = 0;
    for (int i = 0; i < 32; i++)
    {
        unsigned int mask = 1 << i;
        unsigned int m1 = (a & mask) >> i;
        unsigned int m2 = (b & mask) >> i;

        unsigned new_carry = 0;
        unsigned int t = adder(m1, m2, carry_left, new_carry);
        carry_left = new_carry;

        r |= t << i;
    }
    return r;
}

int signed_add(int a, int b, unsigned &of)
{
    unsigned cf = 0;
    int out = add(a, b, cf);
    //符号位相同才存在溢出,
    if ((a & (1 << 31)) == (b & (1 << 31)))
    {
        //如果结果符号与输入相同,则不溢出,否则溢出
        of = (a >> 31 & 1) != (out >> 31 & 1);
    }
    else
    {
        //不同的时候不可能溢出
        of = 0;
    }
    return out;
}

int neg(int a)
{
    return (~a) + 1;
}
int signed_div(int a, int b, int &rem)
{
    int dividend = a;
    unsigned long long divider = b;
    if (a < 0)
    {
        dividend = neg(a);
    }
    if (b < 0)
    {
        divider = neg(b);
    }
    divider = divider << 32;

    //商
    int r = 0;
    //除法器流程
    //1.将除数扩展到64bit,并将除数移到高32bit
    //2.不断右移除数,如果除数比被除数大则这位商为0,否则为1,商为1时,将被除数-除数, 商不断左移
    //3.循环32次
    for (int i = 0; i < 32; i++)
    {
        unsigned c = 0;
        divider = divider >> 1;
        if (divider > dividend)
        {
            c = 0;
        }
        else
        {
            unsigned of = 0;
            //dividend - divider
            dividend = signed_add(dividend, neg(divider), of);
            c = 1;
        }
        r = (r << 1) | c;
    }
    //最后被除数就是余数
    rem = (int)dividend;
    if (((a >> 31) & 1) != ((b >> 31) & 1))
    {
        //输入异号结果即为负
        r = neg(r);
        rem = neg(rem);
    }

    return r;
}

unsigned int mul(unsigned a, unsigned b, unsigned &carry_left)
{
    unsigned sum = 0;
    carry_left = 0;
    while (b != 0)
    {
        unsigned t = b & 1;
        if (t)
        {
            unsigned cf = 0;
            sum = add(sum, a, cf);
            if (cf != 0)
            {
                carry_left = 1;
            }
        }
        a = a << 1;
        b = b >> 1;
    }
    return sum;
}

int signed_mul(int a, int b, unsigned &of)
{
    unsigned cf = 0;
    int out = mul(a, b, cf);
    //不同号,结果肯定是负数,否则溢出
    if ((a & (1 << 31)) != (b & (1 << 31)))
    {
        of = (out >> 1 & 1) == 0;
    }
    else if ((a & (1 << 31)) == (b & (1 << 31)))
    {
        //同号,结果位正,如果符号位位1, 则溢出,
        of = out >> 31 & 1;
    }
    return out;
}

int addINT(int a, int b, unsigned &of)
{
    int r = signed_add(a, b, of);
    printf("addInt a=%d b=%d r=%d of=%u\n", a, b, r, of != 0);
    return r;
}

int mulINT(int a, int b, unsigned &of)
{
    int r = signed_mul(a, b, of);
    printf("mulINT a=%d b=%d r=%d of=%u\n", a, b, r, of != 0);
    return r;
}

void dump_float(float a) {
    unsigned tail = (unsigned&)a << 9 >> 9;
    unsigned level = (unsigned&)a << 1 >> 1 >> 23;
    unsigned sign = (unsigned&)a>>31;
    printf("sign:%x level:%x tail:%x\n", sign, level, tail);
}

float float_add(float a, float b) {
    //dump_float(a);
    //dump_float(b);

    unsigned tail = (unsigned&)a << 9 >> 9;
    unsigned level = (unsigned&)a << 1 >> 1 >> 23;
    unsigned sign = (unsigned&)a>>31;

    unsigned tail2 = (unsigned&)b << 9 >> 9;
    unsigned level2 = (unsigned&)b << 1 >> 1 >> 23;
    unsigned sign2 = (unsigned&)b>>31;

    
    unsigned bigTail = 0;
    unsigned bigLevel = 0;
    unsigned bigSign = 0;

    unsigned smallTail = 0;
    unsigned smallLevel = 0;
    unsigned smallSign = 0;
    if (level > level2) {
        bigTail = tail;
        bigLevel = level;
        bigSign = sign;

        smallTail = tail2;
        smallLevel = level2;
        smallSign = sign2;
    }
    else {
        bigTail = tail2;
        bigLevel = level2;
        bigSign = sign2;

        smallTail = tail;
        smallLevel = level;
        smallSign = sign;
    }

    unsigned levelMargin = bigLevel - smallLevel;

    smallTail = (1 << 23) | smallTail;
    bigTail = (1 << 23) | bigTail;

    //对阶
    smallTail = smallTail >> levelMargin;

    if (bigSign == 1) {
        bigTail = neg(bigTail);
    }
    if (smallSign == 1) {
        smallTail = neg(smallTail);
    }


    //尾数相加
    unsigned st = smallTail + bigTail;
    //printf("%x\n", st);
    unsigned sgr = 0;
    if ((st >> 31) & 1) {
        sgr = 1;
        //负数取反,由符号为决定正负
        st = neg(st);
    }
    if (st == 0) {
        //对阶后加法,结果直接为0,结果就是0,0是特殊的表达
        return 0;
    }
    int top1Index = 23;
    unsigned mask = 1 << 31;
    //printf("%x\n", st);
    //标准化,保证24位总是1,而且是最高位
    //由于24位的加法最多也就到25位,因此右移只需要管25位是否为1即可
    if (((st>>24) & 1) == 1) {
        st = st >> 1;
        bigLevel += 1;
    }
    else {
        //如果24位不为1,则必然小于1<<23
        while (st < (1 << 23)) {
            st = st << 1;
            bigLevel -= 1;
        }
    }
    
    //printf("%x\n", st);

    st = st << 9 >> 9;

    unsigned u = (sgr << 31 | bigLevel << 23 | st);
    float r = (float&)u;

    return r;
}

/*
unsigned int mul_para(unsigned a, unsigned b, unsigned n) {

    a = a << n;
    b = b >> n;
    unsigned t = b & 1;
    unsigned int r = 0;
    if (t) {
        r = a;
    }
    else {
        r = 0;
    }
    return r;
}
*/

int main()
{
    
    float r = float_add(1.1, 1.5);
    //dump_float(r);
    printf("%.7f\n", r);
    

    r = float_add(0, -0.5);
    //dump_float(r);
    printf("%.7f\n", r);
    

    r = float_add(-1.1, -0.5);
    //dump_float(r);
    printf("%.7f\n", r);

    r = float_add(900, -0.5);
    //dump_float(r);
    printf("%.7f\n", r);


    r = float_add(0.3, -0.44);
    //dump_float(r);
    printf("%.7f\n", r);

    /*
    float f = 1.1 + 0.5;
    printf("%.7f\n", f);
    */
    /*
    unsigned sum = 0;
    addINT(2, 3, sum);
    addINT(-2, -3, sum);
    addINT(-2147483648, 0, sum);
    addINT(-2147483647, -2147483647, sum);
 
    unsigned mul = 0;
    mulINT(2, 15, mul);
    mulINT(213, 155, mul);
    mulINT(1123412345, 2387648, mul);
 
    mulINT(-2, 15, mul);
    mulINT(-213, 155, mul);
    mulINT(-1123412345, 2387648, mul);
 
    mulINT(-2, -15, mul);
    mulINT(-213, -155, mul);
    mulINT(-2147483648, -1, mul);
    */
    /*
    int rem = 0;
    int r = signed_div(7, 2, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(307, 15, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(2147483647, 35, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(-7, 2, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(-307, 15, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(-2147483647, 35, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(-7, -2, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(-307, -15, rem);
    printf("%d %d\n", r, rem);
    r = signed_div(-2147483647, -35, rem);
    printf("%d %d\n", r, rem);
    */

    return 0;
}
