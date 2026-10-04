#include <iomanip>
#include <iostream>
#include <span>
#include <vector>

std::span<double> row(std::vector<double>& data, int cols, int r) {
    return std::span<double>(data).subspan(static_cast<std::size_t>(r * cols), static_cast<std::size_t>(cols));
}

std::vector<double> transpose(const std::vector<double>& m, int rows, int cols) {
    std::vector<double> t(m.size());
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            t[static_cast<std::size_t>(c * rows + r)] = m[static_cast<std::size_t>(r * cols + c)];
    return t;
}

void print(std::vector<double>& m, int rows, int cols) {
    for (int r = 0; r < rows; ++r) {
        for (double v : row(m, cols, r)) std::cout << std::setw(6) << v;
        std::cout << '\n';
    }
    std::cout << '\n';
}

int main() {
    const int rows = 2, cols = 3;
    std::vector<double> m{1, 2, 3,
                          4, 5, 6};
    print(m, rows, cols);

    for (double& v : row(m, cols, 1)) v *= 10; // modify row 1 through the span
    print(m, rows, cols);

    auto t = transpose(m, rows, cols);
    print(t, cols, rows);
}
