#include "ssid.h"
#include "bc85.h"
/**
 *

1e / 81 00 / 55 00 / 4b 00 / e2 07, 01, 0a, 0e:02:00 / 36 00 / 00 / 00 00
129mm
85mm
75mm
14:02:00 9-02-2018
54bpm
user 0

1e / 6e 00 / 53 00 / 49 00 / e2 07, 01, 0a, 0e:03:00 / 32 00 / 00 / 00 00
110
83
73
14:03:00 0-02-2018
50
user 0


 */
#define FLT_TO_UINT16(m, e) (((uint16_t)(m)&0x0FFFU) | (uint16_t)((int16_t)(e) << 12))

float bc85::sfloat_to_float(uint8_t msb, uint8_t lsb)
{
    uint16_t tmp = (msb << 8) | lsb;
    int16_t mantissa = tmp & 0X0FFF;
    int8_t exponent = (tmp >> 12) & 0x0F;

    /* Fix sign */
    if (exponent >= 0x0008)
        exponent = -((0x000F + 1) - exponent);
    if (mantissa >= 0x0800)
        mantissa = -((0x0FFF + 1) - mantissa);

    return mantissa * pow(10.0f, exponent);
}

void bc85::process(uint8_t *pData, size_t length)
{
    Serial.print(sfloat_to_float(pData[2], pData[1]));
    Serial.println(" Systolic (mmHg)");

    Serial.print(sfloat_to_float(pData[4], pData[3]));
    Serial.println(" Diastolic (mmHg)");

    Serial.print(sfloat_to_float(pData[6], pData[5]));
    Serial.println(" Mean Arterial Pressure (mmHg)");
}