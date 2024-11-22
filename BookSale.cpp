// BookSale.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <limits>
#define NOMINMAX
#include <windows.h>

using namespace std;

struct Books {
    unsigned int bookID;
    string authors;
    string title;
    string releaseDate;
    double price;
    int quantity;
    string saleDate;
} book;


string currentFile;

void createFile();
void selectFile();
void checkCurrentFile();
void addRecord();
void addRecordByNumber();
void displayRecords();
void deleteRecordByNumber();
void deleteRecordsByReleaseDate();
void updateBookPrice();
void displayBooksInTimeRange();
void backupFile();
void restoreFileFromBackup();
void deleteFile();

int displayMenu();
void processUserChoice(int choice);
void runApplication();
void addRecordMenu();
Books inputNewRecord();
bool isBookIDUnique(int bookID);
string inputReleaseDate();
bool isValidDate(const string& date);
int countLinesInFile(const string& filename);
void addTableHeader(ofstream& file);
void writeRecord(ofstream& file, const Books& book);
template <typename FilterFunc>
void processFileWithFilter(FilterFunc shouldDelete, const string& errorMessage);


int main() {
    setlocale(LC_ALL, "RU");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    try {
        runApplication();
    }
    catch (const exception& e) {
        cerr << "Произошла ошибка приложения: " << e.what() << endl;
    }
    return 0;
}


void runApplication() {
    int choice;
    do {
        choice = displayMenu();
        try {
            processUserChoice(choice);
        }
        catch (const exception& e) {
            cerr << "Ошибка при выполнении действия: " << e.what() << endl;
        }
    } while (choice != 0);
}

int displayMenu() {
    int choice;
    cout << "\nМеню:\n";
    cout << "1. Создать новый файл\n";
    cout << "2. Выбрать файл для работы\n";
    cout << "3. Добавить запись в файл\n";
    cout << "4. Вывести все записи\n";
    cout << "5. Удалить запись по номеру\n";
    cout << "6. Удалить записи по дате выпуска\n";
    cout << "7. Изменить цену книги\n";
    cout << "8. Вывести книги в промежутке времени\n";
    cout << "9. Скопировать данные в резервный файл\n";
    cout << "10. Восстановить файл из резервного\n";
    cout << "11. Удалить файл\n";
    cout << "0. Выход\n";
    cout << "Ваш выбор: ";
    cin >> choice;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw invalid_argument("Некорректный ввод! Пожалуйста, введите число.");
    }

    return choice;
}

void processUserChoice(int choice) {
    switch (choice) {
    case 1: createFile(); break;
    case 2: selectFile(); break;
    case 3: addRecordMenu(); break;
    case 4: displayRecords(); break;
    case 5: deleteRecordByNumber(); break;
    case 6: deleteRecordsByReleaseDate(); break;
    case 7: updateBookPrice(); break;
    case 8: displayBooksInTimeRange(); break;
    case 9: backupFile(); break;
    case 10: restoreFileFromBackup(); break;
    case 11: deleteFile(); break;
    case 0: cout << "Выход из программы.\n"; break;
    default: throw invalid_argument("Некорректный выбор! Пожалуйста, выберите существую-щий пункт меню.");
    }
}

void createFile() {
    cout << "Введите имя файла: ";
    string filename;
    cin >> filename;

    ifstream existingFile(filename);
    if (existingFile) {
        existingFile.close();
        cout << "Файл \"" << filename << "\" уже существует. Все записи файла \"" << filename << "\" будут уничтожены! Перезаписать? (y/n): ";
        char response;
        cin >> response;
        if (response != 'y' && response != 'Y') {
            cout << "Файл не был создан.\n";
            return;
        }
    }

    ofstream newFile(filename, ios::trunc);
    if (!newFile) {
        throw runtime_error("Не удалось создать файл \"" + filename + "\".");
    }
    cout << "Файл \"" << filename << "\" успешно создан.\n";

    currentFile = filename;
}

void selectFile() {
    cout << "Введите имя файла для работы: ";
    string filename;
    cin >> filename;

    ifstream file(filename);
    if (!file) {
        cout << "Файл \"" << filename << "\" не существует. Создать его? (y/n): ";
        char response;
        cin >> response;
        if (response == 'y' || response == 'Y') {
            ofstream newFile(filename);
            if (!newFile) {
                throw runtime_error("Не удалось создать файл \"" + filename + "\".");
            }
            cout << "Файл \"" << filename << "\" успешно создан.\n";
        }
        else {
            cout << "Выбор файла отменён.\n";
            return;
        }
    }

    currentFile = filename;
    cout << "Файл \"" << filename << "\" выбран для работы.\n";
}

void checkCurrentFile() {
    if (currentFile.empty()) {
        throw runtime_error("Файл не выбран. Пожалуйста, выберите файл из меню 2. \"Выбрать файл для работы\"");
    }
}

void addRecordMenu() {
    checkCurrentFile();

    cout << "Выберите способ добавления записи:\n";
    cout << "1. Добавить запись в конец.\n";
    cout << "2. Добавить запись по номеру.\n";
    cout << "0. Отменить добавление записи.\n";
    cout << "Введите ваш выбор: ";

    int choice;
    cin >> choice;
    cin.ignore();

    switch (choice) {
    case 1:
        addRecord();
        break;
    case 2:
        addRecordByNumber();
        break;
    case 0:
        cout << "Добавление записи отменено.\n";
        break;
    default:
        cout << "Неверный выбор. Возврат в главное меню.\n";
        break;
    }
}

void addRecord() {
    ofstream file(currentFile, ios::app);
    if (!file) {
        throw runtime_error("Ошибка открытия файла \"" + currentFile + "\" для добавления записи.");
    }

    file.seekp(0, ios::end);
    if (file.tellp() == 0) {
        addTableHeader(file);
    }

    cout << "Начало ввода записей. Введите данные книги или * для завершения ввода.\n";
    while (true) {
        cout << "\nВведите * для завершения или любую клавишу для добавления записи: ";
        string choice;
        cin >> choice;
        if (choice == "*") {
            cout << "Ввод записей завершён.\n";
            break;
        }

        try {
            Books book = inputNewRecord();
            writeRecord(file, book);
            cout << "Запись успешно добавлена.\n";
        }
        catch (const runtime_error& e) {
            cerr << "Ошибка при выполнении записи в конец файла: " << e.what() << "\n";
            continue;
        }
    }
    file.close();
}

void addRecordByNumber() {
    cout << "Введите номер позиции для добавления записи: ";
    int position;
    cin >> position;
    cin.ignore();

    int totalRecords = countLinesInFile(currentFile) - 2;
    if (position < 0 || (position - totalRecords) > 1) {
        cout << "Недопустимая позиция для добавления записи. Разница с количеством запи-сей должна быть не более 1.\n";
        return;
    }

    ofstream file(currentFile, ios::app);
    if (!file) {
        throw runtime_error("Ошибка открытия файла \"" + currentFile + "\" для добавления записи.");
    }

    file.seekp(0, ios::end);
    if (file.tellp() == 0) {
        addTableHeader(file);
    }

    file.close();

    try {
        Books book = inputNewRecord();

        ifstream inputFile(currentFile);
        ofstream tempFile("temp.txt");
        if (!inputFile || !tempFile) {
            throw runtime_error("Ошибка открытия файлов для добавления записи.");
        }

        int currentLine = 0;
        bool isWrite = false;
        string line;
        while (getline(inputFile, line)) {
            if (currentLine == position + 1) {
                writeRecord(tempFile, book);
                isWrite = true;
            }
            tempFile << line << "\n";
            ++currentLine;
        }

        if (!isWrite) {
            writeRecord(tempFile, book);
        }

        inputFile.close();
        tempFile.close();

        if (remove(currentFile.c_str()) != 0) {
            perror("Ошибка при удалении оригинального файла");
            return;
        }
        if (rename("temp.txt", currentFile.c_str()) != 0) {
            perror("Ошибка при переименовании временного файла");
            return;
        }

        cout << "Запись успешно добавлена на позицию " << position << ".\n";
    }
    catch (const runtime_error& e) {
        cerr << "Ошибка при выполнении записи в файл по позиции: " << e.what() << "\n";
    }
}

Books inputNewRecord() {
    Books book;

    cout << "\nВведите номер книги: ";
    string input;
    cin >> input;
    cin.ignore();
    try {
        book.bookID = stoi(input);

        if (!isBookIDUnique(book.bookID)) {
            throw runtime_error("Номер книги уже существует. Введите уникальный номер.");
        }
    }
    catch (const exception& e) {
        throw runtime_error(string("Ошибка ввода номера книги: ") + e.what());
    }

    cout << "Введите авторов (или ? если автор неизвестен): ";
    getline(cin, book.authors);
    if (book.authors == "?") {
        book.authors = "Неизвестно";
    }

    cout << "Введите название книги: ";
    getline(cin, book.title);

    book.releaseDate = inputReleaseDate();

    cout << "Введите цену: ";
    cin >> input;
    try {
        book.price = stod(input);
    }
    catch (const exception&) {
        throw runtime_error("Ошибка ввода цены. Введите число.");
    }

    cout << "Введите количество: ";
    cin >> input;
    try {
        book.quantity = stoi(input);
    }
    catch (const exception&) {
        throw runtime_error("Ошибка ввода количества. Введите целое число.");
    }

    cout << "Введите дату продажи (ГГГГ-ММ-ДД): ";
    cin >> book.saleDate;
    if (!isValidDate(book.saleDate)) {
        throw runtime_error("Некорректная дата продажи.");
    }

    return book;
}

bool isBookIDUnique(int bookID) {
    ifstream file(currentFile);
    if (!file) {
        throw runtime_error("Ошибка открытия файла \"" + currentFile + "\" для проверки уникальности.");
    }

    string line;

    for (int i = 0; i < 2; ++i) {
        if (!getline(file, line)) {
            break;
        }
    }

    while (getline(file, line)) {
        istringstream record(line);
        string idStr;
        if (getline(record, idStr, '|')) {
            try {
                int existingID = stoi(idStr);
                if (existingID == bookID) {
                    file.close();
                    return false;
                }
            }
            catch (const exception&) {
                cerr << "Ошибка при чтении данных из файла.\n";
                file.close();
                return false;
            }
        }
    }
    file.close();
    return true;
}

string inputReleaseDate() {
    while (true) {
        cout << "Введите дату выпуска (ГГГГ-ММ-ДД, только год ГГГГ, или ? если дата неиз-вестна): ";
        string date;
        cin >> date;

        if (date == "?") {
            return "Неизвестно";
        }

        if (date.length() == 4) {
            try {
                int year = stoi(date);
                if (year < 1000 || year > 9999) {
                    throw invalid_argument("Год должен быть 4-значным числом.");
                }
                return date + "-00-00";
            }
            catch (const exception&) {
                cerr << "Ошибка: некорректный год. Попробуйте снова.\n";
                continue;
            }
        }

        if (isValidDate(date)) {
            return date;
        }
        else {
            cerr << "Ошибка: некорректная дата. Используйте формат ГГГГ-ММ-ДД.\n";
        }
    }
}

bool isValidDate(const string& date) {
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }

    try {
        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));

        if (year < 1000 || year > 9999) {
            return false;
        }
        if (month < 1 || month > 12) {
            return false;
        }

        static const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        int maxDays = daysInMonth[month - 1];

        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
            maxDays = 29;
        }

        if (day < 1 || day > maxDays) {
            return false;
        }

        return true;
    }
    catch (const exception&) {
        return false;
    }
}

int countLinesInFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Не удалось открыть файл \"" + filename + "\" для подсчёта строк.");
    }

    int count = 0;
    string line;
    while (getline(file, line)) {
        ++count;
    }
    return count;
}

void addTableHeader(ofstream& file) {
    file << left << setw(10) << "ID" << "|"
        << setw(20) << "Авторы" << "|"
        << setw(30) << "Название" << "|"
        << setw(15) << "Дата выхода" << "|"
        << setw(10) << "Цена" << "|"
        << setw(10) << "Кол-во" << "|"
        << setw(15) << "Дата продажи" << "|"
        << "\n";
    file << string(110, '-') << "\n";
}

void writeRecord(ofstream& file, const Books& book) {
    file << left << setw(10) << book.bookID << "|"
        << setw(20) << book.authors << "|"
        << setw(30) << book.title << "|"
        << setw(15) << book.releaseDate << "|"
        << setw(10) << fixed << setprecision(2) << book.price << "|"
        << setw(10) << book.quantity << "|"
        << setw(15) << book.saleDate << "|"
        << "\n";
}

void displayRecords() {
    ifstream file(currentFile);
    if (!file) {
        cerr << "Ошибка открытия файла \"" + currentFile + "\" для чтения записи. \n";
        return;
    }

    string line;
    while (getline(file, line)) {
        cout << line << '\n';
    }
    file.close();
}

void deleteRecordByNumber() {
    cout << "Введите номер записи для удаления: ";
    int targetNumber;
    cin >> targetNumber;

    targetNumber = --targetNumber;
    int totalRecords = countLinesInFile(currentFile) - 2;
    if (targetNumber < 0 || targetNumber >= totalRecords) {
        cerr << "Ошибка: номер записи за пределами допустимого диапазона (1 - " << totalRecords << ").\n";
        return;
    }

    processFileWithFilter(
        [targetNumber, currentLine = 0](const string&) mutable {
            return currentLine++ == targetNumber;
        },
        "Запись с указанным номером не найдена."
    );
}

void deleteRecordsByReleaseDate() {
    cout << "Удаление записей по дате выпуска.\n";

    string targetDate = inputReleaseDate();

    processFileWithFilter(
        [targetDate](const string& line) {
            istringstream record(line);
            string id, authors, title, releaseDate, price, quantity, saleDate;

            if (getline(record, id, '|') &&
                getline(record, authors, '|') &&
                getline(record, title, '|') &&
                getline(record, releaseDate, '|')) {

                releaseDate.erase(0, releaseDate.find_first_not_of(" \t"));
                releaseDate.erase(releaseDate.find_last_not_of(" \t") + 1);

                if (releaseDate == targetDate) return true;
            }
            return false;
        },
        "Записи с указанной датой выпуска не найдены."
    );
}

template <typename FilterFunc>
void processFileWithFilter(FilterFunc shouldDelete, const string& errorMessage) {
    ifstream inputFile(currentFile);
    if (!inputFile) {
        cerr << "Ошибка: не удалось открыть файл \"" << currentFile << "\" для чте-ния.\n";
        return;
    }

    ofstream tempFile("temp.txt");
    if (!tempFile) {
        cerr << "Ошибка: не удалось создать временный файл для записи.\n";
        inputFile.close();
        return;
    }

    string line;
    int lineCount = 0;
    int removedCount = 0;

    while (getline(inputFile, line)) {
        ++lineCount;
        if (lineCount == 1 || lineCount == 2) {
            tempFile << line << "\n";
            continue;
        }

        if (shouldDelete(line)) {
            ++removedCount;
            continue;
        }

        tempFile << line << "\n";
    }

    inputFile.close();
    tempFile.close();

    if (remove(currentFile.c_str()) != 0) {
        perror("Ошибка при удалении оригинального файла");
        return;
    }
    if (rename("temp.txt", currentFile.c_str()) != 0) {
        perror("Ошибка при переименовании временного файла");
        return;
    }

    if (removedCount > 0) {
        cout << "Удалено записей: " << removedCount << ".\n";
    }
    else {
        cout << errorMessage << "\n";
    }
}

void updateBookPrice() {
    checkCurrentFile();

    cout << "Введите ID книги для изменения цены: ";
    int bookID;
    cin >> bookID;
    cin.ignore();

    try {
        if (isBookIDUnique(bookID)) {
            cout << "Книга с ID " << bookID << " не найдена в файле.\n";
            return;
        }

        cout << "Введите новую цену книги: ";
        string newPriceStr;
        cin >> newPriceStr;

        try {
            double price = stod(newPriceStr);
            if (price < 0) {
                cout << "Ошибка: Цена не может быть отрицательной.\n";
                return;
            }
        }
        catch (const invalid_argument&) {
            cout << "Ошибка: Введено некорректное значение для цены.\n";
            return;
        }

        ifstream inputFile(currentFile);
        ofstream tempFile("temp.txt");
        if (!inputFile || !tempFile) {
            throw runtime_error("Ошибка открытия файла.");
        }

        string line;
        int lineCount = 0;
        bool found = false;

        while (getline(inputFile, line)) {
            ++lineCount;
            if (lineCount == 1 || lineCount == 2) {
                tempFile << line << "\n";
                continue;
            }

            istringstream record(line);
            string idStr, authors, title, releaseDate, priceStr, quantity, saleDate;

            if (getline(record, idStr, '|') &&
                getline(record, authors, '|') &&
                getline(record, title, '|') &&
                getline(record, releaseDate, '|') &&
                getline(record, priceStr, '|') &&
                getline(record, quantity, '|') &&
                getline(record, saleDate)) {

                idStr.erase(0, idStr.find_first_not_of(" \t"));
                idStr.erase(idStr.find_last_not_of(" \t") + 1);

                try {
                    int existingID = stoi(idStr);
                    if (existingID == bookID) {
                        found = true;
                        tempFile << left << setw(10) << idStr << "|"
                            << setw(20) << authors << "|"
                            << setw(30) << title << "|"
                            << setw(15) << releaseDate << "|"
                            << setw(10) << fixed << setprecision(2) << newPriceStr << "|"
                            << setw(10) << quantity << "|"
                            << setw(15) << saleDate << "\n";
                    }
                    else {
                        tempFile << line << "\n";
                    }
                }
                catch (const invalid_argument&) {
                    cerr << "Ошибка: некорректный ID в файле.\n";
                    tempFile << line << "\n";
                }
            }
            else {
                tempFile << line << "\n";
            }
        }

        inputFile.close();
        tempFile.close();

        if (remove(currentFile.c_str()) != 0) {
            perror("Ошибка при удалении оригинального файла");
            return;
        }
        if (rename("temp.txt", currentFile.c_str()) != 0) {
            perror("Ошибка при переименовании временного файла");
            return;
        }

        if (found) {
            cout << "Цена книги с ID " << bookID << " успешно обновлена.\n";
        }
        else {
            cout << "Книга с ID " << bookID << " не найдена в файле.\n";
        }
    }
    catch (const runtime_error& e) {
        cerr << "Ошибка при выполнении действия: " << e.what() << "\n";
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << "\n";
    }
}

void displayBooksInTimeRange() {
    ifstream file(currentFile);
    if (!file) {
        cerr << "Ошибка открытия файла \"" << currentFile << "\" для чтения.\n";
        return;
    }

    string startDate, endDate;
    cout << "Введите начальную дату (в формате ГГГГ-ММ-ДД): ";
    cin >> startDate;
    if (!isValidDate(startDate)) {
        cerr << "Ошибка: некорректная начальная дата.\n";
        return;
    }

    cout << "Введите конечную дату (в формате ГГГГ-ММ-ДД): ";
    cin >> endDate;
    if (!isValidDate(endDate)) {
        cerr << "Ошибка: некорректная конечная дата.\n";
        return;
    }

    string line;

    int lineCount = 0;
    bool found = false;

    while (getline(file, line)) {
        ++lineCount;

        if (lineCount == 1 || lineCount == 2) {
            cout << line << "\n";
            continue;
        }

        istringstream record(line);
        string idStr, authors, title, releaseDate, priceStr, quantityStr, saleDate;

        if (getline(record, idStr, '|') && getline(record, authors, '|') &&
            getline(record, title, '|') && getline(record, releaseDate, '|') &&
            getline(record, priceStr, '|') && getline(record, quantityStr, '|') &&
            getline(record, saleDate, '|')) {

            if (saleDate >= startDate && saleDate <= endDate) {
                cout << left << setw(10) << idStr << "|"
                    << setw(20) << authors << "|" << setw(30) << title << "|"
                    << setw(15) << releaseDate << "|" << setw(10) << priceStr << "|"
                    << setw(10) << quantityStr << "|" << setw(15) << saleDate << "|" << "\n";
                found = true;
            }
        }
    }

    if (!found) {
        cout << "Нет записей о проданных книгах в заданный промежуток времени.\n";
    }

    file.close();
}

void backupFile() {
    checkCurrentFile();

    string backupFile = currentFile + "_backup";

    ifstream source(currentFile, ios::binary);
    if (!source) {
        throw runtime_error("Ошибка: не удалось открыть файл \"" + currentFile + "\" для чтения.");
    }

    ofstream backup(backupFile, ios::binary);
    if (!backup) {
        throw runtime_error("Ошибка: не удалось создать файл \"" + backupFile + "\" для записи.");
    }

    try {
        backup << source.rdbuf();
    }
    catch (const exception& e) {
        cerr << "Ошибка при копировании данных в резервный файл: " << e.what() << "\n";
    }

    source.close();
    backup.close();

    cout << "Данные успешно скопированы в резервный файл: \"" << backupFile << "\".\n";
}

void restoreFileFromBackup() {
    checkCurrentFile();

    string backupFile = currentFile + "_backup";

    ifstream backup(backupFile, ios::binary);
    if (!backup) {
        throw runtime_error("Ошибка: резервный файл \"" + backupFile + "\" не найден.");
    }

    ofstream restoredFile(currentFile, ios::binary);
    if (!restoredFile) {
        throw runtime_error("Ошибка: не удалось открыть файл \"" + currentFile + "\" для записи.");
    }

    try {
        restoredFile << backup.rdbuf();
    }
    catch (const std::exception& e) {
        cerr << "Ошибка при копировании данных из резервного файла: " << e.what() << "\n";
    }

    backup.close();
    restoredFile.close();

    cout << "Файл \"" << currentFile << "\" успешно восстановлен из резервного файла.\n";
}

void deleteFile() {
    checkCurrentFile();

    cout << "Вы действительно хотите удалить файл \"" << currentFile << "\"? (y/n): ";
    char confirmation;
    cin >> confirmation;

    if (tolower(confirmation) != 'y') {
        cout << "Удаление файла отменено.\n";
        return;
    }

    if (std::remove(currentFile.c_str()) != 0) {
        perror(("Ошибка при удалении файла \"" + currentFile + "\"").c_str());
        return;
    }

    cout << "Файл \"" << currentFile << "\" успешно удалён.\n";

    currentFile.clear();
}



// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
