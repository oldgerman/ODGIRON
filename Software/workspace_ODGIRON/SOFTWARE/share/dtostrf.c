/*
作者：Apokli
链接：https://blog.csdn.net/BBBBBHY/article/details/109641537
	自带正负号显示
	参数说明：
	double val：要转换的浮点类型数据
	int len：输出字串最长显示长度
	int prec：要求小数点精度（小数点后几位）
	char* buf：要存放的字符串位置
	int buf_len：字符串buf的长度

	与Arduino的dtostrf不同之处：

	① 在最后多加了buf_len参数，即预先创建的buf串长度，防溢出，以及字符串数组占满了没有’\0’出问题。更安全（类似sprintf_s）

	②输出永远没有空格没有前导零（Arduino的dtostrf在满足精度要求但字符数量不足width时会在字符串前添加空格）

	注意事项：

	① 当浮点数翻译出的结果把buf占满了，程序会强制在最后加’\0’挤掉最后一位（尽量使用的时候就不要有越界风险吧~）
	② 小数点也算在len的长度里！！！
	③ 总长度或精度满足其一即翻译结束，例如：
	dtostrf(20.1234, 6, 5, buf, 10) 会存"20.123"
	dtostrf(2.5, 5, 2, buf, 10) 会存"2.50"

	代码测试：

	测试代码：

	int main() {

	   char buf[10] = "aefeag";
	   dtostrf(2.5, 5, 2, buf, 10);
	   printf("%s\n", buf);
	   dtostrf(20.1234, 6, 3, buf, 10);
	   printf("%s\n", buf);
	   dtostrf(0.98798234, 12, 9, buf, 10);
	   printf("%s\n", buf);
	   dtostrf(32, 7, 4, buf, 10);
	   printf("%s\n", buf);
	   return 0;
	}

	输出结果：
	2.50
	20.123
	0.9879823
	32.000
 */

//bug:当个位数无数字时，不显示0
#include "dtostrf.h"

void dtostrf(double val, int len, int prec, char* buf, uint8_t buf_len) {

    int int_count = 0;
    int index = 0;
    //判断正负
    if (val < 0) {
        buf[index] = '-';
        index ++;
        val = -val;
    }else{ //修改：无论正负都空一格
        buf[index] = ' ';
        index ++;
    }
    //先数一下整数部分长度
    while ((int) val > 0) {
        val /= 10.0;
        int_count++;
    }
    if (int_count == 0) {
         buf[index] = '0';
         index++;
    }

    while (index < len && index < buf_len) {
    	//如果整数部分结束
        if (int_count == 0) {
        	//如果一上来就是0
            if (index == 0) {
                buf[index] = '0';
                index++;
            }
            else {
                buf[index] = '.';
                index++;
                int_count--;
            }
        }
        //int_count此时已经是小数长度的相反数
        else if ( int_count >= -prec) {
            val *= 10.0;
            buf[index] = (int)val + '0';
            val -= (double)((int)val);
            index++;
            int_count--;
        }
        else break;
    }
    //强制加'\0'
    if (index == buf_len)  buf[index - 1] = '\0';
    else buf[index] = '\0';
}
