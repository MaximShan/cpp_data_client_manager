#include "manager.h"
#include <iostream>
#include <windows.h>
#include <clocale>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    try {
        ClientManager manager("host=localhost port=5432 dbname=client user=postgres password=29071984");

        std::cout << "CLIENT MANAGEMENT\n";
        std::cout << "====================\n\n";

        std::cout << "Step 0: Clearing database\n";
        manager.clear_database();

        std::cout << "\nStep 1: Creating database structure\n";
        manager.create_database_structure();

        std::cout << "\nStep 2: Adding clients\n";
        int id1 = manager.add_client("Ivan", "Petrov", "ivan@email.com");
        int id2 = manager.add_client("Maria", "Ivanova", "maria@email.com");
        int id3 = manager.add_client("John", "Smith", "john@email.com");

        std::cout << "\nStep 3: Adding phones\n";
        if (id1 != -1) manager.add_phone(id1, "+7-123-456-78-90");
        if (id1 != -1) manager.add_phone(id1, "+7-098-765-43-21");
        if (id2 != -1) manager.add_phone(id2, "+7-555-123-45-67");

        std::cout << "\nStep 4: Searching clients\n";
        if (id1 != -1) manager.find_client("Ivan");
        if (id2 != -1) manager.find_client("+7-555");
        manager.find_client("@email");

        std::cout << "\nStep 5: Updating client data\n";
        if (id2 != -1) manager.update_client(id2, "Maria", "Petrova", "");

        std::cout << "\nStep 6: Deleting phone\n";
        if (id1 != -1) manager.delete_phone(id1, "+7-098-765-43-21");

        std::cout << "\nStep 7: Deleting client\n";
        if (id3 != -1) manager.delete_client(id3);

        std::cout << "\nAll operations completed successfully!\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}