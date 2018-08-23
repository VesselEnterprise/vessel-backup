#include <vessel/network/http_request.hpp>

using namespace Vessel::Networking;

void HttpRequest::set_url ( const std::string& str ) {
    m_url = str;
}

void HttpRequest::set_method( const std::string& str) {
    m_http_method = str;
}

void HttpRequest::add_header( const std::string& str) {
    m_headers.push_back( str );
}

void HttpRequest::set_content_type( const std::string& str) {
    m_content_type = str;
}

void HttpRequest::set_body( const std::string& str) {
    m_body = str;
}

void HttpRequest::set_auth_header(const std::string& str)
{
    m_authorization = str;
}

void HttpRequest::accept(const std::string& str)
{
    m_accept = str;
}

std::string HttpRequest::get_url() const
{
    return m_url;
}

std::string HttpRequest::get_method() const
{
    return m_http_method;
}

std::vector<std::string> HttpRequest::get_headers() const
{
    return m_headers;
}

std::string HttpRequest::get_content_type() const
{
    return m_content_type;
};

std::string HttpRequest::get_body() const
{
    return m_body;
}

std::string HttpRequest::get_accept() const
{
    return m_accept;
}

std::string HttpRequest::get_auth() const
{
    return m_authorization;
}


size_t HttpRequest::get_body_length() const
{
    return m_body.size();
}
