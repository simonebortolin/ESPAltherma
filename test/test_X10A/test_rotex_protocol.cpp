#include <iostream>
#include <fstream>
#include <unity.h>
#include <ArduinoFake.h>
#include <ArduinoJson.h>
#include <x10a.hpp>

#include "../mockSetup.hpp"

using namespace fakeit;
using namespace std;
using namespace X10A;

#define MODELS_CONFIG_SIZE 1024*10

#define X10AFile "build/X10A/English/PROTOCOL_S_ROTEX.json"

X10A_Config* TestX10AConfig = nullptr;

void setUp(void)
{
    ArduinoFakeReset();

    Mock<IX10ASerial> mockX10ASerial;

    When(Method(mockX10ASerial, operator bool)).Return(false);
    When(Method(mockX10ASerial, end)).AlwaysReturn();
    When(Method(mockX10ASerial, begin)).AlwaysReturn();

    TestX10AConfig = new X10A_Config();
    TestX10AConfig->X10A_PROTOCOL = X10AProtocol::S;

    ifstream inputFile(X10AFile);

    if(!inputFile) {
        throw runtime_error("Run script \"build_x10a_commands.py\" before running test!");
    }

    string jsonContent((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    DynamicJsonDocument configDoc(MODELS_CONFIG_SIZE);
    deserializeJson(configDoc, jsonContent);

    JsonObject jsonObject = configDoc.as<JsonObject>();

    jsonObject["PARAMETERS"] = jsonObject["Parameters"];
    jsonObject.remove("Parameters");

    x10a_set_serial(&mockX10ASerial.get());
    x10a_fill_config(jsonObject, TestX10AConfig);
    x10a_init(new MockDebugStream(), TestX10AConfig, true);
}

void tearDown(void)
{
    // clean stuff up here
}

void makeAssert(const String label, const String value) {
    for (size_t i = 0; i < TestX10AConfig->PARAMETERS_LENGTH; i++) {
        auto&& param = *TestX10AConfig->PARAMETERS[i];

        if(param.label.equals(label)) {
            TEST_ASSERT_EQUAL_STRING(value.c_str(), param.asString);
        }
    }
}

void overwriteBuffer(byte* buffer, byte* values, int bufferSize, int valuesSize)
{
    int i;
    for (i = 0; i < valuesSize; ++i) {
        buffer[i] = values[i];
    }

    for (; i < bufferSize; i++) {
        buffer[i] = 0;
    }
}

void test_rotex_converter_53(void)
{
    /*
    Possible values for reg 0x53

    0x53 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xac
    0x53 0x00 0x00 0x00 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xab
    0x53 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xab
    0x53 0x01 0x00 0x00 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xaa
    0x53 0x01 0x00 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xaa
    0x53 0x01 0x00 0x00 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xaa

    */

    RegistryBuffer buffer[1];

    buffer[0].RegistryID = 0x53;
    buffer[0].Success = true;

    byte buffer1[] = {0x53, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa};
    byte buffer2[] = {0x53, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa};

    overwriteBuffer(buffer[0].Buffer, buffer1, sizeof(buffer[0].Buffer), sizeof(buffer1));

    x10a_convert_values(buffer, 1, false);
    makeAssert("Circulation pump", "ON");
    makeAssert("External heater?", "ON");
    makeAssert("Priority to domestic water", "OFF");
    makeAssert("Burner inhibit from solaris", "OFF");

    overwriteBuffer(buffer[0].Buffer, buffer2, sizeof(buffer[0].Buffer), sizeof(buffer2));

    x10a_convert_values(buffer, 1, false);
    makeAssert("Circulation pump", "ON");
    makeAssert("External heater?", "OFF");
    makeAssert("Priority to domestic water", "OFF");
    makeAssert("Burner inhibit from solaris", "ON");
}

void test_rotex_converter_54(void)
{
    RegistryBuffer buffer[1];

    buffer[0].RegistryID = 0x53;
    buffer[0].Success = true;

    byte buffer1[] = {0x54, 0x98, 0x1e, 0x70, 0x1f, 0x1c, 0x24, 0x1c, 0x24, 0xd8, 0x31, 0x01, 0x00, 0x24, 0x00, 0x00, 0x00, 0xb8};
    byte buffer2[] = {0x54, 0x54, 0x13, 0xb0, 0x1c, 0x54, 0x1c, 0x54, 0x1c, 0x40, 0x2e, 0x01, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0b};
    byte buffer3[] = {0x54, 0x3c, 0x13, 0x98, 0x1c, 0x3c, 0x1c, 0x54, 0x1c, 0x40, 0x2e, 0x01, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x53};
    byte buffer4[] = {0x54, 0xb4, 0x12, 0x84, 0x1c, 0x24, 0x1c, 0x3c, 0x1c, 0x40, 0x2e, 0x01, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x20};
    byte buffer5[] = {0x54, 0xcc, 0x1a, 0x6c, 0x1c, 0xb0, 0x1c, 0x3c, 0x1c, 0x40, 0x2e, 0x01, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x8c};

    overwriteBuffer(buffer[0].Buffer, buffer1, sizeof(buffer[0].Buffer), sizeof(buffer1));
    x10a_convert_values(buffer, 1, false);
    makeAssert("Refrig. Temp. liquid side(C)", "30.5938");
    makeAssert("Inlet water temp.(C)", "31.4375");
    makeAssert("Outlet Water Temp.(C)", "36.1094");
    makeAssert("D(C)", "36.1094");
    makeAssert("DHW tank temp.(C)", "49.8438");
    makeAssert("F(C)", "0.00390625");
    makeAssert("Delta-Tr(deg)", "36");
    makeAssert("R/C Setpoint(C)", "0");

    overwriteBuffer(buffer[0].Buffer, buffer2, sizeof(buffer[0].Buffer), sizeof(buffer2));
    x10a_convert_values(buffer, 1, false);
    makeAssert("Refrig. Temp. liquid side(C)", "19.3281");
    makeAssert("Inlet water temp.(C)", "28.6875");
    makeAssert("Outlet Water Temp.(C)", "28.3281");
    makeAssert("D(C)", "28.3281");
    makeAssert("DHW tank temp.(C)", "46.25");
    makeAssert("F(C)", "0.00390625");
    makeAssert("Delta-Tr(deg)", "30");
    makeAssert("R/C Setpoint(C)", "0");

    overwriteBuffer(buffer[0].Buffer, buffer3, sizeof(buffer[0].Buffer), sizeof(buffer3));
    x10a_convert_values(buffer, 1, false);
    makeAssert("Refrig. Temp. liquid side(C)", "19.2344");
    makeAssert("Inlet water temp.(C)", "28.5938");
    makeAssert("Outlet Water Temp.(C)", "28.2344");
    makeAssert("D(C)", "28.3281");
    makeAssert("DHW tank temp.(C)", "46.25");
    makeAssert("F(C)", "0.00390625");
    makeAssert("Delta-Tr(deg)", "30");
    makeAssert("R/C Setpoint(C)", "0");

    overwriteBuffer(buffer[0].Buffer, buffer4, sizeof(buffer[0].Buffer), sizeof(buffer4));
    x10a_convert_values(buffer, 1, false);
    makeAssert("Refrig. Temp. liquid side(C)", "18.7031");
    makeAssert("Inlet water temp.(C)", "28.5156");
    makeAssert("Outlet Water Temp.(C)", "28.1406");
    makeAssert("D(C)", "28.2344");
    makeAssert("DHW tank temp.(C)", "46.25");
    makeAssert("F(C)", "0.00390625");
    makeAssert("Delta-Tr(deg)", "30");
    makeAssert("R/C Setpoint(C)", "0");

    overwriteBuffer(buffer[0].Buffer, buffer5, sizeof(buffer[0].Buffer), sizeof(buffer5));
    x10a_convert_values(buffer, 1, false);
    makeAssert("Refrig. Temp. liquid side(C)", "26.7969");
    makeAssert("Inlet water temp.(C)", "28.4219");
    makeAssert("Outlet Water Temp.(C)", "28.6875");
    makeAssert("D(C)", "28.2344");
    makeAssert("DHW tank temp.(C)", "46.25");
    makeAssert("F(C)", "0.00390625");
    makeAssert("Delta-Tr(deg)", "30");
    makeAssert("R/C Setpoint(C)", "0");
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_rotex_converter_53);
    RUN_TEST(test_rotex_converter_54);
    UNITY_END();

    return 0;
}