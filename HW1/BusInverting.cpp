#include <algorithm>
#include <cmath>
#include <csetjmp>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
typedef unsigned long usize;
void usage(char *prg) {
    std::cout << "Usage : " << prg << " <numbits> " << std::endl;
}

const usize ITER = 500000;
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<usize> ud(0);

class runtime_bitset {
    std::vector<usize> m_data;

  public:
    usize sz;
    runtime_bitset(const usize sz) : sz(sz), m_data(sz / sizeof(usize) + 1) {}
    inline void randomize() {
        int i = 0;
        for (; i < sz / sizeof(usize); ++i) {
            m_data[i] = ud(gen);
        }
        if (i < m_data.size())
            m_data[i] = ud(gen) % (1 << (sz - i * sizeof(usize)));
    }
    inline bool get(usize i) const {
        if (i < sz)
            return (bool)(m_data[i / sizeof(usize)] &
                          (1 << (i % sizeof(usize))));
        return false;
    }
    inline void set(usize i, bool val) {
        if (i >= sz)
            return;
        usize xval = val << (i % sizeof(usize));
        usize bit = 1 << (i % sizeof(usize));
        usize cval = m_data[i / sizeof(usize)];
        cval = (cval & ~bit) | xval;
        m_data[i / sizeof(usize)] = cval;
    }
};
usize normal_activity(const runtime_bitset &bs) {
    usize res = 0;
    for (usize i = 0; i < bs.sz; i += 1) {
        res += bs.get(i);
    }
    return res;
}
usize businv_activity(const runtime_bitset &bs, const usize split_size) {
    usize res = 0;
    for (usize i = 0; i < bs.sz; i += split_size) {
        usize cnt = 0;
        for (usize j = i; j < i + split_size && j < bs.sz; j++) {
            cnt += bs.get(j);
        }
        res += std::min(cnt, split_size + 1 - cnt);
    }
    return res;
}
std::pair<float, float> avg_activity(const usize numbits,
                                     const usize split_size) {
    runtime_bitset bs(numbits);
    float binv_activity = 0;
    float norm_activity = 0;
    for (int i = 0; i < ITER; i++) {
        bs.randomize();
        binv_activity += businv_activity(bs, split_size);
        norm_activity += normal_activity(bs);
    }
    return {norm_activity / ITER, binv_activity / ITER};
}
int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }
    usize numbits;
    try {
        numbits = std::stoul(std::string(argv[1]));
    } catch (std::exception) {
        usage(argv[0]);
        exit(1);
    }

    std::cout << "Split\t" << "AvgNormActivity\t"
              << "AvgBinvActivity\t" << "Improvement" << std::endl;
    for (usize t = 1; t <= numbits; t++) {
        auto x = avg_activity(numbits, t);
        std::cout << t << '\t' << x.first << '\t' << x.second << '\t'
                  << 1 - (x.second / x.first) << std::endl;
    }
}
