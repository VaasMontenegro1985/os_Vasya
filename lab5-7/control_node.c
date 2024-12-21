#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <signal.h>
#include <sys/wait.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>

const char *RABBIT_HOST = "localhost";
int RABBIT_PORT = 5672;
amqp_connection_state_t connection;
amqp_channel_t channel = 1;

typedef struct Node {
    int id;
    pid_t pid;
    bool available;
    struct Node *son;
    struct Node *bro;
} _Tree, *Tree;

Tree tree = NULL;

Tree createTree () {
    Tree newNode = (Tree)malloc(sizeof(_Tree));

    newNode->son = NULL;
    newNode->bro = NULL;
    newNode->id = 0;
    newNode->pid = 0;
    newNode->available = true;

    return newNode;
}

void deleteTree (Tree tree) {
    if (tree == NULL) return;
    deleteTree(tree->son);
    deleteTree(tree->bro);
    free(tree);
}

bool addNodeTree (Tree *tree, int parentId, int id, pid_t pid) {
    if (*tree == NULL) return false;

    if ((*tree)->id != parentId) {
        bool addedToSon = addNodeTree(&((*tree)->son), parentId, id, pid);
        if (addedToSon) return true;
        return addNodeTree(&((*tree)->bro), parentId, id, pid);
    } 

    Tree newNode = (Tree)malloc(sizeof(_Tree));

    newNode->son = NULL;
    newNode->bro = NULL;
    newNode->id = id;
    newNode->pid = pid;
    newNode->available = true;

    Tree *location = &((*tree)->son);

    while (*location != NULL) {
        location = &((*location)->bro);
    }

    *location = newNode;
    
    return true;
}   

bool checkIfAvailable (Tree tree, int id) {
    if (tree == NULL) return false;
    if (tree->id == id) return tree->available;
    return checkIfAvailable(tree->son, id) || checkIfAvailable(tree->bro, id);
}

void setNotAvailable (Tree tree, pid_t pid) {
    if (tree == NULL) return;
    if (tree->pid == pid) {
        tree->available = false;
        return;
    }

    setNotAvailable(tree->son, pid);
    setNotAvailable(tree->bro, pid);
}

pid_t getPidTree (Tree tree, int id) {
    if (tree == NULL) return -1;
    if (tree->id == id) return tree->pid;
    pid_t sonPid = getPidTree(tree->son, id);
    if (sonPid != -1) return sonPid;
    return getPidTree(tree->bro, id);
}

bool printNotAvailable (Tree tree) {
    if (tree == NULL) return false;
    if (!tree->available) printf("%d ", tree->id);

    bool sonAvailable = printNotAvailable(tree->son);
    bool broAvailable = printNotAvailable(tree->bro);
    return sonAvailable || broAvailable || !tree->available;
}

void printTree(Tree tree, int offset) {
    if (tree == NULL) return;
    for (int i = 0; i < offset; i++) printf("\t");
    
    if (tree->id == 0) {
        printf("Controller { id: 0 }\n");
    } else {
        printf("Computing { id: %d, pid: %d, available: %s }\n", tree->id, (int)tree->pid, tree->available ? "true" : "false");
    }
    printTree(tree->son, offset + 1);
    printTree(tree->bro, offset);
}

pid_t createComputingNode (Tree *tree, int parentId, int id) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        char idArg[16];
        sprintf(idArg, "%d", id);
        if (execl("./computing_node.out", "./computing_node.out", idArg, NULL) == -1) perror("execv error");
        return -1;
    } 

    addNodeTree(tree, parentId, id, pid);
    return pid;
}

void exitWithError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void initRabbitMQ() {
    connection = amqp_new_connection();
    amqp_socket_t *socket = amqp_tcp_socket_new(connection);
    if (!socket) exitWithError("Creating TCP socket failed");

    if (amqp_socket_open(socket, RABBIT_HOST, RABBIT_PORT)) exitWithError("Opening TCP socket failed");

    amqp_rpc_reply_t r = amqp_login(
        connection, 
        "/", 
        0, 
        131072, 
        0,
        AMQP_SASL_METHOD_PLAIN, 
        "guest", 
        "guest"
    );
    
    if (r.reply_type != AMQP_RESPONSE_NORMAL) exitWithError("Login failed");

    amqp_channel_open(connection, channel);
    r = amqp_get_rpc_reply(connection);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) exitWithError("Opening channel failed");

    amqp_queue_declare(connection, channel, amqp_cstring_bytes("control"), 0, 0, 0, 0, amqp_empty_table);
    r = amqp_get_rpc_reply(connection);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) exitWithError("Queue declare failed");

    amqp_basic_consume(connection, channel, amqp_cstring_bytes("control"), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
    r = amqp_get_rpc_reply(connection);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) exitWithError("Consume on control failed");

    amqp_queue_purge(connection, channel, amqp_cstring_bytes("control"));
}

void sendMessage (int nodeId, char *cmd) {
    char queue_name[64];
    snprintf(queue_name, sizeof(queue_name), "computing_%d", nodeId);

    uuid_t uuid;
    uuid_generate(uuid);
    char corr_id[37];
    uuid_unparse_lower(uuid, corr_id);

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CORRELATION_ID_FLAG;
    props.correlation_id = amqp_cstring_bytes(corr_id);

    amqp_basic_publish(
        connection, 
        channel, 
        amqp_empty_bytes, 
        amqp_cstring_bytes(queue_name),
        0, 
        0, 
        &props, 
        amqp_cstring_bytes(cmd)
    );
}

void *computersResponseRandler(void *arg) {
    while (1) {
        amqp_envelope_t envelope;
        amqp_maybe_release_buffers(connection);
        amqp_rpc_reply_t res = amqp_consume_message(connection, &envelope, NULL, 0);
        amqp_basic_ack(connection, channel, envelope.delivery_tag, 0);
        if (res.reply_type == AMQP_RESPONSE_NORMAL) {
            printf("%.*s\n", (int)envelope.message.body.len, (char *)envelope.message.body.bytes);
            fflush(stdout);

            amqp_destroy_envelope(&envelope);
        } else {
            printf("Error: amqp_consume_message failed: %s\n", amqp_error_string2(res.reply_type));
            break;
        }
    }
    return NULL;
}

void availableObserver(int signo) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        setNotAvailable(tree, pid);
    }
}

int main () {
    initRabbitMQ();

    tree = createTree();

    pthread_t thr;
    pthread_create(&thr, NULL, computersResponseRandler, NULL);
    pthread_detach(thr);

    struct sigaction action;
    action.sa_handler = availableObserver;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &action, NULL);

    while (true) {
        char prompt[128];

        fgets(prompt, sizeof(prompt), stdin);

        char *operation = strtok(prompt, " \n");

        if (strcmp(operation, "create") == 0) {
            int id = atoi(strtok(NULL, " \n"));
            if (getPidTree(tree, id) != -1) {
                printf("Error: Already Exists\n");
                continue;
            }

            int parentId = atoi(strtok(NULL, " \n"));
            if (getPidTree(tree, parentId) == -1) {
                printf("Error: Parent not found\n");
                continue;
            }

            pid_t pidCreated = createComputingNode(&tree, parentId, id);
            printf("Ok: %d\n", (int)pidCreated); 
        } else if (strcmp(operation, "exec") == 0) {
            int id = atoi(strtok(NULL, " \n"));
            if (getPidTree(tree, id) == -1) {
                printf("Error: %d not found\n", id);
                continue;
            }

            if (!checkIfAvailable(tree, id)) {
                printf("Error: %d not available\n", id);
                continue;
            }

            char text[256];
            fgets(text, sizeof(text), stdin);
            *strchr(text, '\n') = '\0';

            char substr[64];
            fgets(substr, sizeof(substr), stdin);
            *strchr(substr, '\n') = '\0';

            char request[256 + 64 + 2];
            sprintf(request, "%s;%s;", text, substr);

            sendMessage(id, request);
        } else if (strcmp(operation, "pingall") == 0) {
            printf("Ok: ");
            if (!printNotAvailable(tree)) printf("-1");
            printf("\n");
        } else if (strcmp(operation, "print") == 0) {
            printTree(tree, 0);
        } else if (strcmp(operation, "exit") == 0) {
            break;
        } else {
            printf("Unknown prompt type\n");
        }
    }

    deleteTree(tree);
    return 0;
}
