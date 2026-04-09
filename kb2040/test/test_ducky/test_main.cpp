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

void test_duck_toKeys_a(void)
{
    Ducky ducky;
    std::vector<DuckyKeyPress> keys = ducky.toKeys("STRING a");
    TEST_ASSERT_EQUAL_INT(1, keys.size());
    TEST_ASSERT_TRUE((keys[0] == DuckyKeyPress{HID_KEY_A, 0, 50, 100}));
}

void test_duck_toKeys_A(void)
{
    Ducky ducky;
    std::vector<DuckyKeyPress> keys = ducky.toKeys("STRING A");
    TEST_ASSERT_EQUAL_INT(1, keys.size());
    TEST_ASSERT_TRUE((keys[0] == DuckyKeyPress{HID_KEY_A, KEYBOARD_MODIFIER_LEFTSHIFT, 50, 100}));
}

void test_duck_toKeys_SCROLLLOCK(void)
{
    Ducky ducky;
    std::vector<DuckyKeyPress> keys = ducky.toKeys("SCROLLLOCK");
    TEST_ASSERT_EQUAL_INT(1, keys.size());
    TEST_ASSERT_TRUE((keys[0] == DuckyKeyPress{HID_KEY_SCROLL_LOCK, 0, 50, 100}));
}

void setUp(void)
{
    ArduinoFakeReset();
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_duck_toKeysMock);
    RUN_TEST(test_duck_toKeys_a);
    RUN_TEST(test_duck_toKeys_A);
    RUN_TEST(test_duck_toKeys_SCROLLLOCK);

    return UNITY_END();
}
