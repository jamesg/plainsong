#include "plainsong.hpp"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include "atlas/db/auth.hpp"
#include "atlas/http/server/static_files.hpp"
#include "atlas/log/log.hpp"
#include "atlas/task/poll.hpp"
#include "commandline/commandline.hpp"
#include "hades/connection.hpp"

#include "../db.hpp"
#include "../router.hpp"

plainsong::server::server(
    const options& opts,
    boost::shared_ptr<boost::asio::io_service> io
    ) :
    m_io(io)
{
    // if(!opts.db.length())
        // throw std::runtime_error("database file is required");
    if(!opts.port.length())
        throw std::runtime_error("port number is required");

    // m_connection.reset(new hades::connection(opts.db));
    // atlas::db::auth::create(*m_connection);
    // db::create(*m_connection);
    m_router.reset(new plainsong::router(opts, io));
    m_http_server.reset(
        new atlas::http::server(m_io, opts.address.c_str(), opts.port.c_str())
    );
    boost::shared_ptr<atlas::http::router> atlas_router(
        atlas::http::static_files(io)
    );
    // Forward requests starting with '/atlas' to the Atlas static file router.
    m_http_server->router().install(
        atlas::http::matcher("/atlas(.*)"),
        boost::bind(&atlas::http::router::serve, atlas_router, _1, _2, _3, _4)
    );
    // Forward all other requests to the application router.  The priority
    // argument (1) gives this handler a lower priority than the /atlas
    // handler.
    m_http_server->router().install(
        atlas::http::matcher("(.*)", 1),
        boost::bind(&atlas::http::router::serve, m_router, _1, _2, _3, _4)
    );
}

void plainsong::server::start()
{
    atlas::log::information("plainsong::server::start") << "starting server";
    if(m_http_server)
        m_http_server->start();
}

void plainsong::server::stop()
{
    m_http_server->stop();
}

int main(const int argc, const char *argv[])
{
    bool show_help = false;
    plainsong::options opts;
    std::vector<commandline::option> options{
        commandline::parameter("db", opts.db, "Database file path"),
        commandline::parameter("address", opts.address, "Server IP address"),
        commandline::parameter("port", opts.port, "Server port"),
        commandline::parameter("audio", opts.audio, "Audio path"),
        commandline::flag("help", show_help, "Show a help message")
    };
    commandline::parse(argc, argv, options);

    if(show_help)
    {
        commandline::print(argc, argv, options);
        return 0;
    }

    try
    {
        boost::shared_ptr<boost::asio::io_service> io =
                boost::make_shared<boost::asio::io_service>();
        plainsong::server server(opts, io);
        server.start();
        boost::asio::io_service::work work(*io);
        io->run();
        server.stop();
        return 0;
    }
    catch(const std::exception& e)
    {
        atlas::log::information("main") <<
            "plainsong server closed with exception: " << e.what();
        return 1;
    }
}
