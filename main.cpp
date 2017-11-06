#include <iostream>
#include "compress.hpp"

using namespace std;
using namespace Backup::Compression;

int main()
{
    Compressor * c = new Compressor();

    string tmp = c->deflate_s("Hello hello hello hello hello hello");

    cout << "Compressed: " << tmp << endl;

    return 0;
}
