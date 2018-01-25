//
// zbase64.h, xieling, 2013/03/18
//

#ifndef ___ZBASE64_H___
#define ___ZBASE64_H___

class ZBase64
{
public:
    ZBase64(){};
    ~ZBase64(){};
    unsigned int static GetEncodeLen(unsigned int uEnLen);
    unsigned int static GetDecodeLen(unsigned int uDeLen);
    int static Encode(const unsigned char *pIn, unsigned int uInLen, unsigned char *pOut, unsigned int *uOutLen);
    int static Decode(const unsigned char *pIn, unsigned int uInLen, unsigned char *pOut, unsigned int *uOutLen) ;
};

#endif // ___ZBASE64_H___
