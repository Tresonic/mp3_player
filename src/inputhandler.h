#pragma once

enum class Buttonpress {None, Short, Long, Double};

namespace inputhandler {
void init();
int get_rot();
Buttonpress get_btn_a(); 
}

