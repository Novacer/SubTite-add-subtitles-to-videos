#ifndef SUBTITLER_CLI_IO_INPUT_H
#define SUBTITLER_CLI_IO_INPUT_H

#include <iostream>
#include <stdexcept>
#include <string>

namespace subtitler {
namespace cli {
namespace io {

/**
 * InputGetter enables cross-platform Unicode support.
 * Unicode in Windows is being encoded as UTF-16, while in Unix it is UTF-8.
 * Consequently, for I/O to support Unicode in Windows we must use wcin & wcout.
 * All uses of std::string then needs to be replaced with std::wstring. Doing
 * that means keeping separate code bases for Windows and Unix.
 *
 * Instead, InputGetter allows input to be received as UTF-8 always, even if it
 * is originally UTF-16. So you can keep on using UTF-8 std::string on either
 * platform. Use NarrowInputGetter on Unix, WideInputGetter on Windows. See
 * below for details.
 *
 * If you choose to pass std::cin or std::wcin to an InputGetter, THEN DO NOT
 * USE [w]cin >> something and instead only use
 * input_getter->getline(something).
 */
class InputGetter {
  public:
    virtual bool getline(std::string &line) = 0;
    virtual ~InputGetter(){};
};

/**
 * Sample Usage - Unix:
 * auto input_getter = std::make_unique<NarrowInputGetter>(std::cin);
 * std::string input;
 * if (input_getter->getline(input)) { std::cout << input << std::endl; }
 */
class NarrowInputGetter : public InputGetter {
  public:
    NarrowInputGetter(std::istream &stream) : InputGetter{}, stream_{stream} {};

    bool getline(std::string &line) override {
        return static_cast<bool>(std::getline(stream_, line));
    };

  private:
    std::istream &stream_;
};

/**
 * Sample Usage - Windows:
 * // IMPORTANT: You must have the following line at the top of main!
 * _setmode(_fileno(stdin), _O_U16TEXT);
 * auto input_getter = std::make_unique<WideInputGetter>(std::wcin);
 * std::string input;
 * // Notice that you can still use std::cout even if you are using std::wcin.
 * // The input is converted to UTF-8 std::string automatically.
 * if (input_getter->getline(input)) { std::cout << input << std::endl; }
 */
class WideInputGetter : public InputGetter {
  public:
    // Note: trying to construct this class will throw an exception if you are
    // not using MSVC. Indeed, on Unix systems you shouldn't be using this
    // anyways.
    WideInputGetter(std::wistream &stream);

    bool getline(std::string &line) override;

  private:
    std::wistream &stream_;
};

}  // namespace io
}  // namespace cli
}  // namespace subtitler

#endif
