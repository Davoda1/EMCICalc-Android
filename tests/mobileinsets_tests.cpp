// SPDX-License-Identifier: MIT
#include "mobileinsets.h"
#undef NDEBUG
#include <cassert>
#include <iostream>
int main(){
 assert(remainingMobileInsets({0,0,360,800},{0,24,360,752},{0,24,0,24})==QMargins());
 assert(remainingMobileInsets({0,0,360,800},{0,0,360,800},{0,24,0,24})==QMargins(0,24,0,24));
 assert(remainingMobileInsets({0,0,800,360},{32,0,744,360},{32,0,24,0})==QMargins());
 assert(remainingMobileInsets({0,0,800,360},{0,0,800,360},{32,0,24,0})==QMargins(32,0,24,0));
 assert(remainingMobileInsets({0,0,360,800},{0,24,360,752},{0,24,0,24},{0,500,360,300})==QMargins(0,0,0,276));
 assert(remainingMobileInsets({0,0,360,500},{0,24,360,476},{0,24,0,0},{0,500,360,300})==QMargins());
 assert(remainingMobileInsets({0,0,300,300},{0,0,300,300},{0,0,0,0})==QMargins());
 std::cout<<"PASS 7 inset geometry cases; host arithmetic tests, not Android runtime\n";
}
