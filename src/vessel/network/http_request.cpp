#include <vessel/network/http_request.hpp>

using namespace Vessel::Networking;

void HttpRequest::set_uri ( const std::string& str ) {
    m_uri = str;
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

std::string HttpRequest::get_uri() {
    return m_uri;
}

std::string HttpRequest::get_method() {
    return m_http_method;
}

std::vector<std::string> HttpRequest::get_headers() {
    return m_headers;
}

std::string HttpRequest::get_content_type() {
    return m_content_type;
};

std::string HttpRequest::get_body() {
    return m_body;
}

size_t HttpRequest::get_body_length() {
    return m_body.size();
}
