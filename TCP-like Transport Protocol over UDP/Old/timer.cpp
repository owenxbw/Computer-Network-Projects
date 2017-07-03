
#include <iostream>
#include <cstdio>
#include <ctime>

int main() {
    std::clock_t start;
    double duration;

    start= std::clock();
    //record current time

    /* Your algorithm here */
    while(1){
    duration = 1000*( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    if (duration>3){
    std::cout<<"The duration is: "<< duration <<" ms."<<'\n';
    break;
    }


  }
}