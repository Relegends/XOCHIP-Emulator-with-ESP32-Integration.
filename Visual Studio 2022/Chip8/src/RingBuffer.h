
#pragma once
#include <stdexcept>

template <typename T>
class RingBuffer {
public:
    RingBuffer();
    explicit RingBuffer(size_t capacity);
    ~RingBuffer();

    void Init(size_t capacity);

    void Push(const T& sample);

    T Pop();

    bool Empty() const;

    bool Full() const;

    size_t Size() const;

    size_t Capacity() const;
    
private:
    T* m_buffer;
    size_t m_capacity;
    size_t m_head;
    size_t m_tail;
    bool m_full;
    bool m_init;
};

