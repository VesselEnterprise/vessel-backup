#include <string>
#include <boost/filesystem.hpp>
#include "tarball.hpp"

using namespace std;
using namespace Vessel::Compression;

int main()
{

    //Create some test txt files
    ofstream outfile1("test1.txt", std::ofstream::out | std::ofstream::binary );
    ofstream outfile2("test2.txt", std::ofstream::out | std::ofstream::binary );

    std::string str = "This is just a test";

    for ( int i=0; i < 5; i++ )
        outfile1 << str << std::endl;

    outfile1.close();

    for ( int i=0; i < 5; i++ )
        outfile2 << str << std::endl;

    outfile2.close();

    Compressor * c = new Compressor();
    c->set_z_level(Z_BEST_COMPRESSION);

    //Test tarball functionality
    Tarball* t = new Tarball("test.tar.gz");

    t->add_file("test1.txt");
    t->add_file("test2.txt");

    t->save_tar();

    delete t;
    delete c;

}
