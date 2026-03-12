#pragma once


#include <pqxx/pqxx>
#include <string>
#include <vector>

class ClientManager {
private:
    pqxx::connection conn;
    void execute_query(const std::string& query);

public:
    ClientManager(const std::string& conn_string);
    void create_database_structure();
    int add_client(const std::string& first_name, const std::string& last_name, const std::string& email);
    void add_phone(int client_id, const std::string& phone);
    void update_client(int client_id, const std::string& first_name = "", const std::string& last_name = "", const std::string& email = "");
    void delete_phone(int client_id, const std::string& phone);
    void delete_client(int client_id);
    void find_client(const std::string& search_term);
    void clear_database();
};
