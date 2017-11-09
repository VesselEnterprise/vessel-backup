#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "compress.hpp"
#include "tarball.hpp"

using namespace std;
using namespace Backup::Compression;

int main()
{

    Compressor * c = new Compressor();

    /*

    ofstream outfile("test.txt", std::ofstream::out | std::ofstream::binary );

    int i=0;

    while ( i < 1000 )
    {

        string ts = "Hello hello hello hello hello hello";

        string tmp = *c << ts.c_str();

        string tmp2 = *c >> tmp.c_str();

        cout << "Compressed: " << tmp << endl;
        cout << "Decompressed: " << tmp2 << endl;

        outfile << tmp.c_str() << endl;

        i++;

    }

    outfile.close();

    ifstream infile("test.txt", std::ifstream::in | std::ifstream::binary );
    //infile.seekg(0, infile.beg );

    ofstream outfile2("output_test.txt", std::ofstream::out | std::ofstream::binary );

    std::string tmpLine;

    size_t fs = boost::filesystem::file_size("test.txt");

    while ( std::getline(infile, tmpLine) )
    {

        std::string ds = (*c >> tmpLine.c_str());

        cout << "Compressed (infile): " << tmpLine << endl;
        cout << "Decompressed (infile): " << ds << endl;

        outfile2 << ds << endl;
    }

    infile.close();
    outfile2.close();

    */

    c->set_z_level(Z_BEST_COMPRESSION);
    int i=0;
    while ( i < 10000 )
    {
        c->compress_file("C:\\Users\\Kyle\\Documents\\InsuranceIDCard.pdf", "C:\\Users\\Kyle\\Documents\\InsuranceIDCard.pdf.gz" );
        c->decompress_file("C:\\Users\\Kyle\\Documents\\InsuranceIDCard.pdf.gz", "C:\\Users\\Kyle\\Documents\\InsuranceIDCard_decompressed.pdf" );

        i++;
    }

    delete c;

    return 0;
}
