/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NcursesGraphics
*/

#include "NcursesGraphics.hpp"

#include <curses.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace client::gfx {

namespace {

constexpr const char *logPath = "r-type_client.log";
constexpr const char *systemTerminfo =
    "/etc/terminfo:/lib/terminfo:/usr/share/terminfo";

// Terminals send a key press, then repeat it after a delay (about 500 ms) at
// about 30 Hz while it is held. A first press is held until the repeats
// start; each repeat keeps it held a little longer.
constexpr auto FirstHold = std::chrono::milliseconds{550};
constexpr auto RepeatHold = std::chrono::milliseconds{100};
constexpr auto RepeatWindow = std::chrono::milliseconds{700};

constexpr auto FrameTime = std::chrono::microseconds{16'667}; // 60 Hz

constexpr int CtrlC = 3;
constexpr int EscapeDelayMs = 25;

struct Rgb {
    int r;
    int g;
    int b;
};

/** @brief The 8 basic terminal colors, in COLOR_BLACK..COLOR_WHITE order. */
constexpr std::array<Rgb, 8> BasicColors{{
    {.r = 0, .g = 0, .b = 0},
    {.r = 205, .g = 0, .b = 0},
    {.r = 0, .g = 205, .b = 0},
    {.r = 205, .g = 205, .b = 0},
    {.r = 0, .g = 0, .b = 238},
    {.r = 205, .g = 0, .b = 205},
    {.r = 0, .g = 205, .b = 205},
    {.r = 229, .g = 229, .b = 229},
}};

/** @brief Levels of the 6x6x6 color cube of 256-color terminals. */
constexpr std::array<int, 6> CubeLevels{0, 95, 135, 175, 215, 255};
constexpr int CubeStart = 16;
constexpr int GrayStart = 232;
constexpr int GrayCount = 24;
constexpr int GrayFirst = 8;
constexpr int GrayStep = 10;

int distance(Rgb a, Rgb b)
{
    const int dr = a.r - b.r;
    const int dg = a.g - b.g;
    const int db = a.b - b.b;
    return (dr * dr) + (dg * dg) + (db * db);
}

int nearestCubeLevel(int value)
{
    int best = 0;
    for (int i = 1; i < static_cast<int>(CubeLevels.size()); ++i) {
        if (std::abs(CubeLevels.at(i) - value) <
            std::abs(CubeLevels.at(best) - value)) {
            best = i;
        }
    }
    return best;
}

/** @brief The xterm 256-color index closest to color. */
short color256(Rgb color)
{
    const int r = nearestCubeLevel(color.r);
    const int g = nearestCubeLevel(color.g);
    const int b = nearestCubeLevel(color.b);
    const Rgb cube{
        .r = CubeLevels.at(r), .g = CubeLevels.at(g), .b = CubeLevels.at(b)};

    const int average = (color.r + color.g + color.b) / 3;
    const int step = std::clamp(
        (average - GrayFirst + (GrayStep / 2)) / GrayStep, 0, GrayCount - 1);
    const int level = GrayFirst + (step * GrayStep);
    const Rgb gray{.r = level, .g = level, .b = level};

    if (distance(color, gray) < distance(color, cube)) {
        return static_cast<short>(GrayStart + step);
    }
    return static_cast<short>(CubeStart + (36 * r) + (6 * g) + b);
}

/** @brief The basic terminal color closest to color. */
short color8(Rgb color)
{
    std::size_t best = 0;
    for (std::size_t i = 1; i < BasicColors.size(); ++i) {
        if (distance(color, BasicColors.at(i)) <
            distance(color, BasicColors.at(best))) {
            best = i;
        }
    }
    return static_cast<short>(best);
}

/** @brief The terminal color closest to color. */
short terminalColor(Color color)
{
    const Rgb rgb{.r = color.r, .g = color.g, .b = color.b};
    return COLORS >= 256 ? color256(rgb) : color8(rgb);
}

std::optional<Key> toKey(int character)
{
    switch (character) {
    case KEY_UP:
        return Key::Up;
    case KEY_DOWN:
        return Key::Down;
    case KEY_LEFT:
        return Key::Left;
    case KEY_RIGHT:
        return Key::Right;
    case ' ':
        return Key::Space;
    default:
        return std::nullopt;
    }
}

/** @brief The cells covered by [start, start + size) along one axis. */
struct CellSpan {
    int first = 0;
    int count = 0;
};

CellSpan cellSpan(float start, float size, float cell, int limit)
{
    auto first = static_cast<int>(std::floor(start / cell));
    auto end = static_cast<int>(std::ceil((start + size) / cell));
    end = std::max(end, first + 1); // anything visible covers a cell
    first = std::max(first, 0);
    end = std::min(end, limit);
    return {.first = first, .count = std::max(end - first, 0)};
}

} // namespace

std::unique_ptr<IGraphics> createGraphics(const WindowConfig &config)
{
    return std::make_unique<NcursesGraphics>(config);
}

NcursesGraphics::NcursesGraphics(const WindowConfig &config)
    : m_vsync(config.vsync)
{
    if (isatty(STDIN_FILENO) == 0 || isatty(STDOUT_FILENO) == 0) {
        throw std::runtime_error("the ncurses client must run in a terminal");
    }
    // The ncurses built by Conan looks for the terminal descriptions in its
    // own package folder: use the system's, unless the user chose some.
    if (std::getenv("TERMINFO") == nullptr &&
        std::getenv("TERMINFO_DIRS") == nullptr) {
        setenv("TERMINFO_DIRS", systemTerminfo, 0);
    }
    m_screen = newterm(nullptr, stdout, stdin);
    if (m_screen == nullptr) {
        throw std::runtime_error("cannot use this terminal (check TERM)");
    }
    raw(); // Ctrl+C arrives as a key, so the terminal is always restored
    noecho();
    keypad(stdscr, TRUE);
    // Arrows in the terminal's normal mode too, not only in keypad mode.
    define_key("\033[A", KEY_UP);
    define_key("\033[B", KEY_DOWN);
    define_key("\033[C", KEY_RIGHT);
    define_key("\033[D", KEY_LEFT);
    nodelay(stdscr, TRUE);
    curs_set(0);
    set_escdelay(EscapeDelayMs);
    if (has_colors()) {
        start_color();
        m_colors = true;
    }

    if (isatty(STDERR_FILENO) == 1) {
        std::clog.flush();
        // NOLINTBEGIN(cppcoreguidelines-owning-memory): closed right away
        std::FILE *log = std::fopen(logPath, "a");
        if (log != nullptr) {
            m_savedStderr = dup(STDERR_FILENO);
            dup2(fileno(log), STDERR_FILENO);
            static_cast<void>(std::fclose(log));
        }
        // NOLINTEND(cppcoreguidelines-owning-memory)
    }
}

NcursesGraphics::~NcursesGraphics()
{
    endwin();
    delscreen(m_screen);
    if (m_savedStderr >= 0) {
        std::clog.flush();
        dup2(m_savedStderr, STDERR_FILENO);
        close(m_savedStderr);
    }
}

bool NcursesGraphics::shouldClose()
{
    const Clock::time_point now = Clock::now();
    for (int character = getch(); character != ERR; character = getch()) {
        if (character == 'q' || character == 'Q' || character == CtrlC) {
            m_closeRequested = true;
        } else if (const std::optional<Key> key = toKey(character)) {
            press(*key, now);
        }
    }
    return m_closeRequested;
}

void NcursesGraphics::press(Key key, Clock::time_point now)
{
    const auto index = static_cast<std::size_t>(key);
    const bool repeat = now - m_lastPress.at(index) < RepeatWindow;
    const Clock::time_point until = now + (repeat ? RepeatHold : FirstHold);
    m_heldUntil.at(index) = std::max(m_heldUntil.at(index), until);
    m_lastPress.at(index) = now;
}

engine::Vec2f NcursesGraphics::screenSize() const
{
    int rows = 0;
    int columns = 0;
    getmaxyx(stdscr, rows, columns);
    return {.x = static_cast<float>(columns) * CellWidth,
            .y = static_cast<float>(rows) * CellHeight};
}

void NcursesGraphics::beginFrame(Color clear)
{
    m_zoom = 1.0F;
    if (m_colors) {
        m_clear = terminalColor(clear);
        bkgdset(' ' | COLOR_PAIR(colorPair(m_clear, m_clear)));
    }
    erase();
}

void NcursesGraphics::endFrame()
{
    refresh();
    if (!m_vsync) {
        return;
    }
    // Terminals have no V-Sync: wait instead, or the loop would use a whole
    // CPU core.
    const Clock::time_point now = Clock::now();
    m_nextFrame = std::max(m_nextFrame + FrameTime, now);
    std::this_thread::sleep_until(m_nextFrame);
}

void NcursesGraphics::beginWorld(float zoom)
{
    if (zoom > 0.0F) {
        m_zoom = zoom;
    }
}

void NcursesGraphics::endWorld() { m_zoom = 1.0F; }

Texture NcursesGraphics::loadTexture(const std::string & /*path*/)
{
    return {};
}

void NcursesGraphics::unloadTexture(Texture /*texture*/) {}

void NcursesGraphics::drawRectangle(Rect rect, Color color)
{
    if (rect.width <= 0.0F || rect.height <= 0.0F) {
        return;
    }
    int rows = 0;
    int columns = 0;
    getmaxyx(stdscr, rows, columns);
    const CellSpan x =
        cellSpan(rect.x * m_zoom, rect.width * m_zoom, CellWidth, columns);
    const CellSpan y =
        cellSpan(rect.y * m_zoom, rect.height * m_zoom, CellHeight, rows);

    chtype fill = '#';
    if (m_colors) {
        const short terminal = terminalColor(color);
        fill = ' ' | COLOR_PAIR(colorPair(terminal, terminal));
    }
    for (int row = y.first; row < y.first + y.count; ++row) {
        mvhline(row, x.first, fill, x.count);
    }
}

void NcursesGraphics::drawTexture(Texture /*texture*/, Rect /*source*/,
                                  Rect /*dest*/, Color /*tint*/)
{
}

void NcursesGraphics::drawText(const std::string &text, engine::Vec2f position,
                               int /*fontSize*/, Color color)
{
    int rows = 0;
    int columns = 0;
    getmaxyx(stdscr, rows, columns);
    const auto row =
        static_cast<int>(std::floor(position.y * m_zoom / CellHeight));
    const auto column =
        static_cast<int>(std::floor(position.x * m_zoom / CellWidth));
    if (row < 0 || row >= rows || column >= columns) {
        return;
    }
    // Cut what falls off the left and right edges.
    const auto skip = static_cast<std::size_t>(std::max(-column, 0));
    if (skip >= text.size()) {
        return;
    }
    const int start = std::max(column, 0);
    const int length =
        std::min(static_cast<int>(text.size() - skip), columns - start);

    if (m_colors) {
        attrset(COLOR_PAIR(colorPair(terminalColor(color), m_clear)));
    }
    mvaddnstr(row, start, std::string_view{text}.substr(skip).data(), length);
    attrset(A_NORMAL);
}

bool NcursesGraphics::isKeyDown(Key key) const
{
    return Clock::now() < m_heldUntil.at(static_cast<std::size_t>(key));
}

int NcursesGraphics::colorPair(short foreground, short background)
{
    const std::uint32_t key =
        (static_cast<std::uint32_t>(static_cast<std::uint16_t>(foreground))
         << 16U) |
        static_cast<std::uint16_t>(background);
    const auto it = m_pairs.find(key);
    if (it != m_pairs.end()) {
        return it->second;
    }
    const auto pair = static_cast<int>(m_pairs.size()) + 1;
    if (pair >= COLOR_PAIRS || pair > std::numeric_limits<short>::max()) {
        return 0;
    }
    init_pair(static_cast<short>(pair), foreground, background);
    m_pairs.emplace(key, pair);
    return pair;
}

} // namespace client::gfx
