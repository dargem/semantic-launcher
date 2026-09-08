#pragma once

#include <llama.h>
#include <string>
#include <vector>

class Embedder
{
public:
    explicit Embedder(const std::string&);

    // Embeds text based on the Embedder's model
    // Outputted floats are normalized
    std::vector<float> embed(const std::string_view text) const;

    // Get dimensionality of the embedder (is dependent on model)
    int get_dimensionality() const;

private:
    llama_model* m_model;
    llama_context* m_ctx;
};