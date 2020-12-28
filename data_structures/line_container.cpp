#include <limits>
#include <functional>
#include <set>
#include <deque>
#include <vector>
#include <algorithm>
template<typename T>
struct line {
    T a, b;
    bool q;
    mutable T l;
    T eval(T x) const { return a * x + b; }

    bool operator < (const line& o) const {
        return (q || o.q) ? l < o.l : a < o.a;
    }

    static constexpr T inf = std::numeric_limits<T>::has_infinity ?
        std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max();

    template<typename _T = T> static
    typename std::enable_if<std::is_integral<_T>::value, _T>::type div(const _T a, const _T b) {
        return a / b - ((a ^ b) < 0 && (a % b));
    }
    template<typename _T = T> static
    typename std::enable_if<!std::is_integral<_T>::value, _T>::type div(const _T a, const _T b) {
        return a / b;
    }

    T boundary(const line<T>& o) const {
        if (a == o.a) return b > o.b ? inf : -inf;
        return div(o.b - b, a - o.a);
    }
};

/*
 * insert_line(a, b): inserts the line f(x) = a * x + b
 * maximum(x0): returns the maximum value at x0 among all inserted lines
 */
template<typename T>
struct line_container : std::multiset<line<T>> {
    using Iterator = typename std::multiset<line<T>>::iterator;

    bool set_boundary(Iterator x, Iterator y) {
        if (y == this->end()) { x->l = line<T>::inf; return false; }
        return (x->l = x->boundary(*y)) >= y->l;
    }

    bool covered(Iterator y) {
        return y != this->begin() && set_boundary(prev(y), y);
    }

    // amortized log(container size)
    void insert_line(T a, T b) {
        auto it = this->insert({ a, b, false });
        while (set_boundary(it, next(it))) this->erase(next(it));
        if (covered(it)) set_boundary(prev(it), it = this->erase(it));
        while (it != this->begin() && covered(prev(it))) {
            this->erase(prev(it));
            set_boundary(prev(it), it);
        }
    }

    // worst case log(container size)
    T maximum(T x0) {
        assert(!this->empty());
        return this->lower_bound({ 0, 0, true, x0 })->eval(x0);
    }
};

/*
 * insert_line(a, b): inserts the line f(x) = a * x + b
 *                    the value of a must be non-decreasing across calls
 *
 * maximum(x0): returns the maximum value at x0 among all inserted lines
 *              the value of x0 must be non-decreasing across calls
 */
template<typename T>
struct line_container_monotonic : std::deque<line<T>> {
    // amortized O(1)
    void insert_line(T a, T b) {
        line<T> ins = { a, b, false, line<T>::inf };
        if (!this->empty()) {
            auto bk = this->rbegin();
            if (bk->a == a && bk->b >= b) return;
            bk->l = bk->boundary(ins);
            while (this->size() >= 2 && next(bk)->l >= bk->l) {
                this->pop_back(), bk++;
                bk->l = bk->boundary(ins);
            }
        }
        this->push_back(ins);
    }

    // amortized O(1)
    T maximum(T x0) {
        assert(!this->empty());
        while (this->front().l < x0) this->pop_front();
        return this->front().eval(x0);
    }
};

/*
 * Better performance constant for when all lines are inserted before all queries.
 */
template<typename T>
struct line_container_static {
    std::vector<line<T>> hull;

    line_container_static() {}

    template<typename InputIterator>
    line_container_static(InputIterator begin, InputIterator end) {
        std::vector<line<T>> sorted_input(begin, end);
        std::sort(sorted_input.begin(), sorted_input.end());
        for (line<T> elt : sorted_input) {
            while (hull.size() >= 2 && (hull.end() - 2)->l >= (hull.end() - 2)->boundary(elt))
                hull.pop_back();
            if (hull.size()) hull.back().l = hull.back().boundary(elt);
            hull.push_back({ elt.a, elt.b, false, line<T>::inf });
        }
    }

    // binary search in log(container size)
    T maximum(T x0) const {
        return std::lower_bound(hull.begin(), hull.end(), line<T>({ 0, 0, true, x0 }))->eval(x0);
    }
};
