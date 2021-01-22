#include <stdio.h>

unsigned half_adder(unsigned m1, unsigned m2, unsigned &out_carry) {
    unsigned r = m1 ^ m2;
    out_carry = m1 & m2;
    return r;
}

unsigned adder(unsigned m1, unsigned m2, unsigned int carry, unsigned &out_carry) {

    unsigned x = 0;
    unsigned y = half_adder(m1, m2, x);

    unsigned carry2 = 0;
    unsigned w = half_adder(y, carry, carry2);
    
    out_carry = carry2 | x;
    return w;
}

unsigned int add(unsigned a, unsigned b, unsigned &carry_left) {

    unsigned r = 0;
    for (int i = 0; i < 32; i++) {
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



int signed_add(int a, int b, unsigned &of) {
    unsigned cf = 0;
    int out = add(a, b, cf);
    //符号位相同才存在溢出,
    if ((a & (1<<31)) == (b & (1<<31))) {
        //如果结果符号与输入相同,则不溢出,否则溢出
        of = (a>>31 & 1) != (out>>31 & 1);
    }
    else {
        //不同的时候不可能溢出
        of = 0;
    }
    return out;
}

int neg(int a) {
    return (~a)+1;
}
int signed_div(int a, int b, int &rem) {
    int dividend = a;
    unsigned long long divider = b;
    if (a < 0) {
        dividend = neg(a);
    }
    if (b < 0) {
        divider = neg(b);
    }
    divider = divider << 32;

    //商
    int r = 0;
    //除法器流程
    //1.将除数扩展到64bit,并将除数移到高32bit
    //2.不断右移除数,如果除数比被除数大则这位商为0,否则为1,商为1时,将被除数-除数, 商不断左移
    //3.循环32次
    for (int i = 0; i < 32; i++) {
        unsigned c = 0;
        divider = divider >> 1;
        if (divider > dividend) {
            c = 0;
        }
        else {
            unsigned of = 0;
            //dividend - divider
            dividend = signed_add(dividend , neg(divider), of);
            c = 1;
        }
        r = (r << 1) | c;
    }
    //最后被除数就是余数
    rem = (int)dividend;
    if (((a >> 31) & 1) != ((b >> 31) & 1)) {
        //输入异号结果即为负
        r = neg(r);
        rem = neg(rem);
    }
    
    return r;
}

unsigned int mul(unsigned a, unsigned b, unsigned &carry_left) {
    unsigned sum = 0;
    carry_left = 0;
    while(b!=0) {
        unsigned t = b & 1;
        if (t) {
            unsigned cf = 0;
            sum = add(sum, a, cf);
            if (cf!=0) {
                carry_left = 1;
            }
        }
        a = a<<1;
        b = b>>1;
    }
    return sum;
}


int signed_mul(int a, int b, unsigned &of) {
    unsigned cf = 0;
    int out = mul(a, b, cf);
    //不同号,结果肯定是负数,否则溢出
    if ((a & (1<<31)) != (b & (1<<31))) {
        of = (out>>1 & 1) == 0;
    }
    else if ((a & (1<<31)) == (b & (1<<31))) {
        //同号,结果位正,如果符号位位1, 则溢出,
        of = out>>31 & 1;
    }
    return out;
}

int addINT(int a, int b, unsigned &of) {
    int r = signed_add(a, b, of);
    printf("addInt a=%d b=%d r=%d of=%u\n", a, b, r, of!=0);
    return r;
}

int mulINT(int a, int b, unsigned &of) {
    int r = signed_mul(a, b, of);
    printf("mulINT a=%d b=%d r=%d of=%u\n", a, b, r, of!=0);
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


int main() {
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
    int rem = 0;
    int r = signed_div(-16, 5, rem);
    printf("%d %d\n", r, rem);
    /*
    unsigned int a = -2147483648;
    unsigned int b = 0;
    unsigned of = 0;
    int r = signed_add(a, b, of);

    if (of != 0) {
        printf("overflow!!!\n");
    }
    printf("%d\n", r);
    */
    return 0;
    
   
  /*
    unsigned a = 7;
    unsigned b = 8;
    unsigned r[32] = {0};
    unsigned sum = 0;
    unsigned carry_left = 0;
    for(int i=0; i < 32; i++) {
        //这里开n个线程或者协程即可并行
        r[i] = mul_para(a, b, i);
    }
    for (int i = 0; i < 32; i++) {
        sum = add(sum, r[i], carry_left);
    }
    printf("sum %d\n", sum);
  */
}
