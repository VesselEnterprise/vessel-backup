#include <iostream>
#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include "compress.hpp"

using namespace std;
using namespace Vessel::Compression;

int main()
{

    Compressor * c = new Compressor();
    c->set_z_level(Z_BEST_COMPRESSION);

    std::string filename = "test.txt";

    std::string str = "This is some test data This is some test data This is some test data This is some test data";

    ofstream outfile(filename, std::ofstream::out | std::ofstream::binary );

    outfile << str;

    outfile.close();

    c->compress_file(filename, filename+".gz");

    delete c;

    return 0;
}
