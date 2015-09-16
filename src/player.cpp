#include "player.hpp"

#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "atlas/log/log.hpp"
#include "hades/mkstr.hpp"

namespace
{
    int volume_level[] = {
        0, 5, 10, 17, 25, 34, 45, 55, 65, 76, 88, 100, 112, 128
    };

    int SAMPLERATE = 44100;
}

plainsong::player plainsong::g_player;

plainsong::player::player() :
    m_samples(0),
    m_volume(DEFAULT_VOLUME),
    m_music(nullptr)
{
    atlas::log::information("plainsong::player::player") << "construct player";
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
        throw std::runtime_error("initialising SDL");

    int flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if(Mix_Init(flags) & flags != flags)
        throw std::runtime_error("initialising SDL Mixer");

    Mix_SetPostMix(player::postmix, nullptr);
    if(Mix_OpenAudio(SAMPLERATE, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
        throw std::runtime_error("opening audio device");

    set_volume(DEFAULT_VOLUME);
}

plainsong::player::~player()
{
    while(Mix_Init(0))
        Mix_Quit();
    SDL_Quit();
}

void plainsong::player::postmix(void *, Uint8 *, int length)
{
    if(Mix_PlayingMusic() && !Mix_PausedMusic())
        g_player.m_samples += length;
}

void plainsong::player::finished()
{
    g_player.stop();
    if(g_player.queue().size())
        g_player.queue_next();
}

std::string plainsong::player::filename() const
{
    return m_filename;
}

void plainsong::player::play_file(boost::filesystem::path file)
{
    atlas::log::information("plainsong::player::play_file") << "playing " <<
        file.string().c_str();

    stop();

    m_music = Mix_LoadMUS(file.string().c_str());
    if(!m_music)
        throw std::runtime_error("loading audio");

    if(Mix_PlayMusic(m_music, 1) == -1)
        throw std::runtime_error(
            hades::mkstr() << "playing audio (" << Mix_GetError() << ")"
        );

    m_filename = file.leaf().string();
}

void plainsong::player::queue_file(boost::filesystem::path file)
{
    stop();
    auto dir = file.parent_path();
    m_queue.clear();
    std::copy_if(
        boost::filesystem::directory_iterator(dir),
        boost::filesystem::directory_iterator(),
        std::inserter(m_queue, m_queue.end()),
        [file](const boost::filesystem::path& p) {
            return p.leaf().string() >= file.leaf().string();
        }
    );
    std::sort(m_queue.begin(), m_queue.end());
    for(boost::filesystem::path p : queue())
        atlas::log::test("plainsong::player::queue_file") << "file " << p.string();
    queue_next();
}

void plainsong::player::queue_next()
{
    if(m_queue.empty())
        throw std::runtime_error("the queue is empty");

    play_file(m_queue.front());
    m_queue.pop_front();
}

void plainsong::player::queue_stop()
{
    stop();
    m_queue.clear();
}

const std::deque<boost::filesystem::path>& plainsong::player::queue() const
{
    return m_queue;
}

int plainsong::player::seconds() const
{
    return (m_samples / SAMPLERATE) / 4;
}

int plainsong::player::volume() const
{
    return m_volume;
}

void plainsong::player::set_volume(int volume)
{
    if(volume >= 0 && volume < VOLUME_LEVELS)
    {
        m_volume = volume;
        Mix_VolumeMusic(volume_level[volume]);
    }
}

plainsong::player::state_t plainsong::player::state() const
{
    if(m_music == nullptr)
        return player::STOPPED;
    if(Mix_PausedMusic())
        return player::PAUSED;
    return player::PLAYING;
}

void plainsong::player::stop()
{
    Mix_HaltMusic();
    if(m_music)
    {
        atlas::log::information("plainsong::player::play_file") <<
            "freeing existing file";
        Mix_FreeMusic(m_music);
        m_music = nullptr;
        m_samples = 0;
        m_filename = "";
    }
}

void plainsong::player::pause()
{
    Mix_PauseMusic();
}

void plainsong::player::play()
{
    Mix_ResumeMusic();
}

void plainsong::player::skip_forward()
{
    // Skip forward ten seconds.
    Mix_RewindMusic();
    m_samples = (seconds() + 10) * SAMPLERATE * 4;
    Mix_SetMusicPosition(seconds());

    if(!Mix_PlayingMusic())
        player::finished();
}

void plainsong::player::skip_back()
{
    // Skip back ten seconds (or to the start of the track).
    Mix_RewindMusic();
    m_samples = (seconds() < 10) ? 0 : (seconds() - 10) * SAMPLERATE * 4;
    Mix_SetMusicPosition(seconds());
}
