// inspired by video
// This is C++: Uncompromised Performance, Undefined Behavior, & Move Semantics - Jon Kalb C++Now 2024
// How to describe that the value is not initialized explicitly: modern legal way:
// int a = a;

// in this context, b is initilized as 0
int b = b;

int main(int, const char **)
{
    // but here, if I do
    // int a = a; - it's not initialized
    int a = (a=3, a + 1);
    return b + a;
}
