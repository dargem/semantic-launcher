#include "src/data/database.hpp"

using unum::usearch::index_dense_t;
using unum::usearch::metric_kind_t;
using unum::usearch::metric_punned_t;
using unum::usearch::scalar_kind_t;

// Initialize the database, should read from all sources to collate files
Database::Database()
    : m_vector_db(
          index_dense_t::make(metric_punned_t(EMBEDDING_DIMENSIONS, metric_kind_t::l2sq_k, scalar_kind_t::f64_k)))
{
}