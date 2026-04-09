#include <ArduinoFake.h>
#include <unity.h>
#include "Ring.h"

using namespace fakeit;

void test_ring_empty(void)
{
    Ring<uint32_t, 5> ring;
    TEST_ASSERT_TRUE(ring.is_empty());
    TEST_ASSERT_FALSE(ring.is_full());
    TEST_ASSERT_EQUAL_INT(5, ring.capacity());
    TEST_ASSERT_EQUAL_INT(5, ring.space());
    TEST_ASSERT_EQUAL_INT(0, ring.size());
}

void test_ring_single(void)
{
    Ring<uint32_t, 1> ring;
    TEST_ASSERT_TRUE(ring.is_empty());
    ring.write(5);
    TEST_ASSERT_EQUAL_INT(5, ring.peek());
    TEST_ASSERT_EQUAL_INT(5, ring.read());
    TEST_ASSERT_TRUE(ring.is_empty());
}

void test_ring_vector(void)
{
    Ring<uint32_t, 4> ring;
    TEST_ASSERT_TRUE(ring.is_empty());
    ring.write({1, 2});
    TEST_ASSERT_EQUAL_INT(1, ring.read());
    TEST_ASSERT_EQUAL_INT(2, ring.read());
    ring.write({3, 4, 5});
    
    const std::vector<uint32_t> r1 = ring.read(1);
    const std::vector<uint32_t> r2 = ring.read(2);

    TEST_ASSERT_EQUAL_INT(1, r1.size());
    TEST_ASSERT_EQUAL_INT(2, r2.size());
    TEST_ASSERT_EQUAL_INT(3, r1[0]);
    TEST_ASSERT_EQUAL_INT(4, r2[0]);
    TEST_ASSERT_EQUAL_INT(5, r2[1]);

    TEST_ASSERT_TRUE(ring.is_empty());
}

void setUp(void)
{
    ArduinoFakeReset();
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_ring_empty);
    RUN_TEST(test_ring_single);
    RUN_TEST(test_ring_vector);

    return UNITY_END();
}
