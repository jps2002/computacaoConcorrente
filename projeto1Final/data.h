// data modelling

/**
 * @brief Represents  node from a queue containg a pointer to a client_socket 
 */
typedef struct nodeaux
{
    struct nodeaux* next_node;
    int* client_socket;
} node;



typedef struct nodeListAux
{
    struct nodeListAux* next_node;
    int client_socket;
} nodeFromList;

typedef struct listAux
{
    nodeFromList* head;
    int size;
    
} linkedList;


// function prototypes

void enqueue(int* client_socket);
int* dequeue();
int removeList(int toBeRemoved, linkedList* list);
nodeFromList* insertList(int toBeInserted, linkedList* list);
linkedList* createList();