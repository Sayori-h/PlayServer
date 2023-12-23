#include "Crypto.h"
#include <openssl/md5.h>

//text：要加密的字符串
Buffer Crypto::MD5(const Buffer& text)
{
    Buffer result;
    Buffer data(16);
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, text, text.size());//计算MD5
    MD5_Final(data, &md5);
    char temp[3] = "";
    for (size_t i = 0; i < data.size(); i++) {
        //当前字节的十六进制表示格式化为字符串，并存储在temp中
        snprintf(temp, sizeof(temp), "%02X", data[i] & 0XFF);
        result += temp;
    }
    return result;
}
