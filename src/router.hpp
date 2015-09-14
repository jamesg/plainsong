#ifndef PLAINSONG_ROUTER_HPP
#define PLAINSONG_ROUTER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "atlas/http/server/application_router.hpp"

namespace hades
{
    class connection;
}
namespace plainsong
{
    class options;

    class router : public atlas::http::application_router
    {
    public:
        router(
            const options&,
            boost::shared_ptr<boost::asio::io_service>
            );
    };
}

#endif
