#include "databasemanager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>      // Додали для копіювання файлу
#include <QDateTime>  // Додали для створення унікального імені копії

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
}

bool DatabaseManager::connectToDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/User/Desktop/Coursework/notes.db");

    if (!db.open()) {
        qDebug() << "Помилка підключення до БД:" << db.lastError().text();
        return false;
    } else {
        qDebug() << "Успіх! База даних підключена!";
        return true;
    }
}

bool DatabaseManager::addNote(const QString &title, const QString &content, const QString &tagsString)
{
    if (title.isEmpty()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO notes (title, content) VALUES (:title, :content)");
    query.bindValue(":title", title);
    query.bindValue(":content", content);

    if (!query.exec()) return false;

    qlonglong noteId = query.lastInsertId().toLongLong();

    QStringList tags = tagsString.split(",", Qt::SkipEmptyParts);
    for (QString tag : tags) {
        tag = tag.trimmed();
        if (tag.isEmpty()) continue;

        QSqlQuery tagQuery;
        tagQuery.prepare("INSERT OR IGNORE INTO tags (name) VALUES (:name)");
        tagQuery.bindValue(":name", tag);
        tagQuery.exec();

        tagQuery.prepare("SELECT id FROM tags WHERE name = :name");
        tagQuery.bindValue(":name", tag);
        if (tagQuery.exec() && tagQuery.next()) {
            qlonglong tagId = tagQuery.value(0).toLongLong();

            QSqlQuery linkQuery;
            linkQuery.prepare("INSERT OR IGNORE INTO note_tags (note_id, tag_id) VALUES (:note_id, :tag_id)");
            linkQuery.bindValue(":note_id", noteId);
            linkQuery.bindValue(":tag_id", tagId);
            linkQuery.exec();
        }
    }
    return true;
}

QVariantList DatabaseManager::getNotes(const QString &searchText)
{
    QVariantList list;
    QSqlQuery query;

    if (searchText.isEmpty()) {
        query.prepare("SELECT id, title, content, created_at FROM notes ORDER BY updated_at DESC");
    } else {
        query.prepare("SELECT DISTINCT n.id, n.title, n.content, n.created_at FROM notes n "
                      "LEFT JOIN note_tags nt ON n.id = nt.note_id "
                      "LEFT JOIN tags t ON nt.tag_id = t.id "
                      "WHERE n.title LIKE :search OR n.content LIKE :search OR t.name LIKE :search "
                      "ORDER BY n.updated_at DESC");
        query.bindValue(":search", "%" + searchText + "%");
    }

    if (query.exec()) {
        while (query.next()) {
            QVariantMap note;
            note["id"] = query.value("id");
            note["title"] = query.value("title");
            note["content"] = query.value("content");
            QString rawDate = query.value("created_at").toString();
            note["date"] = rawDate.replace("T", " ").left(16);
            list.append(note);
        }
    }
    return list;
}

QString DatabaseManager::getNoteTags(int noteId)
{
    QSqlQuery query;
    query.prepare("SELECT t.name FROM tags t "
                  "JOIN note_tags nt ON t.id = nt.tag_id "
                  "WHERE nt.note_id = :note_id");
    query.bindValue(":note_id", noteId);

    QStringList tags;
    if (query.exec()) {
        while (query.next()) {
            tags.append(query.value(0).toString());
        }
    }
    return tags.join(", ");
}

bool DatabaseManager::updateNote(int id, const QString &title, const QString &content, const QString &tagsString)
{
    if (title.isEmpty()) return false;

    QSqlQuery query;
    query.prepare("UPDATE notes SET title = :title, content = :content, updated_at = CURRENT_TIMESTAMP WHERE id = :id");
    query.bindValue(":title", title);
    query.bindValue(":content", content);
    query.bindValue(":id", id);

    if (!query.exec()) return false;

    QSqlQuery deleteLinks;
    deleteLinks.prepare("DELETE FROM note_tags WHERE note_id = :note_id");
    deleteLinks.bindValue(":note_id", id);
    deleteLinks.exec();

    QStringList tags = tagsString.split(",", Qt::SkipEmptyParts);
    for (QString tag : tags) {
        tag = tag.trimmed();
        if (tag.isEmpty()) continue;

        QSqlQuery tagQuery;
        tagQuery.prepare("INSERT OR IGNORE INTO tags (name) VALUES (:name)");
        tagQuery.bindValue(":name", tag);
        tagQuery.exec();

        tagQuery.prepare("SELECT id FROM tags WHERE name = :name");
        tagQuery.bindValue(":name", tag);
        if (tagQuery.exec() && tagQuery.next()) {
            qlonglong tagId = tagQuery.value(0).toLongLong();

            QSqlQuery linkQuery;
            linkQuery.prepare("INSERT OR IGNORE INTO note_tags (note_id, tag_id) VALUES (:note_id, :tag_id)");
            linkQuery.bindValue(":note_id", id);
            linkQuery.bindValue(":tag_id", tagId);
            linkQuery.exec();
        }
    }
    return true;
}

bool DatabaseManager::deleteNote(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

// РЕАЛІЗАЦІЯ РЕЗЕРВНОГО КОПІЮВАННЯ
bool DatabaseManager::backupDatabase()
{
    QString sourceFile = "C:/Users/User/Desktop/Coursework/notes.db";

    // Формуємо унікальну назву файлу з поточною датою та часом
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backupFile = "C:/Users/User/Desktop/Coursework/notes_backup_" + timestamp + ".db";

    // Копіюємо файл за допомогою засобів Qt
    if (QFile::copy(sourceFile, backupFile)) {
        qDebug() << "Резервну копію створено успішно:" << backupFile;
        return true;
    } else {
        qDebug() << "Не вдалося створити копію бази даних!";
        return false;
    }
}
