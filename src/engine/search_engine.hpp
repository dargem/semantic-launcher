#pragma once
#include "src/data/database.hpp"
#include "src/data/result.hpp"
#include <QAbstractListModel>
#include <QObject>

class SearchResultModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        NameRole = Qt::UserRole + 1,
        IconRole,
        ExecRole,
        ScoreRole,
        DescriptionRole
    };

    int rowCount(const QModelIndex& = QModelIndex()) const override
    {
        return m_results.size();
    }
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void set_results(const QList<Result>& results);

private:
    QList<Result> m_results;
};

class SearchEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SearchResultModel* results READ results CONSTANT)
public:
    explicit SearchEngine(QObject* parent = nullptr) : m_model(), m_database(Embedder("model.gguf")) {};

    Q_INVOKABLE void search(const QString& query); // gets called from QML on every keystroke
    Q_INVOKABLE void launch(int index);            // gets called on Enter / click

    SearchResultModel* results()
    {
        return &m_model;
    }

private:
    SearchResultModel m_model;
    Database m_database;
    // fuzzy matcher, ANN index, desktop-entry cache, etc
};