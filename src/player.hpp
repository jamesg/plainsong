#ifndef PLAINSONG_PLAYER_HPP
#define PLAINSONG_PLAYER_HPP

#include <deque>

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

        // Add a file, and all files beyond it alphabetically within the same
        // directory, to the playlist.
        void queue_file(boost::filesystem::path);

        // Skip to the next item in the playlist.  Throws an exception if the
        // playlist is empty.
        void queue_next();

        // Stop playing and empty the queue.
        void queue_stop();

        // Access the playlist.
        const std::deque<boost::filesystem::path>& queue() const;

        // Current location of the play head.
        int seconds() const;

        // Current volume on a scale of 0 to VOLUME_LEVELS-1.
        int volume() const;

        // Set the volume on a 14-point scale (volume should be 0-13).
        void set_volume(int volume);

        // Current player state.
        state_t state() const;

        // Pause the currently playing file.  Has no effect if there is no file
        // playing.
        void pause();

        // Play the currently loaded file.  Has no effect if there is a file
        // already playing or no file has been loaded.
        void play();

        // Skip forward ten seconds.
        void skip_forward();

        // Skip back ten seconds.
        void skip_back();

    private:
        static void postmix(void *, Uint8 *, int);
        static void finished();

        // Stop and unload any file loaded.  Has no effect if no file has been
        // loaded.
        void stop();

        int m_samples, m_volume;
        std::string m_filename;
        Mix_Music *m_music;
        std::deque<boost::filesystem::path> m_queue;
    };

    extern player g_player;
}

#endif
