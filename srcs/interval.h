#ifndef INTERVAL_H
#define INTERVAL_H

#include "utils.h"

class interval{
    public:
        double min, max;
        interval():min(+infinity), max(-infinity){}
        interval(double min, double max):min(min), max(max){}

        double size(){
            return max-min;
        }

        bool contains(double x){
            return x<=max && x>=min;
        }
        
        bool surronds(double x){
            return x<max && x>min;
        }

        static const interval empty, universe;

};
const interval interval::empty =interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif