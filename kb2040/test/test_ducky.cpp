#include <ArduinoFake.h>
#include <unity.h>
#include "Ducky.h"

using namespace fakeit;

void test_duck_toKeysMock(void)
{
    Mock<IDucky> mock;

    When(Method(mock, toKeys)).Return(std::vector<DuckyKeyPress>());

    mock.get().toKeys("a");

    Verify(Method(mock, toKeys).Using("a")).Once();
}

void test_duck_toKeysA(void)
{
    Ducky ducky;
    std::vector<DuckyKeyPress> keys = ducky.toKeys("a");
    TEST_ASSERT_TRUE(keys.size() == 1);
    TEST_ASSERT_TRUE((keys[0] == DuckyKeyPress{HID_KEY_A, 50, 100}));
}

void setUp(void)
{
    ArduinoFakeReset();
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_duck_toKeysMock);
    RUN_TEST(test_duck_toKeysA);

    return UNITY_END();
}
