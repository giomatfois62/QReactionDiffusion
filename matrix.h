#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

template<class T>
class Matrix
{
public:
    Matrix() : rows(0), cols(0) {}
    Matrix(const int r, const int c) : rows(r), cols(c)
    {
        m_data.resize(r * c);
    }

    T& operator()(const int &i, const int &j)
    {
        return m_data[i * cols + j];
    }

    void fill(const T &val)
    {
        for(int i = 0; i < rows; i++)
            for(int j = 0; j < cols; j++)
                m_data[i * cols + j] = val;
    }

    T* data()
    {
        return m_data.data();
    }

    int rows, cols;

private:
    std::vector<T> m_data;
};

#endif // MATRIX_H
