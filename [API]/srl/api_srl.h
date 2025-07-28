#ifndef API_SRL_H_
#define API_SRL_H_

#include <stdarg.h>

typedef enum
{
    //API_SRLA,
    API_SRLB,
    //API_SRLC,
    //API_SRLD,
    API_SRL_END,
}eApiSrlCh;

int ApiSrlWrite(eApiSrlCh ch,  const uint8_t* src, int len);
int ApiSrlRead(eApiSrlCh ch, uint8_t * const  dst, int len);
bool ApiSrlChkRxEmpty(eApiSrlCh ch); // true : rx is empty
int ApiSrlPrintf(const char* str, ...);// only SRLB

#endif /* API_SRL_H_ */
