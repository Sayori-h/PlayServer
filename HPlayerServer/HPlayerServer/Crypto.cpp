#include "Crypto.h"
#include <openssl/md5.h>

//text��Ҫ���ܵ��ַ���
Buffer Crypto::MD5(const Buffer& text)
{
    Buffer result;
    Buffer data(16);
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, text, text.size());//����MD5
    MD5_Final(data, &md5);
    char temp[3] = "";
    for (size_t i = 0; i < data.size(); i++) {
        //��ǰ�ֽڵ�ʮ�����Ʊ�ʾ��ʽ��Ϊ�ַ��������洢��temp��
        snprintf(temp, sizeof(temp), "%02X", data[i] & 0XFF);
        result += temp;
    }
    return result;
}
