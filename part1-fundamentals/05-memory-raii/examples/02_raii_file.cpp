// An RAII wrapper over the C FILE* API. The destructor ALWAYS closes the file.
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

class File {
public:
    File(const std::string& path, const char* mode) : f_{std::fopen(path.c_str(), mode)}, path_{path} {
        if (!f_) throw std::runtime_error("cannot open " + path);
        std::cout << "  opened " << path_ << '\n';
    }
    ~File() {
        if (f_) {
            std::fclose(f_);
            std::cout << "  closed " << path_ << '\n';
        }
    }

    // Copying would close the same FILE* twice → forbid it.
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    void write(const std::string& text) {
        if (std::fputs(text.c_str(), f_) < 0) throw std::runtime_error("write failed");
    }

    std::string read_all() {
        std::string out;
        char buf[256];
        while (std::fgets(buf, sizeof buf, f_)) out += buf;
        return out;
    }

private:
    std::FILE* f_;
    std::string path_;
};

int main() {
    const std::string path = "raii_demo.txt";
    {
        File f{path, "w"};
        f.write("line one\n");
        f.write("line two\n");
    } // closed here, automatically

    try {
        File f{path, "r"};
        std::cout << f.read_all();
        throw std::runtime_error("something failed mid-way");
    } catch (const std::exception& e) {
        std::cout << "  caught: " << e.what() << " (file was still closed above)\n";
    }

    std::remove(path.c_str());
}
