#include "router.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

#include "atlas/db/date.hpp"
#include "atlas/http/server/exception.hpp"
#include "atlas/http/server/static_string.hpp"
#include "hades/crud.ipp"
#include "hades/custom_select.hpp"
#include "hades/exists.hpp"
#include "hades/join.hpp"

#include "main/plainsong.hpp"
#include "player.hpp"

#define PLAINSONG_DECLARE_STATIC_STRING(NAME) ATLAS_DECLARE_STATIC_STRING(plainsong, NAME)
#define PLAINSONG_STATIC_STD_STRING(NAME) ATLAS_STATIC_STD_STRING(plainsong, NAME)

PLAINSONG_DECLARE_STATIC_STRING(index_html)
PLAINSONG_DECLARE_STATIC_STRING(index_js)

plainsong::router::router(
    const options& opts,
    boost::shared_ptr<boost::asio::io_service> io
    ) :
    atlas::http::application_router(io)
{
    boost::filesystem::path audio_root(opts.audio);

    install_static_text("/", "html", PLAINSONG_STATIC_STD_STRING(index_html));

    install_static_text("/index.html", PLAINSONG_STATIC_STD_STRING(index_html));
    install_static_text("/index.js", PLAINSONG_STATIC_STD_STRING(index_js));

    auto state = []() {
        styx::object out;
        out["title"] = g_player.filename();
        out.get_int("volume") = g_player.volume();

        player::state_t state = g_player.state();
        switch(state) {
            case player::STOPPED:
            out["state"] = std::string("stopped");
            break;
            case player::PAUSED:
            out["state"] = std::string("paused");
            break;
            case player::PLAYING:
            out["state"] = std::string("playing");
            break;
        }

        out.get_int("time") = g_player.seconds();
        return out;
    };

    //
    // Player state.
    //

    install<>(
        atlas::http::matcher("/state", "GET"),
        [state]() {
            return atlas::http::json_response(state());
        }
    );

    //
    // Update status (playing, paused, volume etc.).
    //

    install_json<styx::object>(
        atlas::http::matcher("/state", "PUT"),
        [state](styx::object s) {
            // Player state.
            if(s.get_string("state") == "playing")
                g_player.play();
            if(s.get_string("state") == "paused")
                g_player.pause();
            if(s.get_string("state") == "stopped")
                g_player.queue_stop();

            // Volume.
            g_player.set_volume(s.get_int("volume"));

            return atlas::http::json_response(state());
        }
    );

    // Start playing a file.  Return the new state.
    install<std::string>(
        atlas::http::matcher("/play/(.*)", "GET"),
        [audio_root, state](std::string path) {
            g_player.queue_file(
                (audio_root / boost::filesystem::path(path)).string().c_str()
            );
            return atlas::http::json_response(state());
        }
    );

    // Skip to the next file in the playlist.  Return the new state.
    install<>(
        atlas::http::matcher("/next", "GET"),
        [state]() {
            g_player.queue_next();
            return atlas::http::json_response(state());
        }
    );

    // Skip forward ten seconds.
    install<>(
        atlas::http::matcher("/forward", "GET"),
        [state]() {
            g_player.skip_forward();
            return atlas::http::json_response(state());
        }
    );

    // Skip back ten seconds.
    install<>(
        atlas::http::matcher("/back", "GET"),
        [state]() {
            g_player.skip_back();
            return atlas::http::json_response(state());
        }
    );

    //
    // File list.
    //

    install<std::string>(
        atlas::http::matcher("/browse/(.*)", "GET"),
        [audio_root](std::string path) {
            styx::object out;

            boost::filesystem::path fragment(path);
            fragment.normalize();
            atlas::log::information("plainsong::router") << "browse to " <<
                fragment.string();
            boost::filesystem::path dir(audio_root / fragment);
            dir.normalize();
            out["path"] = fragment.string();

            for(
                boost::filesystem::directory_iterator i(dir);
                i != boost::filesystem::directory_iterator();
                ++i
                )
            {
                styx::object file;
                file["name"] = i->path().leaf().string();
                file["path"] = (fragment / i->path().leaf()).string();
                if(boost::filesystem::is_directory(i->path()))
                    file["type"] = std::string("dir");
                else
                    file["type"] = std::string("file");
                out.get_list("files").append(file);
            }

            return atlas::http::json_response(out);
        }
    );
}
