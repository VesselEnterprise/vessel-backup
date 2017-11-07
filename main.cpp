#include <iostream>
#include "compress.hpp"

using namespace std;
using namespace Backup::Compression;

int main()
{
    Compressor * c = new Compressor();

    int i=0;

    while ( i < 100000 )
    {

        string tmp = c->compress("Hello hello hello hello hello hello");
        string tmp2 = c->decompress(tmp);

        cout << "Compressed: " << tmp << endl;
        cout << "Decompressed: " << tmp2 << endl;

        i++;

    }

    delete c;

    return 0;
}
