#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    Q_INVOKABLE bool connectToDatabase();
    Q_INVOKABLE bool addNote(const QString &title, const QString &content, const QString &tagsString);
    Q_INVOKABLE QVariantList getNotes(const QString &searchText = "");
    Q_INVOKABLE QString getNoteTags(int noteId);
    Q_INVOKABLE bool updateNote(int id, const QString &title, const QString &content, const QString &tagsString);
    Q_INVOKABLE bool deleteNote(int id);

    // НАША НОВА ФУНКЦІЯ: Резервне копіювання
    Q_INVOKABLE bool backupDatabase();

private:
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
