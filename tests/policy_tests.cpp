#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

// Mirrors the assignment's agreed policy: deduct 10% of the maximum mark
// for each late day (up to 3 days); more than 3 late days receives zero.
double calculateFinalMark(double rawMark, double maxMark, int lateDays) {
    if (lateDays > 3) return 0.0;
    return std::min(maxMark, std::max(0.0, rawMark - (0.10 * maxMark * std::max(0, lateDays))));
}
static bool near(double a, double b) { return std::fabs(a-b) < 0.001; }
int main() {
    assert(near(calculateFinalMark(80,100,0),80));
    assert(near(calculateFinalMark(80,100,1),70));
    assert(near(calculateFinalMark(80,100,2),60));
    assert(near(calculateFinalMark(80,100,3),50));
    assert(near(calculateFinalMark(80,100,4),0));
    assert(near(calculateFinalMark(5,100,1),0));
    assert(near(calculateFinalMark(120,100,0),100));
    assert(near(calculateFinalMark(80,100,-1),80));
    std::cout << "Policy tests passed: 8 assertions\\n";
}
