#include "manager.h"
#include <iostream>
#include <vector>

ClientManager::ClientManager(const std::string& conn_string)
    : conn(conn_string)
{
    if (!conn.is_open()) {
        throw std::runtime_error("Не удалось подключиться к БД");
    }
    conn.set_client_encoding("UTF8");
    std::cout << "Подключение к БД установлено\n";
}

void ClientManager::execute_query(const std::string& query) {
    pqxx::work txn(conn);
    txn.exec(query);
    txn.commit();
}

void ClientManager::create_database_structure() {
    try {
        std::string create_clients = R"(
            CREATE TABLE IF NOT EXISTS clients (
                id SERIAL PRIMARY KEY,
                first_name VARCHAR(50) NOT NULL,
                last_name VARCHAR(50) NOT NULL,
                email VARCHAR(100) UNIQUE NOT NULL
            );
        )";

        std::string create_phones = R"(
            CREATE TABLE IF NOT EXISTS phones (
                id SERIAL PRIMARY KEY,
                client_id INTEGER REFERENCES clients(id) ON DELETE CASCADE,
                phone_number VARCHAR(20) NOT NULL,
                CONSTRAINT unique_client_phone UNIQUE (client_id, phone_number)
            );
        )";

        execute_query(create_clients);
        execute_query(create_phones);

        std::cout << "Структура базы данных создана успешно\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при создании структуры: " << e.what() << std::endl;
    }
}

int ClientManager::add_client(const std::string& first_name,
    const std::string& last_name,
    const std::string& email) {
    try {
        pqxx::work txn(conn);

        std::string query =
            "INSERT INTO clients (first_name, last_name, email) "
            "VALUES (" + txn.quote(first_name) + ", " +
            txn.quote(last_name) + ", " +
            txn.quote(email) + ") "
            "RETURNING id;";

        pqxx::result result = txn.exec(query);
        txn.commit();

        int client_id = result[0][0].as<int>();
        std::cout << "Клиент добавлен с ID: " << client_id << std::endl;
        return client_id;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при добавлении клиента: " << e.what() << std::endl;
        return -1;
    }
}

void ClientManager::add_phone(int client_id, const std::string& phone) {
    try {
        pqxx::work txn(conn);

        std::string query = "INSERT INTO phones (client_id, phone_number) VALUES (" +
            std::to_string(client_id) + ", " +
            txn.quote(phone) + ");";

        txn.exec(query);
        txn.commit();

        std::cout << "Телефон " << phone << " добавлен клиенту ID " << client_id << std::endl;
    }
    catch (const pqxx::foreign_key_violation& e) {
        std::cerr << "Ошибка: клиент с ID " << client_id << " не существует" << std::endl;
    }
    catch (const pqxx::unique_violation& e) {
        std::cerr << "Ошибка: телефон " << phone << " уже существует у этого клиента" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при добавлении телефона: " << e.what() << std::endl;
    }
}

void ClientManager::update_client(int client_id,
    const std::string& first_name,
    const std::string& last_name,
    const std::string& email) {
    try {
        pqxx::work txn(conn);
        std::vector<std::string> updates;

        if (!first_name.empty()) {
            updates.push_back("first_name = " + txn.quote(first_name));
        }
        if (!last_name.empty()) {
            updates.push_back("last_name = " + txn.quote(last_name));
        }
        if (!email.empty()) {
            updates.push_back("email = " + txn.quote(email));
        }

        if (updates.empty()) {
            std::cout << "Нет данных для обновления\n";
            return;
        }

        std::string query = "UPDATE clients SET " + updates[0];
        for (size_t i = 1; i < updates.size(); ++i) {
            query += ", " + updates[i];
        }
        query += " WHERE id = " + std::to_string(client_id) + ";";

        txn.exec(query);
        txn.commit();

        std::cout << "Данные клиента ID " << client_id << " обновлены\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при обновлении клиента: " << e.what() << std::endl;
    }
}

void ClientManager::delete_phone(int client_id, const std::string& phone) {
    try {
        pqxx::work txn(conn);

        std::string query = "DELETE FROM phones WHERE client_id = " +
            std::to_string(client_id) +
            " AND phone_number = " + txn.quote(phone) + ";";

        pqxx::result result = txn.exec(query);
        txn.commit();

        if (result.affected_rows() > 0) {
            std::cout << "Телефон " << phone << " удален у клиента ID " << client_id << std::endl;
        }
        else {
            std::cout << "Телефон " << phone << " не найден у клиента ID " << client_id << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при удалении телефона: " << e.what() << std::endl;
    }
}

void ClientManager::delete_client(int client_id) {
    try {
        pqxx::work txn(conn);

        std::string query = "DELETE FROM clients WHERE id = " +
            std::to_string(client_id) + ";";

        pqxx::result result = txn.exec(query);
        txn.commit();

        if (result.affected_rows() > 0) {
            std::cout << "Клиент ID " << client_id << " удален\n";
        }
        else {
            std::cout << "Клиент с ID " << client_id << " не найден\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при удалении клиента: " << e.what() << std::endl;
    }
}

void ClientManager::find_client(const std::string& search_term) {
    try {
        pqxx::work txn(conn);

        std::string query =
            "SELECT c.id, c.first_name, c.last_name, c.email "
            "FROM clients c "
            "WHERE c.first_name LIKE '%" + search_term + "%' "
            "OR c.last_name LIKE '%" + search_term + "%' "
            "OR c.email LIKE '%" + search_term + "%' "
            "ORDER BY c.last_name, c.first_name;";

        pqxx::result result = txn.exec(query);

        if (result.empty()) {
            std::cout << "Клиенты не найдены по запросу \"" << search_term << "\"\n";
            return;
        }

        std::cout << "\nРезультаты поиска по \"" << search_term << "\":\n";
        std::cout << "----------------------------------------\n";

        for (const auto& row : result) {
            std::cout << "ID: " << row["id"].as<int>() << "\n";
            std::cout << "Имя: " << row["first_name"].c_str() << " " << row["last_name"].c_str() << "\n";
            std::cout << "Email: " << row["email"].c_str() << "\n";

            pqxx::work txn2(conn);
            std::string phone_query =
                "SELECT phone_number FROM phones WHERE client_id = " +
                std::to_string(row["id"].as<int>()) + ";";

            pqxx::result phone_result = txn2.exec(phone_query);

            if (!phone_result.empty()) {
                std::cout << "Телефоны: ";
                for (size_t i = 0; i < phone_result.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << phone_result[i]["phone_number"].c_str();
                }
                std::cout << "\n";
            }
            else {
                std::cout << "Телефоны: не указаны\n";
            }
            std::cout << "----------------------------------------\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при поиске: " << e.what() << std::endl;
    }
}

void ClientManager::clear_database() {
    try {
        pqxx::work txn(conn);
        txn.exec("DELETE FROM phones;");
        txn.exec("DELETE FROM clients;");
        txn.exec("ALTER SEQUENCE clients_id_seq RESTART WITH 1;");
        txn.exec("ALTER SEQUENCE phones_id_seq RESTART WITH 1;");
        txn.commit();
        std::cout << "База данных очищена\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при очистке: " << e.what() << std::endl;
    }
}