/*-------------------------------------------------------
 Data Encryption Standard  56位密钥加密64位数据
 2016.04.15
 --------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "bool.h"
#include "tables.h"

void BitsCopy(bool *DatOut,bool *DatIn,int Len);                       // 数组复制

void TablePermute(bool *DatOut,bool *DatIn,const char *Table,int Num); // 位表置换函数
void LoopMove(bool *DatIn,int Len,int Num);                            // 循环左移 Len长度 Num移动位数
void Xor(bool *DatA,bool *DatB,int Num);                               // 异或函数

void S_Change(bool DatOut[32],bool DatIn[48]);                         // S盒变换
void F_Change(bool DatIn[32],bool DatKi[48]);                          // F函数

void SetKey(char KeyIn[64]);                                           // 设置密钥
void PlayDes(char MesOut[64],char MesIn[64]);                          // 执行DES加密
void KickDes(char MesOut[64],char MesIn[64]);                          // 执行DES解密



int main()
{
    int i=0;
    char MesHex[64]={0};          // 加密密文 64位二进制流
    char MyKey[64]={0};           // 加密密钥 64位二进制流
    char YourKey[64]={0};         // 解密密钥 64位二进制流
    char MyMessage[64]={0};       // 初始明文 64位二进制流
    char buffer[71]={0};          // 读取输入数据时的缓存字符数组，64位二进制流+7位空格
    
    // 读取初始明文
    printf("欢迎来到DES加解密算法演示实验！\n请输入64位二进制流明文(相邻字节以空格为间隔):\n");
    gets(buffer);
    for(i=0;i<64;i++)
    {
        MyMessage[i]=buffer[i+i/8];
    }

    // 读取加密密钥
    printf("\n请输入64位二进制流加密秘钥(相邻字节以空格为间隔):\n");
    gets(buffer);
    for(i=0;i<64;i++)
    {
        MyKey[i]=buffer[i+i/8];
    }

    // 设置密钥 得到子密钥Ki
    SetKey(MyKey);
    // 执行DES加密
    PlayDes(MesHex,MyMessage);
    
    // 打印加密后密文内容
    printf("\n信息已加密:\n");
    for(i=0;i<64;i++)
    {
        if ((i%8==0)&&(i!=0)) printf(" ");
        printf("%c",MesHex[i]);
    }
    printf("\n");
    printf("\n");
    
    // 读取解密密钥
    printf("请输入64位二进制流解密密钥(相邻字节以空格为间隔):\n");
    gets(buffer);
    for(i=0;i<64;i++)
    {
        YourKey[i]=buffer[i+i/8];
    }
    // 设置密钥 得到子密钥Ki
    SetKey(YourKey);
    // 解密输出到MyMessage
    KickDes(MyMessage,MesHex);
    
    // 打印解密后明文内容
    printf("\n解密成功，演示结束:\n");
    for(i=0;i<64;i++)
    {
        if ((i%8==0)&&(i!=0)) printf(" ");
        printf("%c",MyMessage[i]);
    }
    printf("\n");
}


// 把DatIn开始的长度位Len位的二进制复制到DatOut后
void BitsCopy(bool *DatOut,bool *DatIn,int Len)
{
    int i=0;
    for(i=0;i<Len;i++)
    {
        DatOut[i]=DatIn[i];
    }
}

// 字节转换成位函数 每8次换一个字节 每次向右移一位 和1与取最后一位 共64位
void ByteToBit(bool *DatOut,char *DatIn,int Num)
{
    int i=0;
    for(i=0;i<Num;i++)
    {
        DatOut[i]=(DatIn[i/8]>>(i%8))&0x01;
    }
}



// 表置换函数
void TablePermute(bool *DatOut,bool *DatIn,const char *Table,int Num)
{
    int i=0;
    static bool Temp[256]={0};
    for(i=0;i<Num;i++)                // Num为置换的长度
    {
        Temp[i]=DatIn[Table[i]-1];  // 原来的数据按对应的表上的位置排列
    }
    BitsCopy(DatOut,Temp,Num);       // 把缓存Temp的值输出
}

// 子密钥的移位
void LoopMove(bool *DatIn,int Len,int Num) // 循环左移 Len数据长度 Num移动位数
{
    static bool Temp[256]={0};    // 缓存
    BitsCopy(Temp,DatIn,Num);       // 将数据最左边的Num位(被移出去的)存入Temp
    BitsCopy(DatIn,DatIn+Num,Len-Num); // 将数据左边开始的第Num移入原来的空间
    BitsCopy(DatIn+Len-Num,Temp,Num);  // 将缓存中移出去的数据加到最右边
}

// 按位异或
void Xor(bool *DatA,bool *DatB,int Num)           // 异或函数
{
    int i=0;
    for(i=0;i<Num;i++)
    {
        DatA[i]=DatA[i]^DatB[i];                  // 异或
    }
}

// 输入48位 输出32位 与Ri异或
void S_Change(bool DatOut[32],bool DatIn[48])     // S盒变换
{
    int i,X,Y;                                    // i为8个S盒
    for(i=0,Y=0,X=0;i<8;i++,DatIn+=6,DatOut+=4)   // 每执行一次,输入数据偏移6位
    {                                              // 每执行一次,输出数据偏移4位
        Y=(DatIn[0]<<1)+DatIn[5];                          // af代表第几行
        X=(DatIn[1]<<3)+(DatIn[2]<<2)+(DatIn[3]<<1)+DatIn[4]; // bcde代表第几列
        ByteToBit(DatOut,&S_Box[i][Y][X],4);      // 把找到的点数据换为二进制
    }
}

// F函数
void F_Change(bool DatIn[32],bool DatKi[48])       // F函数
{
    static bool MiR[48]={0};             // 输入32位通过E选位变为48位
    TablePermute(MiR,DatIn,E_Table,48);
    Xor(MiR,DatKi,48);                   // 和子密钥异或
    S_Change(DatIn,MiR);                 // S盒变换
    TablePermute(DatIn,DatIn,P_Table,32);   // P置换后输出
}

// 设置密钥 获取子密钥Ki
void SetKey(char KeyIn[64])
{
    int i=0;
    static bool KeyBit[64]={0};                // 密钥二进制存储空间
    static bool *KiL=&KeyBit[0],*KiR=&KeyBit[28];  // 前28,后28共56
    for(i=0;i<64;i++)                              // 把密钥存入KeyBit
    {
        KeyBit[i]=KeyIn[i];
    }
    TablePermute(KeyBit,KeyBit,PC1_Table,56);      // PC1表置换 56次
    for(i=0;i<16;i++)
    {
        LoopMove(KiL,28,Move_Table[i]);       // 前28位左移
        LoopMove(KiR,28,Move_Table[i]);          // 后28位左移
        TablePermute(SubKey[i],KeyBit,PC2_Table,48);
        // 二维数组 SubKey[i]为每一行起始地址
        // 每移一次位进行PC2置换得 Ki 48位
    }
}

// 执行DES加密
void PlayDes(char MesOut[64],char MesIn[64])
{                                           // 字节输入 Bin运算 Hex输出
    int i=0;
    static bool MesBit[64]={0};        // 明文二进制存储空间 64位
    static bool Temp[32]={0};
    static bool *MiL=&MesBit[0],*MiR=&MesBit[32]; // 前32位 后32位
    for(i=0;i<64;i++)
    {
        MesBit[i]=MesIn[i];
    }
    TablePermute(MesBit,MesBit,IP_Table,64);    // IP置换
    for(i=0;i<16;i++)                       // 迭代16次
    {
        BitsCopy(Temp,MiR,32);            // 临时存储
        F_Change(MiR,SubKey[i]);           // F函数变换
        Xor(MiR,MiL,32);                  // 得到Ri
        BitsCopy(MiL,Temp,32);            // 得到Li
    }
    TablePermute(MesBit,MesBit,IPR_Table,64);
    for(i=0;i<64;i++)
    {
        MesOut[i]=MesBit[i];
    }
}

// 执行DES解密
void KickDes(char MesOut[64],char MesIn[64])
{                                                // Hex输入 Bin运算 字节输出
    int i=0;
    static bool MesBit[64]={0};        // 密文二进制存储空间 64位
    static bool Temp[32]={0};
    static bool *MiL=&MesBit[0],*MiR=&MesBit[32]; // 前32位 后32位
    for(i=0;i<64;i++)
    {
        MesBit[i]=MesIn[i];
    }
    TablePermute(MesBit,MesBit,IP_Table,64);    // IP置换
    for(i=15;i>=0;i--)
    {
        BitsCopy(Temp,MiL,32);
        F_Change(MiL,SubKey[i]);
        Xor(MiL,MiR,32);
        BitsCopy(MiR,Temp,32);
    }
    TablePermute(MesBit,MesBit,IPR_Table,64);
    for(i=0;i<64;i++)
    {
        MesOut[i]=MesBit[i];
    }
}