import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: mainWindow
    width: 850
    height: 600
    visible: true
    title: qsTr("Система управління нотатками")

    property int selectedNoteId: -1

    function refreshNotes() {
        notesListView.model = myDbManager.getNotes(searchInput.text)
    }

    Component.onCompleted: {
        refreshNotes()
    }

    // Спливаюче вікно для підтвердження бекапу
    Popup {
        id: backupPopup
        anchors.centerIn: parent
        width: 300
        height: 100
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#f0f0f0"
            border.color: "#cccccc"
            radius: 10
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            Label {
                text: "Резервну копію успешно створено!"
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }
            Button {
                text: "Чудово"
                Layout.alignment: Qt.AlignHCenter
                onClicked: backupPopup.close()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // ЛІВА ПАНЕЛЬ
        ColumnLayout {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            spacing: 10

            TextField {
                id: searchInput
                Layout.fillWidth: true
                placeholderText: "Пошук (текст або тег)..."
                onTextChanged: {
                    mainWindow.refreshNotes()
                }
            }

            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                    id: notesListView
                    anchors.fill: parent
                    clip: true
                    spacing: 5

                    delegate: ItemDelegate {
                        width: parent.width
                        text: "<b>" + modelData.title + "</b><br><font color='gray'>" + modelData.date + "</font>"

                        onClicked: {
                            mainWindow.selectedNoteId = modelData.id
                            titleInput.text = modelData.title
                            contentInput.text = modelData.content
                            tagsInput.text = myDbManager.getNoteTags(modelData.id)
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                Button {
                    Layout.fillWidth: true
                    text: "+ Додати"
                    highlighted: true
                    onClicked: {
                        mainWindow.selectedNoteId = -1
                        titleInput.text = ""
                        contentInput.text = ""
                        tagsInput.text = ""
                    }
                }

                Button {
                    text: "Зробити бекап"
                    onClicked: {
                        if (myDbManager.backupDatabase()) {
                            backupPopup.open()
                        }
                    }
                }
            }
        }

        // ПРАВА ПАНЕЛЬ
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            TextField {
                id: titleInput
                Layout.fillWidth: true
                font.pointSize: 16
                font.bold: true
                placeholderText: "Введіть заголовок..."
            }

            TextField {
                id: tagsInput
                Layout.fillWidth: true
                placeholderText: "Теги (через кому)..."
            }

            // ПАНЕЛЬ ДЛЯ ФОРМАТУВАННЯ ТЕКСТУ
            RowLayout {
                spacing: 5
                Layout.fillWidth: true

                Button {
                    text: "<b>Ж</b>"
                    implicitWidth: 40
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (contentInput.selectedText.length > 0) {
                            var start = contentInput.selectionStart
                            var end = contentInput.selectionEnd
                            var formatted = "<b>" + contentInput.selectedText + "</b>"
                            contentInput.remove(start, end)
                            contentInput.insert(start, formatted)
                        }
                    }
                }

                Button {
                    text: "<i>К</i>"
                    implicitWidth: 40
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (contentInput.selectedText.length > 0) {
                            var start = contentInput.selectionStart
                            var end = contentInput.selectionEnd
                            var formatted = "<i>" + contentInput.selectedText + "</i>"
                            contentInput.remove(start, end)
                            contentInput.insert(start, formatted)
                        }
                    }
                }

                // НАША НОВА КНОПКА СКИДАННЯ ФОРМАТУВАННЯ
                Button {
                    text: "Скинути"
                    implicitWidth: 75
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (contentInput.selectedText.length > 0) {
                            var start = contentInput.selectionStart
                            var end = contentInput.selectionEnd

                            // Видаляємо всі HTML-теги з виділеного тексту
                            var plainText = contentInput.selectedText.replace(/<[^>]*>/g, "")

                            contentInput.remove(start, end)
                            contentInput.insert(start, plainText)
                        }
                    }
                }

                Label {
                    text: " (виділіть текст мишкою)"
                    color: "gray"
                    font.italic: true
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                TextArea {
                    id: contentInput
                    placeholderText: "Почніть писати тут..."
                    wrapMode: TextArea.Wrap
                    textFormat: TextArea.RichText
                    font.pointSize: 12
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10

                Button {
                    text: "Видалити"
                    enabled: mainWindow.selectedNoteId !== -1
                    onClicked: {
                        if (myDbManager.deleteNote(mainWindow.selectedNoteId)) {
                            titleInput.text = ""
                            contentInput.text = ""
                            tagsInput.text = ""
                            mainWindow.selectedNoteId = -1
                            mainWindow.refreshNotes()
                        }
                    }
                }

                Button {
                    text: "Зберегти"
                    highlighted: true

                    onClicked: {
                        var success = false;
                        if (mainWindow.selectedNoteId === -1) {
                            success = myDbManager.addNote(titleInput.text, contentInput.text, tagsInput.text);
                        } else {
                            success = myDbManager.updateNote(mainWindow.selectedNoteId, titleInput.text, contentInput.text, tagsInput.text);
                        }

                        if (success) {
                            titleInput.text = "";
                            contentInput.text = "";
                            tagsInput.text = "";
                            mainWindow.selectedNoteId = -1;
                            mainWindow.refreshNotes();
                        }
                    }
                }
            }
        }
    }
}
