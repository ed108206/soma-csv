#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <locale>
#include <iomanip>


struct apostrophe_numpunct : std::numpunct<char> {
protected:
    char do_thousands_sep() const override { return '\''; }
    std::string do_grouping() const override { return "\3"; }
};

std::string random_string(std::mt19937& gen,
                          std::uniform_int_distribution<>& len_dist,
                          std::uniform_int_distribution<>& char_dist) {
    int length = len_dist(gen);
    std::string s;
    s.reserve(length);
    for (int i = 0; i < length; ++i) {
        int r = char_dist(gen);
        if (r == 26) {
            s.push_back(' '); 
        } else {
            s.push_back(static_cast<char>('a' + r));
        }
    }
    return s;
}

int main() {
    std::ofstream ofs("gdata.csv");
    if (!ofs) {
        std::cerr << "Can't create gdata.csv\n";
        return 1;
    }

    ofs.imbue(std::locale(std::locale(), new apostrophe_numpunct));

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> len_dist(32, 64);
    std::uniform_int_distribution<> char_dist(0, 26);

    const int rows = 25'000'000;
    const int cols = 10;

    for (int i = 0; i < rows; ++i) {
        ofs << i;

        for (int j = 1; j < cols; ++j) {
            ofs << "," << random_string(gen, len_dist, char_dist);
        }
        ofs << "\n";
    }

    std::cout << "File gdata.csv: " << rows << " rows and " << cols << " cols.\n";

    return 0;
}