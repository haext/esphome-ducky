#ifndef _RING_H
#define _RING_H

#include <stdint.h>
#include <array>
#include <vector>
#include <stdexcept>

class invalid_state : public std::logic_error {
    public:
        explicit invalid_state(const std::string& message)
            : std::logic_error(message) {}
};

template <typename T> class IRing {
    public:
        virtual uint32_t size() = 0;
        virtual uint32_t capacity() = 0;
        virtual uint32_t space() = 0;
        virtual bool is_full() = 0;
        virtual bool is_empty() = 0;
};

template <typename T, uint32_t _capacity>  class Ring : public IRing<T> {
    public:
        Ring() : buffer({})
        {
            head = 0;
            tail = 0;
            lastRead = true;
        }

        uint32_t capacity()
        {
            return _capacity;
        }

        uint32_t size()
        {
            const uint32_t capacity = this->capacity();
            const uint32_t diff = (head - tail) % capacity;
            if (diff != 0) return diff;
            return lastRead ? 0 : capacity;
        }

        uint32_t space()
        {
            return capacity() - size();
        }

        bool is_full()
        {
            return (head == tail) && !lastRead;
        }

        bool is_empty()
        {
            return (head == tail) && lastRead;
        }

        T peek()
        {
            if (is_empty()) throw invalid_state("Ring is empty");
            return buffer[tail];
        }

        T read()
        {
            if (is_empty()) throw invalid_state("Ring is empty");
            uint32_t prev = tail;
            tail = (tail + 1) % capacity();
            lastRead = true;
            return buffer[prev];
        }

        void write(T value)
        {
            if (is_full()) throw invalid_state("Ring is full");
            uint32_t prev = head;
            head = (head + 1) % capacity();
            lastRead = false;
            buffer[prev] = value;
        }

        std::vector<T> read(uint32_t count)
        {
            if (size() < count) throw invalid_state("Ring does not have enough data");
            uint32_t prev = tail;
            tail = (tail + count) % capacity();
            lastRead = true;
            if (tail < prev)
            {
                // Split the read into two, since it wrapped
                const uint32_t end = capacity() - prev;
                std::vector<T> retVal(buffer.begin() + prev, buffer.begin() + prev + end);
                retVal.insert(retVal.end(), buffer.begin(), buffer.begin() + (count - end));
                return retVal;
            }
            else return std::vector<T>(buffer.begin() + prev, buffer.begin() + prev + count);
        }

        void write(std::vector<T> values)
        {
            if (space() < values.size()) throw invalid_state("Ring does not have enough space");
            uint32_t prev = head;
            head = (head + values.size()) % capacity();
            lastRead = false;
            if (head < prev)
            {
                // Split the write into two, since it wrapped
                const uint32_t end = capacity() - prev;
                std::copy(values.begin(), values.begin() + end, buffer.begin() + prev);
                std::copy(values.begin() + end, values.end(), buffer.begin());
            }
            else std::copy(values.begin(), values.end(), buffer.begin() + prev);
        }

    protected:
        std::array<T, _capacity> buffer;
        uint32_t head, tail;
        bool lastRead;
};


#endif