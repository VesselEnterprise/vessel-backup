#include <vessel/vessel/upload_manager.hpp>

UploadInterface::UploadInterface()
{
    std::cout << "Called the constructor!" << '\n';
    m_vessel = std::make_shared<VesselClient>(LocalDatabase::get_database().get_setting_str("master_server"));
}

std::shared_ptr<VesselClient> UploadInterface::get_vessel_client()
{
    return m_vessel;
}
