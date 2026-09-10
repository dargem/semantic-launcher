#include "src/embedder/embedder.hpp"

#include <cmath>
#include <numeric>

Embedder::Embedder(const std::string& model_path)
{
    llama_backend_init();

    llama_model_params mparams = llama_model_default_params();
    m_model = llama_model_load_from_file(model_path.c_str(), mparams);

    llama_context_params cparams = llama_context_default_params();
    cparams.embeddings = true;
    cparams.n_ubatch = cparams.n_batch = 512;
    m_ctx = llama_init_from_model(m_model, cparams);
}

int Embedder::get_dimensionality() const
{
    return llama_model_n_embd(m_model);
}

std::vector<float> Embedder::embed(const std::string_view text) const
{
    // C style string conversion as string_view isn't null terminated
    std::string input(text);

    std::vector<llama_token> tokens(input.size() + 8);
    int n = llama_tokenize(
        llama_model_get_vocab(m_model), input.c_str(), input.size(), tokens.data(), tokens.size(), true, false);
    tokens.resize(n);

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_encode(m_ctx, batch);

    const float* emb = llama_get_embeddings_seq(m_ctx, 0);
    int dim = llama_model_n_embd(m_model);
    std::vector<float> result(emb, emb + dim);

    // normalize for cosine similarity
    float norm = std::sqrt(std::inner_product(result.begin(), result.end(), result.begin(), 0.0f));
    for (auto& v : result)
        v /= norm;
    return result;
}