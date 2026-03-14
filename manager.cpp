#include "manager.h"
#include <iostream>
#include <memory>

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

int ClientManager::add_client(const std::string& first_name,
    const std::string& last_name,
    const std::string& email) {

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

void ClientManager::add_phone(int client_id, const std::string& phone) {
    pqxx::work txn(conn);

    std::string query = "INSERT INTO phones (client_id, phone_number) VALUES (" +
        std::to_string(client_id) + ", " +
        txn.quote(phone) + ");";

    txn.exec(query);
    txn.commit();

    std::cout << "Телефон " << phone << " добавлен клиенту ID " << client_id << std::endl;
}

void ClientManager::update_client(int client_id,
    const std::string& first_name,
    const std::string& last_name,
    const std::string& email) {

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

void ClientManager::delete_phone(int client_id, const std::string& phone) {
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

void ClientManager::delete_client(int client_id) {
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

std::vector<Client> ClientManager::find_client(const std::string& search_term) {
    std::vector<Client> clients;

    {
        pqxx::work txn(conn);
        std::string query =
            "SELECT id, first_name, last_name, email "
            "FROM clients "
            "WHERE first_name ILIKE '%' || " + txn.quote(search_term) + " || '%' "
            "OR last_name ILIKE '%' || " + txn.quote(search_term) + " || '%' "
            "OR email ILIKE '%' || " + txn.quote(search_term) + " || '%' "
            "ORDER BY last_name, first_name;";

        pqxx::result result = txn.exec(query);

        for (const auto& row : result) {
            Client client;
            client.id = row["id"].as<int>();
            client.first_name = row["first_name"].as<std::string>();
            client.last_name = row["last_name"].as<std::string>();
            client.email = row["email"].as<std::string>();
            clients.push_back(client);
        }
    }

   
    for (auto& client : clients) {
        pqxx::work txn(conn);
        std::string phone_query =
            "SELECT phone_number FROM phones WHERE client_id = " +
            std::to_string(client.id) + ";";

        pqxx::result phone_result = txn.exec(phone_query);

        for (const auto& phone_row : phone_result) {
            client.phones.push_back(phone_row["phone_number"].as<std::string>());
        }
    }

    return clients;
}

void ClientManager::clear_database() {
    pqxx::work txn(conn);
    txn.exec("DELETE FROM phones;");
    txn.exec("DELETE FROM clients;");
    txn.exec("ALTER SEQUENCE clients_id_seq RESTART WITH 1;");
    txn.exec("ALTER SEQUENCE phones_id_seq RESTART WITH 1;");
    txn.commit();
    std::cout << "База данных очищена\n";
}