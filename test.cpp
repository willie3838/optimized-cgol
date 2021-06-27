#include <iostream>
using namespace std;

int main(){
    unsigned char *testing = new unsigned char[1];
    int i = 3;

    while(i){
        *(testing) -= 0x100;
        cout << static_cast<unsigned>(*(testing)) << endl;
        i--;
    }
   

    return 0;
}