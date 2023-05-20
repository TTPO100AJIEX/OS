#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "sem/sem.h"
#include "shm/shm.h"
#include "msq/msq.h"
#include "log/log.h"
#include "rooms/rooms.h"

void stop(__attribute__ ((unused)) int signal) { }

struct MessageQueue msq;
struct Logger logger;
struct Semaphore msgsem;
#include <errno.h>
void read_logs(__attribute__ ((unused)) int signal)
{
    printf("-");
    char message[1024]; // Buffer
    printf("*");
    errno = 0;
    read_message_queue(&msq, message, 1000); // Read one message
    perror("t");
    printf("!");
    for (unsigned int i = 0; i < 100; i++)
    {
        printf("%d", (int)(message[i]));
    }
    // printf("%s", message); // Print the message
    // for (unsigned int i = 0; i < logger.destinations_amount; i++) send(logger.destinations[i], message, strlen(message), 0); // Broadcast the message to the loggers
    if (post_semaphore(&msgsem) == -1) perror("TTT");
}

int main(int argc, char** argv) // <Port>
{
    // Check the command line arguments
    if (argc < 2) { printf("Not enough command line arguments specified: <Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1); // Signals must interrupt all system calls
    signal(SIGINT, stop); // Register an empty handler for the change to take effect

    // Create a semaphore for synchronization
    struct Semaphore sem = create_semaphore("/hotel_semaphore", 1);
    if (sem.id == -1) { perror("Failed to create a semaphore"); return 1; }

    // Create a shared memory for rooms storage
    struct Memory mem = create_memory("/hotel_memory", 25 * sizeof(struct Room));
    if (!mem.mem) { perror("Failed to create the memory for rooms"); goto stop_server_1; }

    // Create a message queue for logging
    msgsem = create_semaphore("/hotel_msq_semaphore", 1);
    msq = create_message_queue("/hotel_msq");
    if (msq.id == -1) { perror("Failed to create a message queue"); goto stop_server_2; }

    // Initialize the logger
    logger = initialize_logger(&msq, &msgsem);
    if (false) { perror("Failed to initialize the logger"); goto stop_server_3; }
    signal(SIGUSR1, read_logs); // Setup the signal listener to print logs

    // Initialize the rooms
    struct Rooms rooms = initialize_rooms(mem.mem, 10, 15);
    if (false) { perror("Failed to initialize the rooms"); goto stop_server_4; }

    // Print the initial layout
    if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto stop_server_5; }
    log_message(&logger, "Initialized the rooms\n");
    log_layout(&logger, &rooms);
    if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto stop_server_5; }

    // Create the socket
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) { perror("Failed to create a socket"); goto stop_server_5; }
    // Set socket to allow easy disconnect
    int socket_flag = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (void*)(&socket_flag), sizeof(socket_flag)) == -1) { perror("Failed to setup the socket"); goto stop_server; }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); goto stop_server; }
    // Start listening for incoming connections
    if (listen(server, 5) == 1) { perror("Failed to listen for incoming connections"); goto stop_server; }
    
    // Print the log
    if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto stop_server; }
    log_message(&logger, "Started the server\n");
    if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto stop_server; }

    while (true)
    {
        // Receive the connection from the client
        int client = accept(server, NULL, 0);
        if (client == -1) break; // Stop the hotel if something went wrong (usually - a signal interruption)
        // Get the type of the client
        enum ClientType client_type = UNKNOWN;
        if (recv(client, &client_type, sizeof(client_type), 0) != sizeof(client_type)) client_type = UNKNOWN; // Something went wrong and the request should be declined
        
        switch (client_type)
        {
            case LOGGER:
            {
                if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); break; }
                if (add_log_destination(&logger, client) == -1) log_message(&logger, "Failed to register a logger\n");
                else log_message(&logger, "Registered a logger successfully\n");
                if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); break; }
                break;
            }
            case VISITOR:
            {
                pid_t process = fork(); // Create a child process that will handle the client
                if (process == -1) { perror("Failed to fork"); break; } // If something went wrong, stop the program
                if (process != 0) { close(client); continue; } // Parent process; the client is not needed in the parent process
                // Child process
                signal(SIGUSR1, SIG_IGN); // Remove an unnecessary signal handler

                // Close the server as it is not needed in the child process
                if (close(server) == -1) { perror("Failed to close the server"); goto end_child; }

                // Log the connection
                if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto end_child; }
                log_message(&logger, "Accepted a connection");
                // log_pid(&logger);
                if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto end_child; }

                
                // Receive visitor's gender
                enum Gender gender = NONE;
                if (recv(client, &gender, sizeof(gender), 0) != sizeof(gender)) goto end_child; // Something went wrong and the request should be declined

                // Log the visitor
                if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto end_child; }
                if (gender == MALE) log_message(&logger, "Received gender MALE from the visitor");
                else log_message(&logger, "Received gender FEMALE from the visitor");
                log_pid(&logger);
                if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto end_child; }
        
                // Find a room for the visitor and log this
                if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto end_child; }
                int room = take_room(&rooms, gender);
                if (room == -1) { log_message(&logger, "Failed to register"); log_pid(&logger); }
                else { log_message(&logger, "Registered into the room "); log_integer(&logger, room); log_pid(&logger); log_layout(&logger, &rooms); }
                if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto end_child; }
                
                // Send the status to the visitor
                enum ComeStatus status = (room == -1 ? COME_SORRY : COME_OK);
                if (send(client, &status, sizeof(status), 0) != sizeof(status)) goto end_child;

                // Wait for the client to close the connection - leave the hotel
                char tmp; recv(client, &tmp, sizeof(tmp), 0);

                // Free the room and log this
                if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto end_child; }
                room = free_room(&rooms);
                if (room == -1) { log_message(&logger, "Visitor left"); log_pid(&logger); }
                else { log_message(&logger, "Visitor left room "); log_integer(&logger, room); log_pid(&logger); log_layout(&logger, &rooms); }
                if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto end_child; }

                // Log the stoppage of the server
                if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto end_child; }
                log_message(&logger, "Closed the connection");
                log_pid(&logger);
                if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto end_child; }
                
            end_child:
                // Close everything
                if (close(client) == -1) { perror("Failed to close the client"); delete_rooms(&rooms); delete_logger(&logger); delete_message_queue(&msq); delete_memory(&mem); delete_semaphore(&sem); return 1; }
                if (delete_rooms(&rooms) == -1) { perror("Failed to delete the rooms"); delete_logger(&logger); delete_message_queue(&msq); delete_memory(&mem); delete_semaphore(&sem); return 1; }
                if (delete_logger(&logger) == -1) { perror("Failed to delete the logger"); delete_message_queue(&msq); delete_memory(&mem); delete_semaphore(&sem); return 1; }
                if (delete_message_queue(&msq) == -1) { perror("Failed to delete the memory"); delete_memory(&mem); delete_semaphore(&sem); return 1; }
                if (delete_memory(&mem) == -1) { perror("Failed to delete the memory"); delete_semaphore(&sem); return 1; }
                if (delete_semaphore(&sem) == -1) { perror("Failed to delete the semaphore"); return 1; }
                return 0;
            }
            default:
            {
                if (close(client) == -1) perror("Failed to close the client");
            }
        }
    }

    // Log the stoppage of the server
    if (wait_semaphore(&sem) == -1) { perror("Failed to wait on the semaphore"); goto stop_server; }
    log_message(&logger, "Stopped the server\n");
    if (post_semaphore(&sem) == -1) { perror("Failed to post to the semaphore"); goto stop_server; }
    
stop_server:
    // Delete everything
    if (close(server) == -1) { perror("Failed to close the client"); delete_rooms(&rooms); delete_logger(&logger); delete_message_queue(&msq); delete_memory(&mem); delete_semaphore(&sem); return 1; }
stop_server_5:
    if (delete_rooms(&rooms) == -1) { perror("Failed to delete the rooms"); delete_logger(&logger); delete_message_queue(&msq); delete_memory(&mem); delete_semaphore(&sem); return 1; }
stop_server_4:
    if (delete_logger(&logger) == -1) { perror("Failed to delete the logger"); delete_message_queue(&msq); delete_memory(&mem); delete_semaphore(&sem); return 1; }
stop_server_3:
    if (delete_message_queue(&msq) == -1) { perror("Failed to delete the memory"); delete_memory(&mem); delete_semaphore(&sem); return 1; }
stop_server_2:
    if (delete_memory(&mem) == -1) { perror("Failed to delete the memory"); delete_semaphore(&sem); return 1; }
stop_server_1:
    if (delete_semaphore(&sem) == -1) { perror("Failed to delete the semaphore"); return 1; }
    return 0;
}