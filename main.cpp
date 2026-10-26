#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // 1. Tworzymy gniazdo: IPv4 (AF_INET), strumieniowe = TCP (SOCK_STREAM)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Pozwala od razu uruchomić program ponownie po zamknięciu
    // (bez tego bind() potrafi zgłosić "Address already in use")
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Adres, na którym nasłuchujemy
    sockaddr_in addr{};                 // {} zeruje całą strukturę
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // wszystkie interfejsy sieciowe
    addr.sin_port = htons(1883);        // htons: zamiana na kolejność bajtów sieciową

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // 3. Zaczynamy nasłuchiwać (10 = ile połączeń może czekać w kolejce)
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }
    std::cout << "Nasluchuje na porcie 1883...\n";

    // 4. Czekamy na klienta (program tu "stoi", aż ktoś się połączy)
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        return 1;
    }
    std::cout << "Klient polaczony!\n";

    // 5. Pętla echo: odbieramy dane i odsyłamy je z powrotem
    char buffer[1024];
    while (true) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
        if (n <= 0) {   // 0 = klient się rozłączył, <0 = błąd
            break;
        }
        send(client_fd, buffer, n, 0);
    }

    std::cout << "Klient rozlaczony.\n";
    close(client_fd);
    close(server_fd);
    return 0;
}