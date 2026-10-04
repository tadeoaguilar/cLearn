#include <array>
#include <cstddef>
#include <iostream>

template <typename T, std::size_t R, std::size_t C>
class Matrix {
public:
    Matrix() = default;
    Matrix(std::initializer_list<T> values) {
        std::size_t i = 0;
        for (const T& v : values) {
            if (i < R * C) data_[i++] = v;
        }
    }

    T& operator()(std::size_t r, std::size_t c) { return data_[r * C + c]; }
    const T& operator()(std::size_t r, std::size_t c) const { return data_[r * C + c]; }

    static constexpr std::size_t rows() { return R; }
    static constexpr std::size_t cols() { return C; }

    Matrix<T, C, R> transpose() const {
        Matrix<T, C, R> t;
        for (std::size_t r = 0; r < R; ++r)
            for (std::size_t c = 0; c < C; ++c) t(c, r) = (*this)(r, c);
        return t;
    }

    static Matrix identity()
        requires(R == C) // only square matrices get this function
    {
        Matrix m;
        for (std::size_t i = 0; i < R; ++i) m(i, i) = T{1};
        return m;
    }

    friend Matrix operator+(const Matrix& a, const Matrix& b) {
        Matrix out;
        for (std::size_t i = 0; i < R * C; ++i) out.data_[i] = a.data_[i] + b.data_[i];
        return out;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix& m) {
        for (std::size_t r = 0; r < R; ++r) {
            for (std::size_t c = 0; c < C; ++c) os << m(r, c) << '\t';
            os << '\n';
        }
        return os;
    }

private:
    std::array<T, R * C> data_{};
};

// The shared dimension C appears in both operands: mismatches can't even be expressed.
template <typename T, std::size_t R, std::size_t C, std::size_t K>
Matrix<T, R, K> operator*(const Matrix<T, R, C>& a, const Matrix<T, C, K>& b) {
    Matrix<T, R, K> out;
    for (std::size_t r = 0; r < R; ++r)
        for (std::size_t k = 0; k < K; ++k) {
            T sum{};
            for (std::size_t c = 0; c < C; ++c) sum += a(r, c) * b(c, k);
            out(r, k) = sum;
        }
    return out;
}

int main() {
    Matrix<int, 2, 3> a{1, 2, 3,
                        4, 5, 6};
    Matrix<int, 3, 2> b{7, 8,
                        9, 10,
                        11, 12};

    std::cout << "a =\n" << a << "\na^T =\n" << a.transpose();
    std::cout << "\na * b (2x2) =\n" << a * b;
    std::cout << "\na + a =\n" << a + a;
    std::cout << "\nI3 =\n" << Matrix<int, 3, 3>::identity();

    // a * a;                          // ERROR: no operator* for 2x3 * 2x3
    // Matrix<int, 2, 3>::identity();  // ERROR: constraints not satisfied (R == C)
}
