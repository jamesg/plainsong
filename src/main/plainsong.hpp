#ifndef PLAINSONG_MAIN_PLAINSONG_HPP
#define PLAINSONG_MAIN_PLAINSONG_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "../router.hpp"
#include "atlas/http/server/mimetypes.hpp"
#include "atlas/http/server/server.hpp"

namespace hades
{
    class connection;
}
namespace plainsong
{
    struct options {
        std::string address, db, port, audio;
        options() :
            address("0.0.0.0")
        {
        }
    };

    class server
    {
    public:
        server(const options&, boost::shared_ptr<boost::asio::io_service>);
        void start();
        void stop();
    private:
        boost::shared_ptr<boost::asio::io_service> m_io;
        boost::shared_ptr<hades::connection> m_connection;
        boost::shared_ptr<atlas::http::router> m_router;
        boost::shared_ptr<atlas::http::server> m_http_server;
        atlas::http::mimetypes m_mime_information;
    };
}

#endif
