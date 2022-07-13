#include "bits.h"
#include <math.h>

Bitboard trace_ray(Square origin, Direction d)
{
    Bitboard ray = 0ULL;
    for(Square s=origin + d; in_bounds(s,d); s = s + d) {
        ray |= bit(s);  
    }
    return ray;
}

// For debugging
void print(Bitboard bb) 
{
    for(int y=7; y >=0; --y){
        std::cout << '\n';
        for(int x=0; x < 8; ++x)
            std::cout << get_bit(bb, x,y) << " ";
    }
    std::cout << '\n';
}

void print16(BMove m)
{
    for(size_t i=0; i < 16; ++i)
        std::cout << ( (m >> i) & 1ULL );
    std::cout << '\n';
}

void fit_power(double &a, double &b, std::vector<size_t> ys)
{
    double sumx = 0, sumx2, sumy = 0, sumxy = 0;
    size_t n = ys.size();
    for(size_t i = 0; i < n; ++i) {
        double x = i + 1;
        double y = ys[i] + 1;
        sumx += log(x);
        sumx2 += log(x) * log(x);
        sumy += log(y);
        sumxy += log(x) * log(y);
    }
    
    b = ( (n * sumxy) - (sumx * sumy) ) / ( (n * sumx2) - (sumx * sumx));
    a = exp( (sumy - b * sumx) / n );
}
