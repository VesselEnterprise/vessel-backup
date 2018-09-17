#include <vessel/azure/azure_client.hpp>

AzureClient::AzureClient(const StorageProvider& provider) : HttpClient(provider.server), m_storage_provider(provider)
{
    m_xms_version = "2018-03-28"; //Default
    m_xms_date = get_ms_date();
}

AzureClient::~AzureClient()
{

}

std::string AzureClient::get_ms_date()
{

    std::stringstream ss;
    boost::posix_time::ptime pt( boost::posix_time::second_clock::universal_time() );
    boost::posix_time::time_facet* input_facet = new boost::posix_time::time_facet(); //No memory leak here
    input_facet->format("%a, %d %b %Y %T GMT");
    ss.imbue( std::locale(ss.getloc(), input_facet ) );
    ss << pt;

    return ss.str();

}
