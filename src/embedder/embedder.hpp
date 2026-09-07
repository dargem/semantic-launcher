#pragma once

#include <cmath>
#include <numeric>
#include <string>
#include <vector>
#include <llama.h>

class Embedder {
public:
    explicit Embedder(const std::string&);

    // Embeds text based on the Embedder's model
    // Outputted floats are normalized
    std::vector<float> embed(const std::string &text);

private:
    llama_model* m_model;
    llama_context* m_ctx;
};