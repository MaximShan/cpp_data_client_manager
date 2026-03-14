#include "manager.h"
#include <iostream>
#include <windows.h>
#include <clocale>
#include <vector>

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
        try {
            manager.add_phone(id1, "+7-123-456-78-90");
            manager.add_phone(id1, "+7-098-765-43-21");
            manager.add_phone(id2, "+7-555-123-45-67");
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при добавлении телефона: " << e.what() << std::endl;
        }

        std::cout << "\nStep 4: Searching clients\n";

        std::cout << "----------------------------------------\n";
        auto clients1 = manager.find_client("Ivan");
        for (const auto& client : clients1) {
            std::cout << "ID: " << client.id << "\n";
            std::cout << "Имя: " << client.first_name << " " << client.last_name << "\n";
            std::cout << "Email: " << client.email << "\n";
            if (!client.phones.empty()) {
                std::cout << "Телефоны: ";
                for (size_t i = 0; i < client.phones.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << client.phones[i];
                }
                std::cout << "\n";
            }
            else {
                std::cout << "Телефоны: не указаны\n";
            }
            std::cout << "----------------------------------------\n";
        }

      
        std::cout << "----------------------------------------\n";
        auto clients2 = manager.find_client("555");
        for (const auto& client : clients2) {
            std::cout << "ID: " << client.id << "\n";
            std::cout << "Имя: " << client.first_name << " " << client.last_name << "\n";
            std::cout << "Email: " << client.email << "\n";
            if (!client.phones.empty()) {
                std::cout << "Телефоны: ";
                for (size_t i = 0; i < client.phones.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << client.phones[i];
                }
                std::cout << "\n";
            }
            else {
                std::cout << "Телефоны: не указаны\n";
            }
            std::cout << "----------------------------------------\n";
        }

       
        std::cout << "----------------------------------------\n";
        auto clients3 = manager.find_client("email");
        for (const auto& client : clients3) {
            std::cout << "ID: " << client.id << "\n";
            std::cout << "Имя: " << client.first_name << " " << client.last_name << "\n";
            std::cout << "Email: " << client.email << "\n";
            if (!client.phones.empty()) {
                std::cout << "Телефоны: ";
                for (size_t i = 0; i < client.phones.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << client.phones[i];
                }
                std::cout << "\n";
            }
            else {
                std::cout << "Телефоны: не указаны\n";
            }
            std::cout << "----------------------------------------\n";
        }

        std::cout << "\nStep 5: Updating client data\n";
        manager.update_client(id2, "Maria", "Petrova", "");

        std::cout << "\nStep 6: Deleting phone\n";
        manager.delete_phone(id1, "+7-098-765-43-21");

        std::cout << "\nStep 7: Deleting client\n";
        manager.delete_client(id3);

        std::cout << "\nAll operations completed successfully!\n";
    }
    catch (const pqxx::sql_error& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
        std::cerr << "Query: " << e.query() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}