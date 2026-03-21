#include "sharedTypes.hpp"
#include "terminal.hpp"

#include <cstdint>
#include <errno.h>
#include <signal.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

volatile sig_atomic_t Terminal::signalFlag = 0;

Terminal::Terminal()
{
    enableRawMode();
    setupSignalHandler();
    write("\x1b[?1049h");
}

Terminal::~Terminal()
{
    disableRawMode();
    write("\x1b[?1049l");
}

Status Terminal::enableRawMode()
{
    if(tcgetattr(STDIN_FILENO, &original_) == -1) return AttributeGettingError;
    
    struct termios raw_ = original_;
    raw_.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw_.c_oflag &= ~(OPOST);
    raw_.c_cflag |= (CS8);
    raw_.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw_.c_cc[VMIN] = 0;
    raw_.c_cc[VTIME] = 2;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_) == -1) return AttributeSettingError;
    return Success;
}

Status Terminal::disableRawMode()
{
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_) == -1) return AttributeSettingError;
    return Success;
}

Status Terminal::read(void* storage)
{
    int read_status = ::read(STDIN_FILENO, storage, 1);
    
    switch(read_status)
    {
        case 0:
            return NoInput;
        case -1:
            if(errno == EINTR) return SignalInterrupt;
            else return ReadingError;
        default:
            return Success;
    }
}

Status Terminal::write(const std::string_view input)
{
    int write_status = ::write(STDOUT_FILENO, input.data(), input.size());
    
    switch(write_status)
    {
        case -1:
            return WritingError;
        default:
            return Success;
    }
}

void Terminal::setupSignalHandler()
{
    struct sigaction sa{};
    sa.sa_handler = Terminal::handleInterrupt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGWINCH, &sa, nullptr) == -1)
        perror("sigaction");
}

void Terminal::handleInterrupt(int sig)
{
    switch(sig)
    {
        default:
        case SIGWINCH:
            signalFlag |= WinResize;
            break;
    }
}

bool Terminal::checkFlag(Interrupt flag)
{
    return (signalFlag & (flag));
}

void Terminal::clearFlag()
{
    signalFlag = StandBy;
}

WinSize Terminal::getWindowSize()
{
    struct winsize ws;
    WinSize winsize;
    
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    winsize.rows = ws.ws_row;
    winsize.cols = ws.ws_col;
    
    return winsize;
}
