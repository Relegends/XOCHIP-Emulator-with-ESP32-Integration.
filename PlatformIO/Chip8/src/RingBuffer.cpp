#include "RingBuffer.h"

template<typename T>
RingBuffer<T>::RingBuffer() : m_buffer(nullptr), m_capacity(0), m_head(0), m_tail(0), m_full(false), m_init(false) {}

template<typename T>
RingBuffer<T>::RingBuffer(size_t capacity)
    : m_buffer(new T[capacity]), m_capacity(capacity), m_head(0), m_tail(0), m_full(false), m_init(true) {}

template<typename T>
RingBuffer<T>::~RingBuffer()
{
    delete[] m_buffer;
}

template<typename T>
void RingBuffer<T>::Init(size_t capacity)
{
    if (m_init)
    {
        LOGE("Buffer is alredy initialized!");
        abort();
    }
    
    m_buffer = new T[capacity];
    m_capacity = capacity;
    m_head = 0;
    m_tail = 0;
    m_full = false;
    m_init = true;
}

template<typename T>
void RingBuffer<T>::Push(const T& sample) {
    if (!m_init) {
        LOGE("Buffer is not initialized");
        abort();
    }
    m_buffer[m_head] = sample;
    m_head = (m_head + 1) % m_capacity;
    if (m_full) {
        m_tail = (m_tail + 1) % m_capacity;
    }
    m_full = m_head == m_tail;
}

template<typename T>
T RingBuffer<T>::Pop() {
    if (!m_init) {
        LOGE("Buffer is not initialized");
        abort();
    }
    if (Empty()) {
        LOGE("Buffer is empty");
        abort();
    }
    auto value = m_buffer[m_tail];
    m_full = false;
    m_tail = (m_tail + 1) % m_capacity;
    return value;
}

template<typename T>
bool RingBuffer<T>::Empty() const {
    return (!m_full && (m_head == m_tail));
}

template<typename T>
bool RingBuffer<T>::Full() const {
    return m_full;
}

template <typename T>
size_t RingBuffer<T>::Size() const {
    if (!m_init) {
        LOGE("Buffer is not initialized");
        abort();
    }
    size_t size = m_capacity;
    if (!m_full) {
        if (m_head >= m_tail) {
            size = m_head - m_tail;
        }
        else {
            size = m_capacity + m_head - m_tail;
        }
    }
    return size;
}

template<typename T>
size_t RingBuffer<T>::Capacity() const {
    if (!m_init) {
        LOGE("Buffer is not initialized");
        abort();
    }
    return m_capacity;
}

// Explicit instantiation
template class RingBuffer<int16_t>;
template class RingBuffer<float>;