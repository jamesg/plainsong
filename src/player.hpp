#ifndef PLAINSONG_PLAYER_HPP
#define PLAINSONG_PLAYER_HPP

#include <boost/filesystem.hpp>

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

namespace plainsong
{
    class player
    {
    public:
        enum state_t
        {
            PLAYING, PAUSED, STOPPED
        };

        // Number of volume levels (0 to VOLUME_LEVELS-1).
        static const constexpr int VOLUME_LEVELS = 14;

        static const constexpr int DEFAULT_VOLUME = 8;

        player();
        ~player();

        // Name of the file currently playing.
        std::string filename() const;

        // Load a file and start playing it from the beginning.
        void play_file(boost::filesystem::path);

        // Current location of the play head.
        int seconds() const;

        // Current volume on a scale of 0 to VOLUME_LEVELS-1.
        int volume() const;

        // Set the volume on a 14-point scale (volume should be 0-13).
        void set_volume(int volume);

        // Current player state.
        state_t state() const;

        // Stop and unload any file loaded.  Has no effect if no file has been
        // loaded.
        void stop();

        // Pause the currently playing file.  Has no effect if there is no file
        // playing.
        void pause();

        // Play the currently loaded file.  Has no effect if there is a file
        // already playing or no file has been loaded.
        void play();

    private:
        static void postmix(void *, Uint8 *, int);
        static void finished();

        int m_samples, m_volume;
        std::string m_filename;
        Mix_Music *m_music;
    };

    extern player g_player;
}

#endif
