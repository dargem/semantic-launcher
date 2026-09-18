#pragma once

#include "src/data/result.hpp"
#include <string>

class Launcher
{
public:
    Launcher();

    void launch(File& f) const;

private:
    std::string m_terminal_name;
};