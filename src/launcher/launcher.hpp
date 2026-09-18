#pragma once

#include "src/data/result.hpp"
#include <string>

class Launcher
{
public:
    Launcher();

    bool launch(File& f) const;

private:
    std::string m_terminal_name;
};