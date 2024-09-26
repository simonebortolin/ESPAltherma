#pragma once

#include <Arduino.h>
#include <IDebugStream.hpp>
#include "parameterDef.hpp"

namespace X10A
{
    class Converter
    {
    public:
        void convert(IDebugStream* debugStream, ParameterDef *def, byte *data);

    private:
        void convertTable300(IDebugStream* debugStream, byte *data, int tableID, char *ret);

        void convertTable203(byte *data, char *ret);

        void convertTable204(byte *data, char *ret);

        double convertTable312(byte *data);

        void convertTable315(byte *data, char *ret);

        void convertTable316(byte *data, char *ret);

        void convertTable200(byte *data, char *ret);

        void convertTable217(byte *data, char *ret);

        double convertPress2Temp(double data);

        unsigned short getUnsignedValue(byte *data, int dataSize, int cnvflg);

        short getSignedValue(byte *data, int datasize, int cnvflg);

        size_t strCopy(char *dest, const char *src, size_t size);
    };

}

extern X10A::Converter converter;